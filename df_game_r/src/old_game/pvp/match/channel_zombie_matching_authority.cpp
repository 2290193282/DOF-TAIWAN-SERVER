#include "old_game/pvp/match/channel_zombie_matching_authority.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/config.h"
#include "core/config/env/environment.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/log_manager.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/pvp_party/db_matching_channel_handoff.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/instance/zombie_itembox_runtime.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/arena/zombie_owner_snapshot.h"
#include "old_game/pvp/battlefield/zombie_preferred_start_runtime.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/pvp/match/matching_channel_handoff.h"
#include "old_game/social/party/party.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/system_time.h"

#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <ctime>
#include <deque>
#include <map>
#include <utility>
#include <vector>

namespace channel_zombie_matching_authority {
namespace {

using channel_zombie_matching_producer::AuthorityV2;
using channel_zombie_matching_producer::TargetRoute;
using matching_channel_handoff::ClaimedHandoff;
using matching_channel_handoff_db::Message;

const uint8_t kBound =
    channel_zombie_matching_producer::kChannelMatchingBound;
const uint16_t kMode =
    channel_zombie_matching_producer::kZombieMatchingMode;
// Andrea1 sends CMD518 with the client-facing matching key (0, 0).  The
// dispatcher normalizes that key to the project-owned authority pair (1, 15)
// only inside the server.  NOTI458 is consumed by the client without an
// inverse normalization step, so it must carry the original wire key.
const uint8_t kClientWireBound = 0u;
const uint8_t kClientWireMode = 0u;
const size_t kMaximumMembers =
    channel_zombie_matching_producer::kMaximumMembers;
const size_t kRoomSeats =
    channel_zombie_matching_producer::kRoomSeats;
const int kRouteCacheLifetimeSeconds = 15;
const int kRouteRetrySeconds = 5;
const int kQueueDrainSeconds = 1;
const int kSourceBuildDeadlineSeconds = 15;
const int kRouteResponseDeadlineSeconds = 15;
const unsigned int kMaximumRouteAttempts = 2u;
const int kSourceHandoffDeadlineSeconds = 90;
const int kSourceReleaseDeadlineSeconds = 15;
const int kTargetClaimDeadlineSeconds = 90;
const int kControlResponseDeadlineSeconds = 15;
const int kTargetAuthorizeDeadlineSeconds = 15;
// Both original 0828 matching implementations arm a 180-second P2P timer.
// Keep that protocol window distinct from the later map-load timeout.
const int kTargetP2PDeadlineSeconds = 180;
const int kTargetLoadDeadlineSeconds = 60;
const int kZombieSettlementGraceSeconds = 115;
const size_t kMaximumQueuedGroups = 64u;

struct MemberRef {
    MemberRef()
        : account_id(0u), charac_no(0u), world_uid(0xffffu),
          dispatch_uid(-1), server_group(-1), ready_key(0u),
          lobby_token(0u), global_slot(0xffu), lobby_side(0xffu) {}

    uint32_t account_id;
    uint32_t charac_no;
    uint16_t world_uid;
    int dispatch_uid;
    int server_group;
    // Project-owned matching-UI identity.  It is allocated once when the
    // complete QueueGroup enters a six-seat lobby and survives source-room
    // failure/RestoreGroupsLocked.  None of these values are PII.
    uint32_t ready_key;
    uint32_t lobby_token;
    uint8_t global_slot;
    uint8_t lobby_side;
};

struct QueueGroup {
    QueueGroup() : party_index(-1), leader() {}

    int party_index;
    MemberRef leader;
    std::vector<MemberRef> members;
};

enum SourceState {
    SOURCE_BUILDING = 1,
    SOURCE_ROUTE_PENDING = 2,
    SOURCE_ROUTE_READY = 3,
    SOURCE_HANDOFF_PENDING = 4,
    SOURCE_DETACHING = 5,
    SOURCE_ARMED = 6,
};

struct SourceCohort {
    SourceCohort()
        : session_token(0u), next_request_token(1u),
          pending_request_token(0u), state(SOURCE_BUILDING), room(0),
          room_index(-1), cohort_id(0u), source_server_group(-1),
          source_channel(-1), route_cached_at(0), deadline_at(0),
          route_attempts(0u), leader(), groups(), members(), route() {}

    uint32_t session_token;
    uint32_t next_request_token;
    uint32_t pending_request_token;
    SourceState state;
    PvP_Room* room;
    int room_index;
    uint32_t cohort_id;
    int source_server_group;
    int source_channel;
    std::time_t route_cached_at;
    std::time_t deadline_at;
    unsigned int route_attempts;
    MemberRef leader;
    std::vector<QueueGroup> groups;
    std::vector<MemberRef> members;
    TargetRoute route;
};

enum TargetState {
    TARGET_COLLECTING = 1,
    TARGET_ACTIVATE_PENDING = 2,
    TARGET_AUTHORIZING = 3,
    TARGET_CONNECTING = 4,
    TARGET_LOADING = 5,
    TARGET_READY = 6,
    TARGET_STARTING = 7,
    TARGET_RUNNING = 8,
    TARGET_FINISHING = 9,
    TARGET_SETTLING = 10,
    TARGET_DEGRADING = 11,
    TARGET_SELECTING = 12,
};

struct TargetCohort {
    TargetCohort()
        : cohort_id(0u), cohort_size(0u), cohort_flags(0u),
          session_token(0u), next_request_token(1u),
          pending_request_token(0u), state(TARGET_COLLECTING), room(0),
          room_index(-1), authorized(false), local_same_channel(false),
          bootstrap_publish_in_progress(false), abort_pending(false),
          external_reset_pending(false), matching_scene_committed(false),
          source_server_group(-1),
          source_channel(-1), target_server_group(-1), target_channel(-1),
          field0(0u), endpoint_key(0u), endpoint_port_like(0u),
          deadline_at(0), ready_duration_ms(0u), play_duration_ms(0u),
          ready_deadline_ms(0u), play_deadline_ms(0u),
          play_policy(), complete_load_expected_mask(0u),
          complete_load_ready_mask(0u), p2p_expected_mask(0u),
          p2p_ready_mask(0u),
          load_epoch(0u), expected_mask(0u),
          loaded_mask(0u), active_mask(0u), active_count(0u),
          degraded_from_six(false) {
        std::memset(present, 0, sizeof(present));
        std::memset(original_users, 0, sizeof(original_users));
        std::memset(handoffs, 0, sizeof(handoffs));
        std::memset(map_parties, 0, sizeof(map_parties));
        std::memset(map_loaded, 0, sizeof(map_loaded));
        std::memset(global_to_active, 0xff, sizeof(global_to_active));
        std::memset(active_to_global, 0xff, sizeof(active_to_global));
        for (size_t slot = 0u; slot < kRoomSeats; ++slot)
            map_party_indices[slot] = -1;
    }

    uint32_t cohort_id;
    uint8_t cohort_size;
    uint8_t cohort_flags;
    uint32_t session_token;
    uint32_t next_request_token;
    uint32_t pending_request_token;
    TargetState state;
    PvP_Room* room;
    int room_index;
    bool authorized;
    // Official Zombie matching stays in the current Type19 process.  This
    // flag distinguishes that local lifecycle from the retired project-owned
    // DB handoff path so completion/reset never emits route SQL.
    bool local_same_channel;
    // The lease starts in the same locked transaction that publishes the
    // local target/remote room attachment.  It therefore protects Activate,
    // all six map-owner builds, TARGET_LOADING commit and the exhaustive
    // bootstrap publication.  A terminal request is collapsed to one delayed
    // abort; an external PvP_Room::reset waiter owns the final room reset so
    // the room is never reset/pooled twice.  Network sends never run under
    // g_mutex.
    bool bootstrap_publish_in_progress;
    bool abort_pending;
    bool external_reset_pending;
    // Set in the same transaction that moves the cohort to CONNECTING,
    // before NOTI458 can become visible.  Rollback uses it to issue the
    // official CMD42 client-scene reset even when no load epoch was allocated.
    bool matching_scene_committed;
    int source_server_group;
    int source_channel;
    int target_server_group;
    int target_channel;
    uint8_t field0;
    uint32_t endpoint_key;
    uint32_t endpoint_port_like;
    std::time_t deadline_at;
    uint32_t ready_duration_ms;
    uint32_t play_duration_ms;
    uint32_t ready_deadline_ms;
    uint32_t play_deadline_ms;
    zombie_mode::ZombiePlayModePolicy play_policy;
    // CMD295 reports use the same immutable physical seats as CMD296.  Each
    // first report publishes one official NOTI278(reporter unique_id) to all
    // six clients; CMD296 is rejected until this mask is complete.
    uint8_t complete_load_expected_mask;
    uint8_t complete_load_ready_mask;
    // CMD296 reports use immutable PvP/global seats.  NOTI279 is published
    // exactly once after all six reporter bits arrive.  The retained map
    // bootstrap must already be visible so the 0828 controller can emit
    // CMD295 in the first place.
    uint8_t p2p_expected_mask;
    uint8_t p2p_ready_mask;
    // One epoch is allocated at the sealed CONNECTING commit immediately
    // before bootstrap publication.  CMD40 may arrive while P2P is still
    // connecting; its immutable seat bit is buffered and cannot start the
    // room until NOTI279 has been sent.  The five-player timeout path never
    // compacts those seats.
    uint32_t load_epoch;
    uint8_t expected_mask;
    uint8_t loaded_mask;
    uint8_t active_mask;
    uint8_t active_count;
    bool degraded_from_six;
    uint8_t global_to_active[kRoomSeats];
    uint8_t active_to_global[kRoomSeats];
    bool present[kRoomSeats];
    MemberRef members[kRoomSeats];
    // Pointer value of the exact connection/session that entered this cohort.
    // It is comparison-only after registration: ResolveMember supplies the
    // live object and this value prevents a late rollback ACK from reaching a
    // newly logged-in session that happens to reuse the same wire identity.
    CUser* original_users[kRoomSeats];
    ClaimedHandoff handoffs[kRoomSeats];
    // The PvP_Room remains the authoritative eight-seat combat roster.  Each
    // occupied seat owns one ordinary auto-created single CParty solely as
    // its independent 3101 map context; no CParty/CBattle_Field ABI is grown.
    CParty* map_parties[kRoomSeats];
    int map_party_indices[kRoomSeats];
    bool map_loaded[kRoomSeats];
};

struct MapPartyOwnerRef {
    MapPartyOwnerRef()
        : cohort_id(0u), slot(0u), party_index(-1), load_epoch(0u),
          detaching(false), member() {}

    uint32_t cohort_id;
    size_t slot;
    int party_index;
    uint32_t load_epoch;
    bool detaching;
    MemberRef member;
};

void AbortTargetCohort(uint32_t cohort_id);
bool FreezeSixPlayerLoadLocked(TargetCohort& target);
bool StartTargetCohort(
    uint32_t cohort_id);

struct LobbyReadyDelivery {
    std::vector<MemberRef> recipients;
    std::vector<MemberRef> entries;
};

struct LobbyCancelDelivery {
    LobbyCancelDelivery() : ready_key(0u), recipients() {}

    uint32_t ready_key;
    std::vector<MemberRef> recipients;
};

struct LobbyUiUpdates {
    std::vector<LobbyReadyDelivery> ready;
    std::vector<LobbyCancelDelivery> cancel;
};

struct PendingEnqueueCommit {
    PendingEnqueueCommit() : requester(), ui_updates(), cohort_token(0u) {}

    MemberRef requester;
    LobbyUiUpdates ui_updates;
    uint32_t cohort_token;
};

Mutex g_mutex;
pthread_cond_t g_bootstrap_publish_cond = PTHREAD_COND_INITIALIZER;
bool g_started = false;
uint32_t g_next_authority_token = 0u;
uint32_t g_next_ready_key = 0u;
uint32_t g_next_lobby_token = 0u;
uint32_t g_next_load_epoch = 0u;
std::time_t g_route_retry_at = 0;
std::deque<QueueGroup> g_queue;
std::map<uint32_t, SourceCohort> g_sources;
std::map<PvP_Room*, uint32_t> g_source_rooms;
std::map<uint32_t, uint32_t> g_source_cohorts;
std::map<uint32_t, TargetCohort> g_targets;
std::map<PvP_Room*, uint32_t> g_target_rooms;
std::map<CParty*, MapPartyOwnerRef> g_target_map_parties;
std::map<CUser*, PendingEnqueueCommit> g_pending_enqueue_commits;

void QueueTargetAbortControl(const TargetCohort& target);

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) result = value++;
    if (value == 0u) ++value;
    return result;
}

uint32_t MakeProcessAuthoritySeed() {
    const uint32_t seconds = static_cast<uint32_t>(std::time(0));
    const uint32_t process = static_cast<uint32_t>(::getpid());
    uint32_t seed = seconds ^ (process << 16) ^ (process * 0x45d9f3bu);
    if (seed == 0u) seed = 1u;
    return seed;
}

uint32_t CurrentTickMilliseconds() {
    return static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
}

bool TickDeadlineReached(uint32_t now, uint32_t deadline) {
    return deadline != 0u &&
           static_cast<int32_t>(now - deadline) >= 0;
}

uint32_t AddTickDelay(uint32_t now, uint32_t delay) {
    const uint32_t deadline = now + delay;
    return deadline ? deadline : 1u;
}

int MillisecondsToCeilingSeconds(uint32_t milliseconds) {
    return static_cast<int>((milliseconds + 999u) / 1000u);
}

bool IsServerGroup(int value) {
    return value >= 0 && value < 18;
}

bool IsChannel(int value) {
    return value > 0 && value <= 65535;
}

bool IsMatchingCohortSize(size_t value) {
    return zombie_matching_runtime_policy::IsRuntimeCohortSize(value);
}

bool IsPlayableCohortSize(size_t value) {
    return value == static_cast<size_t>(
                        zombie_matching_runtime_policy::
                            kOfficialMinimumMembers) ||
           value == static_cast<size_t>(
                        zombie_matching_runtime_policy::
                            kOfficialMaximumMembers);
}

uint8_t GlobalSlotBit(size_t slot) {
    return slot < kRoomSeats
               ? static_cast<uint8_t>(1u << slot)
               : static_cast<uint8_t>(0u);
}

size_t CountGlobalSlots(uint8_t mask) {
    size_t count = 0u;
    for (size_t slot = 0u; slot < kRoomSeats; ++slot) {
        if (mask & GlobalSlotBit(slot)) ++count;
    }
    return count;
}

bool InstallActiveSlotMapping(TargetCohort& target,
                              uint8_t active_mask,
                              uint8_t active_count) {
    if (!IsPlayableCohortSize(active_count) ||
        CountGlobalSlots(active_mask) != active_count ||
        (active_mask & static_cast<uint8_t>(~target.expected_mask)) != 0u) {
        return false;
    }
    std::memset(target.global_to_active, 0xff,
                sizeof(target.global_to_active));
    std::memset(target.active_to_global, 0xff,
                sizeof(target.active_to_global));
    uint8_t active_slot = 0u;
    for (size_t global_slot = 0u;
         global_slot < target.cohort_size;
         ++global_slot) {
        if ((active_mask & GlobalSlotBit(global_slot)) == 0u) continue;
        if (!target.present[global_slot] || active_slot >= active_count)
            return false;
        target.global_to_active[global_slot] = active_slot;
        target.active_to_global[active_slot] =
            static_cast<uint8_t>(global_slot);
        ++active_slot;
    }
    if (active_slot != active_count) return false;
    target.active_mask = active_mask;
    target.active_count = active_count;
    return true;
}

MemberRef MakeMember(CUser* user) {
    MemberRef result;
    if (!user) return result;
    result.account_id = static_cast<uint32_t>(user->get_acc_id());
    result.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    result.world_uid = static_cast<uint16_t>(user->get_unique_id());
    result.dispatch_uid = user->GetUID();
    result.server_group = user->GetServerGroup();
    return result;
}

bool SameMember(const MemberRef& lhs, const MemberRef& rhs) {
    return lhs.account_id == rhs.account_id &&
           lhs.charac_no == rhs.charac_no &&
           lhs.world_uid == rhs.world_uid &&
           lhs.dispatch_uid == rhs.dispatch_uid &&
           lhs.server_group == rhs.server_group;
}

CUser* ResolveMember(const MemberRef& expected) {
    GameWorld* const world = G_GameWorld();
    CUser* const user = world ? world->find_from_world(expected.world_uid) : 0;
    return user && SameMember(MakeMember(user), expected) ? user : 0;
}

CUser* ResolveOriginalTargetMember(const TargetCohort& target, size_t slot) {
    if (slot >= target.cohort_size || !target.present[slot] ||
        !target.original_users[slot]) return 0;
    CUser* const live = ResolveMember(target.members[slot]);
    return live == target.original_users[slot] ? live : 0;
}

bool IsMatchingIdentityEligible(CUser* user) {
    CEnvironment* const environment = G_CEnvironment();
    GameWorld* const world = G_GameWorld();
    return user && environment && world &&
           world->GetChannelType() ==
               channel_zombie_matching_producer::kZombieChannelType &&
           user->get_state() == 3 &&
           user->getCurCharacR() && !user->IsCurCharacGhost() &&
           user->get_acc_id() > 0 && user->getCurCharacNo() > 0 &&
           user->get_unique_id() > 0 && user->get_unique_id() < 0xffff &&
           user->get_charac_job() !=
               oldsrv::config::kCharacterJobCreatorMage &&
           user->GetUID() >= 0 &&
           IsServerGroup(user->GetServerGroup()) &&
           user->GetServerGroup() == environment->get_server_group() &&
           IsChannel(environment->get_channel_no());
}

bool IsQueueEligible(CUser* user) {
    return IsMatchingIdentityEligible(user) && user->CheckInPvp() == 0;
}

bool IsBuiltSourceRoomMember(CUser* user, PvP_Room* source_room) {
    return source_room && IsMatchingIdentityEligible(user) &&
           !user->CheckInParty() && user->CheckInPvp() != 0 &&
           user->GetPvpIndex() == source_room->get_index() &&
           user->GetPVPRoom() == source_room;
}

void TraceQueueEligibilityReject(CUser* user) {
    CEnvironment* const environment = G_CEnvironment();
    GameWorld* const world = G_GameWorld();
    unsigned int mask = 0u;
    if (!user) {
        mask |= 1u << 0;
    } else {
        if (!world || world->GetChannelType() !=
                channel_zombie_matching_producer::kZombieChannelType)
            mask |= 1u << 1;
        if (user->get_state() != 3) mask |= 1u << 2;
        if (!user->getCurCharacR()) mask |= 1u << 3;
        if (user->IsCurCharacGhost()) mask |= 1u << 4;
        if (user->get_acc_id() <= 0 || user->getCurCharacNo() <= 0)
            mask |= 1u << 5;
        if (user->get_unique_id() <= 0 || user->get_unique_id() >= 0xffff)
            mask |= 1u << 6;
        if (user->get_charac_job() ==
                oldsrv::config::kCharacterJobCreatorMage)
            mask |= 1u << 7;
        if (user->GetUID() < 0) mask |= 1u << 8;
        if (user->CheckInPvp() != 0) mask |= 1u << 9;
        if (!environment || !IsServerGroup(user->GetServerGroup()) ||
            user->GetServerGroup() != environment->get_server_group())
            mask |= 1u << 10;
        if (!environment || !IsChannel(environment->get_channel_no()))
            mask |= 1u << 11;
        if (user->CheckInParty()) mask |= 1u << 12;
    }

    cMyTrace trace(
        "channel_zombie_matching_authority::TraceQueueEligibilityReject",
        258,
        0);
    trace("ZombieQueueReject mask=0x%X state=%d channel_type=%d party=%d pvp=%d server_group=%d env_group=%d channel=%d job=%d",
          mask,
          user ? user->get_state() : -1,
          world ? world->GetChannelType() : -1,
          user ? user->CheckInParty() : -1,
          user ? user->CheckInPvp() : -1,
          user ? user->GetServerGroup() : -1,
          environment ? environment->get_server_group() : -1,
          environment ? environment->get_channel_no() : -1,
          user ? user->get_charac_job() : -1);
}

bool ContainsMember(const std::vector<MemberRef>& members,
                    const MemberRef& target) {
    for (std::vector<MemberRef>::const_iterator it = members.begin();
         it != members.end(); ++it) {
        if (SameMember(*it, target)) return true;
    }
    return false;
}

bool CaptureQueueGroup(CUser* requester, QueueGroup& group) {
    if (!IsQueueEligible(requester)) return false;
    group = QueueGroup();
    group.leader = MakeMember(requester);
    // The official Andrea entry rejects users who are already in a party.
    // Zombie matching therefore queues only independent users; the authority
    // still assembles six accepted solo entries into its exact 3+3 lobby.
    if (requester->CheckInParty()) return false;
    group.members.push_back(group.leader);
    return true;
}

bool ValidateQueueGroup(const QueueGroup& group) {
    if (group.members.empty() || group.members.size() > 3u) return false;
    std::vector<CUser*> users;
    for (std::vector<MemberRef>::const_iterator it = group.members.begin();
         it != group.members.end(); ++it) {
        CUser* const user = ResolveMember(*it);
        if (!IsQueueEligible(user)) return false;
        users.push_back(user);
    }
    if (group.party_index < 0) {
        return users.size() == 1u && !users[0]->CheckInParty();
    }
    CParty* const party = G_CGameManager()->GetParty(group.party_index);
    if (!party || party->getManager() != ResolveMember(group.leader) ||
        party->get_member_count() != static_cast<int>(users.size())) {
        return false;
    }
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const member = party->get_user(seat);
        if (member && !ContainsMember(group.members, MakeMember(member)))
            return false;
    }
    return true;
}

bool GroupOverlaps(const QueueGroup& lhs, const QueueGroup& rhs) {
    for (std::vector<MemberRef>::const_iterator it = lhs.members.begin();
         it != lhs.members.end(); ++it) {
        if (ContainsMember(rhs.members, *it)) return true;
    }
    return false;
}

bool SameQueueGroup(const QueueGroup& lhs, const QueueGroup& rhs) {
    if (lhs.party_index != rhs.party_index ||
        lhs.members.size() != rhs.members.size() ||
        !SameMember(lhs.leader, rhs.leader)) return false;
    for (size_t index = 0u; index < lhs.members.size(); ++index) {
        if (!SameMember(lhs.members[index], rhs.members[index])) return false;
    }
    return true;
}

bool MemberInManagedCohortLocked(const MemberRef& member) {
    for (std::map<uint32_t, SourceCohort>::const_iterator source =
             g_sources.begin(); source != g_sources.end(); ++source) {
        if (ContainsMember(source->second.members, member)) return true;
    }
    for (std::map<uint32_t, TargetCohort>::const_iterator target =
             g_targets.begin(); target != g_targets.end(); ++target) {
        for (size_t slot = 0u; slot < target->second.cohort_size; ++slot) {
            if (target->second.present[slot] &&
                SameMember(target->second.members[slot], member)) return true;
        }
    }
    return false;
}

struct LobbySlotLess {
    bool operator()(const MemberRef& lhs, const MemberRef& rhs) const {
        return lhs.global_slot < rhs.global_slot;
    }
};

bool HasLobbyIdentity(const MemberRef& member) {
    return member.ready_key != 0u && member.lobby_token != 0u &&
           member.global_slot < 6u && member.lobby_side >= 1u &&
           member.lobby_side <= 2u &&
           member.lobby_side == member.global_slot / 3u + 1u;
}

std::vector<MemberRef> CollectLobbyMembersLocked(uint32_t lobby_token) {
    std::vector<MemberRef> members;
    if (!lobby_token) return members;
    for (std::deque<QueueGroup>::const_iterator group = g_queue.begin();
         group != g_queue.end(); ++group) {
        for (std::vector<MemberRef>::const_iterator member =
                 group->members.begin();
             member != group->members.end(); ++member) {
            if (member->lobby_token == lobby_token)
                members.push_back(*member);
        }
    }
    std::sort(members.begin(), members.end(), LobbySlotLess());
    return members;
}

bool QueueCanonicalLobbyReadyLocked(uint32_t lobby_token,
                                    LobbyUiUpdates& updates) {
    const std::vector<MemberRef> members =
        CollectLobbyMembersLocked(lobby_token);
    if (members.empty()) return false;

    // The CMD518 success handler clears only the requester's ready-user
    // container.  Re-publish one canonical snapshot to every member so all
    // clients converge even if an earlier incremental NOTI595 was missed.
    LobbyReadyDelivery delivery;
    delivery.recipients = members;
    delivery.entries = members;
    updates.ready.push_back(delivery);
    return true;
}

bool AssignLobbyPlacementLocked(QueueGroup& group) {
    const size_t group_size = group.members.size();
    if (group_size == 0u || group_size > 3u) return false;

    std::vector<uint32_t> lobby_order;
    std::map<uint32_t, unsigned int> occupied;
    for (std::deque<QueueGroup>::const_iterator queued = g_queue.begin();
         queued != g_queue.end(); ++queued) {
        if (queued->members.empty()) return false;
        const uint32_t lobby_token = queued->members[0].lobby_token;
        if (!lobby_token) return false;
        if (occupied.find(lobby_token) == occupied.end()) {
            occupied[lobby_token] = 0u;
            lobby_order.push_back(lobby_token);
        }
        for (std::vector<MemberRef>::const_iterator member =
                 queued->members.begin();
             member != queued->members.end(); ++member) {
            if (!HasLobbyIdentity(*member) ||
                member->lobby_token != lobby_token ||
                (occupied[lobby_token] &
                     (1u << member->global_slot)) != 0u) {
                return false;
            }
            occupied[lobby_token] |= 1u << member->global_slot;
        }
    }

    uint32_t selected_lobby = 0u;
    uint8_t selected_side = 0xffu;
    unsigned int selected_mask = 0u;
    for (std::vector<uint32_t>::const_iterator lobby = lobby_order.begin();
         lobby != lobby_order.end() && !selected_lobby; ++lobby) {
        const unsigned int mask = occupied[*lobby];
        for (uint8_t side = 0u; side < 2u; ++side) {
            size_t free_count = 0u;
            const uint8_t first = static_cast<uint8_t>(side * 3u);
            for (uint8_t slot = first; slot < first + 3u; ++slot) {
                if ((mask & (1u << slot)) == 0u) ++free_count;
            }
            if (free_count >= group_size) {
                selected_lobby = *lobby;
                selected_side = side;
                selected_mask = mask;
                break;
            }
        }
    }
    if (!selected_lobby) {
        selected_lobby = NextNonZero(g_next_lobby_token);
        selected_side = 0u;
        selected_mask = 0u;
    }

    size_t member_index = 0u;
    const uint8_t first = static_cast<uint8_t>(selected_side * 3u);
    for (uint8_t slot = first;
         slot < first + 3u && member_index < group_size; ++slot) {
        if ((selected_mask & (1u << slot)) != 0u) continue;
        MemberRef& member = group.members[member_index++];
        member.ready_key = NextNonZero(g_next_ready_key);
        member.lobby_token = selected_lobby;
        member.global_slot = slot;
        // CLIENT_IDA_EVIDENCE: NOTI595 record+0x34 uses presentation tags
        // 1 and 2 for the two mirrored three-seat columns.  Zero means
        // "choose any free seat" and must never be used for an assigned
        // lobby member.
        member.lobby_side = static_cast<uint8_t>(selected_side + 1u);
    }
    if (member_index != group_size) return false;

    bool leader_updated = false;
    for (std::vector<MemberRef>::const_iterator member = group.members.begin();
         member != group.members.end(); ++member) {
        if (SameMember(*member, group.leader)) {
            group.leader = *member;
            leader_updated = true;
            break;
        }
    }
    return leader_updated;
}

void QueueLobbyCancelForGroupLocked(const QueueGroup& group,
                                    LobbyUiUpdates& updates) {
    if (group.members.empty()) return;
    const std::vector<MemberRef> recipients =
        CollectLobbyMembersLocked(group.members[0].lobby_token);
    for (std::vector<MemberRef>::const_iterator member = group.members.begin();
         member != group.members.end(); ++member) {
        if (!member->ready_key) continue;
        LobbyCancelDelivery delivery;
        delivery.ready_key = member->ready_key;
        delivery.recipients = recipients;
        updates.cancel.push_back(delivery);
    }
}

void QueueLobbyCancelForMembers(const std::vector<MemberRef>& members,
                                LobbyUiUpdates& updates) {
    for (std::vector<MemberRef>::const_iterator member = members.begin();
         member != members.end(); ++member) {
        if (!member->ready_key) continue;
        LobbyCancelDelivery delivery;
        delivery.ready_key = member->ready_key;
        delivery.recipients = members;
        updates.cancel.push_back(delivery);
    }
}

void TraceLobbyReadySendFailure(const char* reason,
                                const LobbyReadyDelivery& delivery,
                                const MemberRef* recipient,
                                const MemberRef* invalid_entry) {
    size_t side1_count = 0u;
    size_t side2_count = 0u;
    size_t invalid_side_count = 0u;
    for (std::vector<MemberRef>::const_iterator entry =
             delivery.entries.begin();
         entry != delivery.entries.end(); ++entry) {
        if (entry->lobby_side == 1u) {
            ++side1_count;
        } else if (entry->lobby_side == 2u) {
            ++side2_count;
        } else {
            ++invalid_side_count;
        }
    }

    LogManager::logFormat(
        1,
        "channel_zombie_matching_authority.cpp",
        "channel_zombie_matching_authority::TraceLobbyReadySendFailure",
        __LINE__,
        "[ZOMBIE_MATCH][LOBBY_READY_SEND_FAIL] reason=%s recipient_key=%u recipient_slot=%u recipient_side=%u recipients=%u entries=%u side1=%u side2=%u side_invalid=%u entry_key=%u entry_slot=%u entry_side=%u",
        reason ? reason : "unknown",
        recipient ? static_cast<unsigned int>(recipient->ready_key) : 0u,
        recipient ? static_cast<unsigned int>(recipient->global_slot) : 0xffu,
        recipient ? static_cast<unsigned int>(recipient->lobby_side) : 0xffu,
        static_cast<unsigned int>(delivery.recipients.size()),
        static_cast<unsigned int>(delivery.entries.size()),
        static_cast<unsigned int>(side1_count),
        static_cast<unsigned int>(side2_count),
        static_cast<unsigned int>(invalid_side_count),
        invalid_entry ?
            static_cast<unsigned int>(invalid_entry->ready_key) : 0u,
        invalid_entry ?
            static_cast<unsigned int>(invalid_entry->global_slot) : 0xffu,
        invalid_entry ?
            static_cast<unsigned int>(invalid_entry->lobby_side) : 0xffu);
}

bool SendLobbyReady(const LobbyReadyDelivery& delivery) {
    const MemberRef* const first_recipient =
        delivery.recipients.empty() ? 0 : &delivery.recipients[0];
    if (delivery.recipients.empty()) {
        TraceLobbyReadySendFailure("recipients_empty", delivery, 0, 0);
        return false;
    }
    if (delivery.entries.empty() || delivery.entries.size() > 6u) {
        TraceLobbyReadySendFailure("entry_count", delivery,
                                   first_recipient, 0);
        return false;
    }

    std::vector<CUser*> entries;
    for (std::vector<MemberRef>::const_iterator entry =
             delivery.entries.begin();
         entry != delivery.entries.end(); ++entry) {
        CUser* const user = ResolveMember(*entry);
        const int job = user ? user->get_charac_job() : -1;
        const int packed_grow = user ? user->getCurCharacGrowType() : -1;
        const int level = user ? user->get_charac_level() : -1;
        const char* const name = user ? user->getCurCharacName() : 0;
        const size_t name_length = name ? std::strlen(name) : 0u;
        const char* failure = 0;
        if (!user) {
            failure = "entry_offline";
        } else if (!HasLobbyIdentity(*entry)) {
            failure = "entry_identity";
        } else if (!name || name_length == 0u || name_length > 255u) {
            failure = "entry_name";
        } else if (job < 0 || job > 0xff) {
            failure = "entry_job";
        } else if (packed_grow < 0 || packed_grow > 0x7f) {
            failure = "entry_grow";
        } else if (level < 0 || level > 0xffff) {
            failure = "entry_level";
        }
        if (failure) {
            TraceLobbyReadySendFailure(failure, delivery, &*entry, &*entry);
            return false;
        }
        entries.push_back(user);
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_READY_USER_INFO);
    packet.put_byte(static_cast<uint8_t>(entries.size()));
    for (size_t index = 0u; index < entries.size(); ++index) {
        CUser* const user = entries[index];
        const MemberRef& entry = delivery.entries[index];
        const char* const name = user->getCurCharacName();
        const int name_length = static_cast<int>(std::strlen(name));
        const uint8_t packed_grow = static_cast<uint8_t>(
            user->getCurCharacGrowType());
        packet.put_int(static_cast<int>(entry.ready_key));
        packet.put_int(name_length);
        packet.put_str(name, name_length);
        packet.put_byte(entry.lobby_side);
        packet.put_byte(static_cast<uint8_t>(user->get_charac_job()));
        packet.put_byte(packed_grow);
        packet.put_short(static_cast<uint16_t>(user->get_charac_level()));
    }
    packet.finalize(true);

    bool sent = true;
    for (std::vector<MemberRef>::const_iterator recipient =
             delivery.recipients.begin();
         recipient != delivery.recipients.end(); ++recipient) {
        CUser* const user = ResolveMember(*recipient);
        if (!user) {
            TraceLobbyReadySendFailure("recipient_offline", delivery,
                                       &*recipient, 0);
            sent = false;
        } else if (user->Send(packet) == 0) {
            TraceLobbyReadySendFailure("send_zero", delivery,
                                       &*recipient, 0);
            sent = false;
        }
    }
    return sent;
}

bool SendLobbyCancel(const LobbyCancelDelivery& delivery) {
    if (!delivery.ready_key || delivery.recipients.empty()) return false;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CANCEL_USER);
    packet.put_int(static_cast<int>(delivery.ready_key));
    packet.finalize(true);
    bool sent = true;
    for (std::vector<MemberRef>::const_iterator recipient =
             delivery.recipients.begin();
         recipient != delivery.recipients.end(); ++recipient) {
        CUser* const user = ResolveMember(*recipient);
        if (!user || user->Send(packet) == 0) sent = false;
    }
    return sent;
}

bool DispatchLobbyUiUpdates(const LobbyUiUpdates& updates) {
    for (std::vector<LobbyCancelDelivery>::const_iterator cancel =
             updates.cancel.begin();
         cancel != updates.cancel.end(); ++cancel) {
        (void)SendLobbyCancel(*cancel);
    }
    bool ready_sent = true;
    for (std::vector<LobbyReadyDelivery>::const_iterator ready =
             updates.ready.begin();
         ready != updates.ready.end(); ++ready) {
        if (!SendLobbyReady(*ready)) ready_sent = false;
    }
    return ready_sent;
}

void PurgeInvalidQueueLocked(LobbyUiUpdates& updates) {
    for (size_t index = 0u; index < g_queue.size();) {
        if (!ValidateQueueGroup(g_queue[index])) {
            QueueLobbyCancelForGroupLocked(g_queue[index], updates);
            g_queue.erase(g_queue.begin() + index);
        } else {
            ++index;
        }
    }
}

bool SelectCohortLocked(std::vector<size_t>& selected) {
    selected.clear();
    std::vector<uint32_t> lobby_order;
    for (size_t index = 0u; index < g_queue.size(); ++index) {
        if (g_queue[index].members.empty()) continue;
        const uint32_t token = g_queue[index].members[0].lobby_token;
        if (std::find(lobby_order.begin(), lobby_order.end(), token) ==
            lobby_order.end()) {
            lobby_order.push_back(token);
        }
    }
    for (std::vector<uint32_t>::const_iterator lobby = lobby_order.begin();
         lobby != lobby_order.end(); ++lobby) {
        std::vector<size_t> candidate;
        unsigned int slot_mask = 0u;
        size_t member_count = 0u;
        size_t left_count = 0u;
        size_t right_count = 0u;
        bool valid = *lobby != 0u;
        for (size_t index = 0u; index < g_queue.size(); ++index) {
            const QueueGroup& group = g_queue[index];
            if (group.members.empty() ||
                group.members[0].lobby_token != *lobby) continue;
            candidate.push_back(index);
            for (std::vector<MemberRef>::const_iterator member =
                     group.members.begin();
                 member != group.members.end(); ++member) {
                if (!HasLobbyIdentity(*member) ||
                    member->lobby_token != *lobby ||
                    (slot_mask & (1u << member->global_slot)) != 0u) {
                    valid = false;
                    break;
                }
                slot_mask |= 1u << member->global_slot;
                ++member_count;
                if (member->lobby_side == 1u) ++left_count;
                else if (member->lobby_side == 2u) ++right_count;
                else valid = false;
            }
        }
        if (valid && member_count == 6u && left_count == 3u &&
            right_count == 3u && slot_mask == 0x3fu) {
            selected = candidate;
            return true;
        }
    }
    return false;
}

void RestoreGroupsLocked(const SourceCohort& source) {
    for (std::vector<QueueGroup>::const_reverse_iterator it =
             source.groups.rbegin(); it != source.groups.rend(); ++it) {
        g_queue.push_front(*it);
    }
}

bool TakeSourceLocked(uint32_t token, SourceCohort& source) {
    std::map<uint32_t, SourceCohort>::iterator found = g_sources.find(token);
    if (found == g_sources.end()) return false;
    source = found->second;
    if (source.room) g_source_rooms.erase(source.room);
    if (source.cohort_id) g_source_cohorts.erase(source.cohort_id);
    g_sources.erase(found);
    return true;
}

bool TakeTargetLocked(uint32_t cohort_id, TargetCohort& target) {
    std::map<uint32_t, TargetCohort>::iterator found =
        g_targets.find(cohort_id);
    if (found == g_targets.end()) return false;
    if (found->second.bootstrap_publish_in_progress) {
        // The publisher owns every raw room/user/party pointer until the last
        // retained START_MAP round has been attempted.  Never let a terminal
        // caller recycle those owners underneath it; one delayed request is
        // enough; the success/failure lease-release helper performs the sole
        // take, unless an already blocked external reset owns final cleanup.
        found->second.abort_pending = true;
        found->second.deadline_at = 0;
        return false;
    }
    target = found->second;
    if (target.room) g_target_rooms.erase(target.room);
    // Detach every CParty lifecycle hook before cleanup calls ReturnToVillage
    // or destroy.  Those ordinary methods re-enter the hook; removing the
    // exact owner record here makes the nested call a no-op and prevents a
    // second cohort abort/PutParty.
    for (size_t slot = 0u; slot < kRoomSeats; ++slot) {
        CParty* const party = target.map_parties[slot];
        if (!party) continue;
        std::map<CParty*, MapPartyOwnerRef>::iterator owner =
            g_target_map_parties.find(party);
        if (owner != g_target_map_parties.end() &&
            owner->second.cohort_id == cohort_id &&
            owner->second.slot == slot &&
            owner->second.party_index == target.map_party_indices[slot]) {
            g_target_map_parties.erase(owner);
        }
    }
    g_targets.erase(found);
    return true;
}

void RollBackRoom(PvP_Room* room, const std::vector<MemberRef>& members) {
    if (!room) return;
    const int room_index = room->get_index();
    for (std::vector<MemberRef>::const_iterator it = members.begin();
         it != members.end(); ++it) {
        CUser* const user = ResolveMember(*it);
        if (user && user->GetPvpIndex() == room_index)
            user->SetPvpIndex(-1);
    }
    room->reset();
    G_CGameManager()->PutPvp(room);
}

int SanitizeTargetUserAfterOwnerDetach(const TargetCohort& target,
                                       size_t slot,
                                       int room_index) {
    CUser* const user = ResolveOriginalTargetMember(target, slot);
    if (!user) return 0;
    const int old_party_index = target.map_party_indices[slot];
    if (old_party_index >= 0 &&
        user->GetPartyIndex() == old_party_index) {
        user->SetPartyIndex(-1);
    }
    if (user->GetPvpIndex() == room_index) user->SetPvpIndex(-1);

    // ReturnToVillage intentionally preserves state while MoveSpace is set.
    // That is valid for ordinary transfers but not for an aborted matching
    // scene.  Only the exact old, now fully detached session is normalized;
    // a user already attached elsewhere is never overwritten by stale work.
    // CUser::GetPartyIndex() zero-extends its internal short, so the detached
    // sentinel -1 is observed as 65535 through that accessor.  CheckInParty()
    // is the original sentinel predicate and must own every detached test.
    if (user->CheckInParty() == 0 && user->CheckInPvp() == 0) {
        if (user->get_state() == 5 && G_GameWorld())
            G_GameWorld()->out_from_dungeon(user);
        if (user->get_state() != 3) user->set_state(3);
    }
    return user->CheckInParty() == 0 && user->CheckInPvp() == 0 &&
                   user->get_state() == 3
               ? 1
               : 2;
}

void ResetClientSceneAfterTargetRollback(const TargetCohort& target,
                                         bool matching_scene_aborted) {
    // CLIENT_EA: the CMD42 success handler at 0x556BF0 consumes only the
    // common success byte and drives the controller through sub_12CF420.  It
    // is the official completion for both the ordinary give-up sender and the
    // client's ten-second loading timeout sender.  Never send it until the
    // server has completed every party/room/user detach, and never use it for
    // a pre-NOTI458 failure or the normal result-settlement path.
    if (!matching_scene_aborted || !target.matching_scene_committed ||
        target.state == TARGET_SETTLING) return;

    size_t sent = 0u;
    size_t send_failed = 0u;
    size_t identity_missing = 0u;
    size_t state_rejected = 0u;
    for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
        if (!target.present[slot]) continue;
        CUser* const user = ResolveOriginalTargetMember(target, slot);
        if (!user) {
            ++identity_missing;
            continue;
        }
        // A reconnected or concurrently reattached character must never
        // receive a stale scene transition from the old cohort.
        if (user->get_state() != 3 || user->CheckInParty() != 0 ||
            user->CheckInPvp() != 0) {
            ++state_rejected;
            continue;
        }
        if (user->SendCmdOkPacket(ENUM_CMDPACKET_GIVEUP_GAME)) ++sent;
        else ++send_failed;
    }

    cMyTrace trace(
        "channel_zombie_matching_authority::ResetClientSceneAfterTargetRollback",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][ROLLBACK_UI_RESET] room=%d cohort=%u load_epoch=%u expected=%u sent=%u send_failed=%u identity_missing=%u state_rejected=%u cmd=42",
          target.room_index,
          static_cast<unsigned int>(target.cohort_id),
          static_cast<unsigned int>(target.load_epoch),
          static_cast<unsigned int>(target.cohort_size),
          static_cast<unsigned int>(sent),
          static_cast<unsigned int>(send_failed),
          static_cast<unsigned int>(identity_missing),
          static_cast<unsigned int>(state_rejected));
}

void ResetDroppedClientSceneAfterFallback(const TargetCohort& snapshot,
                                          size_t missing_slot) {
    CUser* const user =
        ResolveOriginalTargetMember(snapshot, missing_slot);
    bool sent = false;
    const bool clean = user && snapshot.load_epoch != 0u &&
        user->get_state() == 3 && user->CheckInParty() == 0 &&
        user->CheckInPvp() == 0;
    if (clean) {
        sent = user->SendCmdOkPacket(ENUM_CMDPACKET_GIVEUP_GAME);
    }
    cMyTrace trace(
        "channel_zombie_matching_authority::ResetDroppedClientSceneAfterFallback",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][FALLBACK_UI_RESET] room=%d cohort=%u load_epoch=%u missing_slot=%u clean=%d sent=%d cmd=42",
          snapshot.room_index,
          static_cast<unsigned int>(snapshot.cohort_id),
          static_cast<unsigned int>(snapshot.load_epoch),
          static_cast<unsigned int>(missing_slot),
          clean ? 1 : 0,
          sent ? 1 : 0);
}

void RollBackTargetRoom(PvP_Room* room, const TargetCohort& target) {
    if (!room) return;
    const bool matching_scene_aborted = target.state != TARGET_SETTLING;
    const bool map_runtime_terminating =
        zombie_map_runtime::BeginRoomTermination(
            room, target.cohort_id);
    const bool map_runtime_detached =
        !map_runtime_terminating ||
        zombie_map_runtime::DetachRoomAdapters(
            room, target.cohort_id);
    if (map_runtime_terminating && !map_runtime_detached) {
        cMyTrace trace(
            "channel_zombie_matching_authority::RollBackTargetRoom",
            __LINE__, 5);
        trace("[ZOMBIE_MATCH][MAP_RUNTIME_DETACH_FAIL] room=%d generation=%u",
              room->get_index(),
              static_cast<unsigned int>(target.cohort_id));
    }
    // The central room owns pointer-keyed role/round sidecars.  Tear that
    // authority down first, then sever every CUser -> PvP_Room edge before a
    // matching map CParty is allowed to send its ordinary village return.
    zombie_mode::ResetRoom(room);
    const int room_index = room->get_index();
    for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
        if (!target.present[slot]) continue;
        CUser* const user = ResolveOriginalTargetMember(target, slot);
        if (user && user->GetPvpIndex() == room_index)
            user->SetPvpIndex(-1);
    }

    for (size_t slot = 0u; slot < kRoomSeats; ++slot) {
        CParty* const party = target.map_parties[slot];
        if (!party) continue;
        const int expected_party_index = target.map_party_indices[slot];
        CUser* const expected_user =
            slot < target.cohort_size && target.present[slot]
                ? ResolveOriginalTargetMember(target, slot) : 0;
        CUser* const party_user = party->get_user(0);
        if (!expected_user || party_user != expected_user ||
            expected_party_index < 0 ||
            party->GetPartyIndex() != expected_party_index ||
            expected_user->GetPartyIndex() != expected_party_index ||
            party->get_member_count() != 1 ||
            expected_user->GetParty() != party) {
            if (expected_user &&
                expected_user->GetPartyIndex() == expected_party_index) {
                if (expected_user->get_state() == 5 && G_GameWorld())
                    G_GameWorld()->out_from_dungeon(expected_user);
                expected_user->SetPartyIndex(-1);
            }
            cMyTrace trace(
                "channel_zombie_matching_authority::RollBackTargetRoom",
                __LINE__, 5);
            trace("[ZOMBIE_MATCH][MAP_OWNER_RELEASE_FAIL] room=%d slot=%u party=%d",
                  room_index,
                  static_cast<unsigned int>(slot),
                  party->GetPartyIndex());
            continue;
        }
        zombie_preferred_start_runtime::Cancel(&party->battle_field());
        if (party->get_state() == 2 || expected_user->get_state() == 5) {
            party->ReturnToVillage();
        } else {
            expected_user->SetPartyIndex(-1);
            if (expected_user->get_state() != 3) expected_user->set_state(3);
            party->destroy();
        }
    }

    size_t detached_clean = 0u;
    size_t detached_missing = 0u;
    size_t detached_reattached = 0u;
    for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
        if (!target.present[slot]) continue;
        const int result =
            SanitizeTargetUserAfterOwnerDetach(target, slot, room_index);
        if (result == 1) ++detached_clean;
        else if (result == 0) ++detached_missing;
        else ++detached_reattached;
    }
    cMyTrace detach_trace(
        "channel_zombie_matching_authority::RollBackTargetRoom",
        __LINE__, 5);
    detach_trace("[ZOMBIE_MATCH][ROLLBACK_DETACH] room=%d cohort=%u clean=%u identity_missing=%u reattached=%u",
                 room_index,
                 static_cast<unsigned int>(target.cohort_id),
                 static_cast<unsigned int>(detached_clean),
                 static_cast<unsigned int>(detached_missing),
                 static_cast<unsigned int>(detached_reattached));

    if (map_runtime_terminating && map_runtime_detached) {
        zombie_map_runtime::FinishRoomTermination(
            room, target.cohort_id);
    }

    room->reset();
    G_CGameManager()->PutPvp(room);
    ResetClientSceneAfterTargetRollback(target, matching_scene_aborted);
}

bool AbortBootstrapPublishLease(uint32_t cohort_id, PvP_Room* room) {
    TargetCohort failed_target;
    bool release_found = false;
    bool abort_here = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found != g_targets.end() && found->second.room == room &&
            found->second.bootstrap_publish_in_progress) {
            found->second.bootstrap_publish_in_progress = false;
            found->second.abort_pending = true;
            found->second.deadline_at = 0;
            release_found = true;
            // PvP_Room::reset is already blocked at OnRoomReset when this is
            // true.  Leave the exact target registered so that waiter can
            // atomically Take it, release map owners, and let the original
            // reset/PutPvp call run exactly once.
            if (!found->second.external_reset_pending) {
                abort_here = TakeTargetLocked(cohort_id, failed_target);
            }
            pthread_cond_broadcast(&g_bootstrap_publish_cond);
        }
    }
    if (abort_here) {
        QueueTargetAbortControl(failed_target);
        RollBackTargetRoom(failed_target.room, failed_target);
    }
    return release_found;
}

bool FailBootstrapBuild(uint32_t cohort_id, PvP_Room* room) {
    if (!AbortBootstrapPublishLease(cohort_id, room)) {
        cMyTrace trace(
            "channel_zombie_matching_authority::FailBootstrapBuild",
            __LINE__, 5);
        trace("[ZOMBIE_MATCH][PUBLISH_LEASE_ABORT_FAIL] cohort=%u",
              static_cast<unsigned int>(cohort_id));
    }
    return false;
}

bool ReleaseBootstrapPublishLease(uint32_t cohort_id, PvP_Room* room) {
    TargetCohort deferred_abort;
    bool release_found = false;
    bool abort_after_publish = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found != g_targets.end() && found->second.room == room &&
            found->second.bootstrap_publish_in_progress) {
            // End the lease only after every presentation/map send was
            // attempted.  Clearing it before TakeTargetLocked makes this the
            // one legal owner of a delayed terminal request.
            found->second.bootstrap_publish_in_progress = false;
            release_found = true;
            if (found->second.abort_pending &&
                !found->second.external_reset_pending) {
                abort_after_publish =
                    TakeTargetLocked(cohort_id, deferred_abort);
            }
            pthread_cond_broadcast(&g_bootstrap_publish_cond);
        }
    }
    if (abort_after_publish) {
        QueueTargetAbortControl(deferred_abort);
        RollBackTargetRoom(deferred_abort.room, deferred_abort);
    }
    return release_found;
}

void ReleaseTargetMapOwnersForExternalRoomReset(
    PvP_Room* room,
    const TargetCohort& target) {
    if (!room) return;
    const bool map_runtime_terminating =
        zombie_map_runtime::BeginRoomTermination(
            room, target.cohort_id);
    const bool map_runtime_detached =
        !map_runtime_terminating ||
        zombie_map_runtime::DetachRoomAdapters(
            room, target.cohort_id);
    zombie_mode::ResetRoom(room);
    const int room_index = room->get_index();
    for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
        if (!target.present[slot]) continue;
        CUser* const user = ResolveOriginalTargetMember(target, slot);
        if (user && user->GetPvpIndex() == room_index)
            user->SetPvpIndex(-1);
    }
    for (size_t slot = 0u; slot < kRoomSeats; ++slot) {
        CParty* const party = target.map_parties[slot];
        if (!party) continue;
        const int expected_party_index = target.map_party_indices[slot];
        CUser* const expected_user =
            slot < target.cohort_size && target.present[slot]
                ? ResolveOriginalTargetMember(target, slot) : 0;
        CUser* const party_user = party->get_user(0);
        if (!expected_user || party_user != expected_user ||
            expected_party_index < 0 ||
            party->GetPartyIndex() != expected_party_index ||
            expected_user->GetPartyIndex() != expected_party_index ||
            party->get_member_count() != 1 ||
            expected_user->GetParty() != party) {
            if (expected_user &&
                expected_user->GetPartyIndex() == expected_party_index) {
                if (expected_user->get_state() == 5 && G_GameWorld())
                    G_GameWorld()->out_from_dungeon(expected_user);
                expected_user->SetPartyIndex(-1);
            }
            continue;
        }
        zombie_preferred_start_runtime::Cancel(&party->battle_field());
        if (party->get_state() == 2 || expected_user->get_state() == 5) {
            party->ReturnToVillage();
        } else {
            expected_user->SetPartyIndex(-1);
            if (expected_user->get_state() != 3) expected_user->set_state(3);
            party->destroy();
        }
    }
    for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
        if (target.present[slot])
            (void)SanitizeTargetUserAfterOwnerDetach(
                target, slot, room_index);
    }
    if (map_runtime_terminating && map_runtime_detached) {
        zombie_map_runtime::FinishRoomTermination(
            room, target.cohort_id);
    }
}

void TraceLocalPromotionFailure(const char* stage,
                                const SourceCohort& source) {
    CEnvironment* const environment = G_CEnvironment();
    cMyTrace trace(
        "channel_zombie_matching_authority::TraceLocalPromotionFailure",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][LOCAL_PROMOTE_FAIL] stage=%s room=%d members=%u channel=%d",
          stage ? stage : "unknown",
          source.room_index,
          static_cast<unsigned int>(source.members.size()),
          environment ? environment->get_channel_no() : -1);
}

void TraceLocalPromotionPolicyFailure(
    bool mode_ok,
    uint32_t ready_time_ms,
    uint32_t play_time_ms,
    const zombie_mode::ZombiePlayModePolicy& policy,
    size_t member_count) {
    // GetPlayModePolicy leaves its output untouched when no matching record
    // exists.  Never inspect that storage unless mode_ok proves it was filled.
    const uint32_t policy_player =
        mode_ok ? static_cast<uint32_t>(policy.player_count) : 0u;
    const uint32_t policy_initial =
        mode_ok ? static_cast<uint32_t>(policy.initial_zombie_count) : 0u;
    const uint32_t policy_dungeon = mode_ok ? policy.dungeon_index : 0u;
    const uint32_t mother_form =
        mode_ok ? policy.mother_zombie_form_id : 0u;
    const uint32_t infected_form =
        mode_ok ? policy.infected_zombie_form_id : 0u;

    CDataManager* const data_manager = G_CDataManager();
    CDungeon* const dungeon =
        data_manager && policy_dungeon
            ? data_manager->find_dungeon(static_cast<int>(policy_dungeon))
            : 0;
    const bool dungeon_found = dungeon != 0;
    const bool is_zombie = dungeon_found && dungeon->isZombieDungeon();
    const int raw_limit =
        dungeon_found ? dungeon->get_limit_party_count() : -1;

    const char* reason = "validation_failed";
    if (!mode_ok) {
        reason = "mode_missing";
    } else if (!ready_time_ms) {
        reason = "ready_zero";
    } else if (!play_time_ms) {
        reason = "play_zero";
    } else if (policy_player != member_count) {
        reason = "player_mismatch";
    } else if (!policy_initial) {
        reason = "initial_zero";
    } else if (policy_initial > member_count) {
        reason = "initial_gt_members";
    } else if (!policy_dungeon) {
        reason = "dungeon_zero";
    } else if (!mother_form) {
        reason = "mother_form_zero";
    } else if (!infected_form) {
        reason = "infected_form_zero";
    } else if (!data_manager) {
        reason = "data_manager_missing";
    } else if (!dungeon_found) {
        reason = "dungeon_missing";
    } else if (!is_zombie) {
        reason = "not_zombie_dungeon";
    } else if (raw_limit < static_cast<int>(member_count)) {
        reason = "party_limit_lt_members";
    }

    LogManager::logFormat(
        1,
        "channel_zombie_matching_authority.cpp",
        "channel_zombie_matching_authority::TraceLocalPromotionPolicyFailure",
        __LINE__,
        "[ZOMBIE_MATCH][POLICY_DETAIL] mode_ok=%d ready_ok=%d play_ok=%d ready_ms=%u play_ms=%u player=%u init=%u dungeon=%u mother_form=%u infected_form=%u",
        mode_ok ? 1 : 0,
        ready_time_ms ? 1 : 0,
        play_time_ms ? 1 : 0,
        static_cast<unsigned int>(ready_time_ms),
        static_cast<unsigned int>(play_time_ms),
        static_cast<unsigned int>(policy_player),
        static_cast<unsigned int>(policy_initial),
        static_cast<unsigned int>(policy_dungeon),
        static_cast<unsigned int>(mother_form),
        static_cast<unsigned int>(infected_form));
    LogManager::logFormat(
        1,
        "channel_zombie_matching_authority.cpp",
        "channel_zombie_matching_authority::TraceLocalPromotionPolicyFailure",
        __LINE__,
        "[ZOMBIE_MATCH][POLICY_FAIL] reason=%s data_manager_found=%d dungeon_found=%d is_zombie=%d raw_limit=%d member_count=%u",
        reason,
        data_manager ? 1 : 0,
        dungeon_found ? 1 : 0,
        is_zombie ? 1 : 0,
        raw_limit,
        static_cast<unsigned int>(member_count));
}

bool ValidateZombiePlayPolicy(
    const zombie_mode::ZombiePlayModePolicy& policy,
    size_t member_count) {
    if (!IsPlayableCohortSize(member_count) ||
        policy.player_count != member_count ||
        policy.initial_zombie_count == 0u ||
        policy.initial_zombie_count > member_count ||
        policy.dungeon_index == 0u ||
        policy.mother_zombie_form_id == 0u ||
        policy.infected_zombie_form_id == 0u) {
        return false;
    }
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) return false;
    CDungeon* const dungeon = data_manager->find_dungeon(
        static_cast<int>(policy.dungeon_index));
    return dungeon && dungeon->isZombieDungeon() &&
           dungeon->get_limit_party_count() >=
               static_cast<int>(member_count);
}

bool SendMatchingComplete(PvP_Room* room) {
    if (!room) return false;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_MATCHING_COMPLETE);
    // CLIENT_EA: sub_4EF810 reads two u8 values verbatim, stores them as the
    // current matching key, then publishes event 2.  Do not leak the internal
    // authority pair (1, 15) onto this client-facing packet.
    packet.put_byte(kClientWireBound);
    packet.put_byte(kClientWireMode);
    packet.finalize(true);
    bool sent = true;
    for (int seat = 0; seat < 8; ++seat) {
        CUser* const user = room->get_user_seat(seat);
        if (user && user->Send(packet) == 0) sent = false;
    }
    return sent;
}

void BuildZombieStartPermutation(
    std::vector<std::pair<int, int> >& starts) {
    starts.clear();
    starts.push_back(std::make_pair(0, 1));
    starts.push_back(std::make_pair(0, 2));
    starts.push_back(std::make_pair(1, 0));
    starts.push_back(std::make_pair(1, 2));
    starts.push_back(std::make_pair(2, 0));
    starts.push_back(std::make_pair(2, 1));
    for (size_t remaining = starts.size(); remaining > 1u; --remaining) {
        const size_t selected = static_cast<size_t>(std::rand()) % remaining;
        std::swap(starts[selected], starts[remaining - 1u]);
    }
}

void ReleaseUnattachedMapParty(CParty* party, CUser* user) {
    if (!party) return;
    zombie_preferred_start_runtime::Cancel(&party->battle_field());
    if (user && user->GetParty() == party) user->SetPartyIndex(-1);
    party->destroy();
}

bool AttachMapParty(uint32_t cohort_id,
                    size_t slot,
                    PvP_Room* room,
                    CParty* party) {
    if (!party || party->GetPartyIndex() < 0) return false;
    MutexGuard guard(&g_mutex);
    std::map<uint32_t, TargetCohort>::iterator found =
        g_targets.find(cohort_id);
    if (found == g_targets.end() ||
        found->second.state != TARGET_AUTHORIZING ||
        !found->second.authorized || found->second.room != room ||
        slot >= found->second.cohort_size ||
        found->second.map_parties[slot] ||
        g_target_map_parties.find(party) != g_target_map_parties.end()) {
        return false;
    }
    found->second.map_parties[slot] = party;
    found->second.map_party_indices[slot] = party->GetPartyIndex();
    found->second.map_loaded[slot] = false;
    MapPartyOwnerRef owner;
    owner.cohort_id = cohort_id;
    owner.slot = slot;
    owner.party_index = party->GetPartyIndex();
    owner.member = found->second.members[slot];
    g_target_map_parties[party] = owner;
    return true;
}

bool SendZombieOwnerFullActorSnapshots(
    PvP_Room* room,
    const std::vector<CUser*>& users) {
    if (!room || !IsMatchingCohortSize(users.size())) return false;
    bool sent = true;
    for (size_t slot = 0u; slot < users.size(); ++slot) {
        if (!SendZombieOwnerFullActorSnapshot(
                room, users[slot], &users[0], users.size())) {
            sent = false;
        }
    }
    return sent;
}

bool SendZombieOwnerCombatSeatMappings(
    PvP_Room* room,
    const std::vector<CUser*>& users,
    const std::vector<CParty*>& parties) {
    if (!room || users.size() != parties.size() ||
        !IsMatchingCohortSize(users.size())) return false;
    bool sent = true;
    for (size_t slot = 0u; slot < users.size(); ++slot) {
        if (!parties[slot] ||
            !SendZombieOwnerCombatSeatMapping(
                room, users[slot], &users[0], users.size(),
                parties[slot]->GetPartyIndex())) {
            sent = false;
        }
    }
    return sent;
}

bool SendZombieOwnerFullPeerSnapshots(
    PvP_Room* room,
    const std::vector<CUser*>& users) {
    if (!room || !IsMatchingCohortSize(users.size())) return false;
    bool sent = true;
    for (size_t slot = 0u; slot < users.size(); ++slot) {
        if (!SendZombieOwnerFullPeerSnapshot(
                room, users[slot], &users[0], users.size())) {
            sent = false;
        }
    }
    return sent;
}

bool SendZombieConnectP2PReady(const std::vector<CUser*>& users) {
    if (users.size() != static_cast<size_t>(
                            zombie_matching_runtime_policy::
                                kOfficialMatchingMembers)) {
        return false;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CONNECT_P2P_PVP);
    packet.finalize(true);
    bool sent = true;
    for (std::vector<CUser*>::const_iterator user = users.begin();
         user != users.end(); ++user) {
        if (!*user || (*user)->Send(packet) == 0) sent = false;
    }
    return sent;
}

bool SendZombieCompleteLoad(const std::vector<CUser*>& users,
                            CUser* reporter) {
    if (!reporter ||
        users.size() != static_cast<size_t>(
                            zombie_matching_runtime_policy::
                                kOfficialMatchingMembers)) {
        return false;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_COMPLETE_LOAD_PVP);
    packet.put_short(reporter->get_unique_id());
    packet.finalize(true);
    bool sent = true;
    for (std::vector<CUser*>::const_iterator user = users.begin();
         user != users.end(); ++user) {
        if (!*user || (*user)->Send(packet) == 0) sent = false;
    }
    return sent;
}

enum P2PPublishStatus {
    P2P_PUBLISH_NOT_READY = 0,
    P2P_PUBLISH_DONE = 1,
    P2P_PUBLISH_INVALID = 2,
    P2P_PUBLISH_ABORTED = 3,
};

enum P2PLeaseFinalizeStatus {
    P2P_LEASE_FINALIZE_INVALID = 0,
    P2P_LEASE_FINALIZE_RELEASED = 1,
    P2P_LEASE_FINALIZE_STARTING = 2,
    P2P_LEASE_FINALIZE_ABORTED = 3,
};

P2PLeaseFinalizeStatus FinalizeP2PPublishLease(
    uint32_t cohort_id,
    PvP_Room* room,
    uint32_t load_epoch,
    bool& start_now) {
    TargetCohort deferred_abort;
    bool abort_here = false;
    P2PLeaseFinalizeStatus status = P2P_LEASE_FINALIZE_INVALID;
    start_now = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() || found->second.room != room ||
            !found->second.bootstrap_publish_in_progress) {
            return P2P_LEASE_FINALIZE_INVALID;
        }

        TargetCohort& target = found->second;
        const bool identity_valid = target.state == TARGET_LOADING &&
            load_epoch != 0u && target.load_epoch == load_epoch;
        // Clear, decide, and optionally freeze under the same mutex.  If the
        // lease were cleared first and this decision made in a second lock,
        // TickAt could start the room in between and the publisher would then
        // misclassify that successful start as an invalid target.
        target.bootstrap_publish_in_progress = false;
        if (!identity_valid || target.abort_pending ||
            target.external_reset_pending) {
            target.abort_pending = true;
            target.deadline_at = 0;
            status = P2P_LEASE_FINALIZE_ABORTED;
            if (!target.external_reset_pending) {
                abort_here = TakeTargetLocked(cohort_id, deferred_abort);
            }
        } else if (target.loaded_mask == target.expected_mask) {
            start_now = FreezeSixPlayerLoadLocked(target);
            if (start_now) {
                status = P2P_LEASE_FINALIZE_STARTING;
            } else {
                target.abort_pending = true;
                target.deadline_at = 0;
                status = P2P_LEASE_FINALIZE_ABORTED;
                abort_here = TakeTargetLocked(cohort_id, deferred_abort);
            }
        } else {
            status = P2P_LEASE_FINALIZE_RELEASED;
        }
        pthread_cond_broadcast(&g_bootstrap_publish_cond);
    }
    if (abort_here) {
        QueueTargetAbortControl(deferred_abort);
        RollBackTargetRoom(deferred_abort.room, deferred_abort);
    }
    return status;
}

P2PPublishStatus TryPublishZombieP2PBarrier(
    uint32_t cohort_id,
    PvP_Room* room,
    std::time_t now) {
    std::vector<CUser*> users;
    uint32_t load_epoch = 0u;
    uint8_t expected_mask = 0u;
    uint8_t loaded_mask = 0u;
    bool invalid = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() || found->second.room != room) {
            return P2P_PUBLISH_NOT_READY;
        }
        TargetCohort& target = found->second;
        if (target.state != TARGET_CONNECTING ||
            target.p2p_ready_mask != target.p2p_expected_mask ||
            target.bootstrap_publish_in_progress) {
            return P2P_PUBLISH_NOT_READY;
        }
        const uint8_t official_mask = static_cast<uint8_t>(
            (1u << zombie_matching_runtime_policy::
                       kOfficialMatchingMembers) - 1u);
        invalid = !target.authorized || !target.matching_scene_committed ||
            target.abort_pending || target.external_reset_pending ||
            target.cohort_size !=
                zombie_matching_runtime_policy::kOfficialMatchingMembers ||
            target.complete_load_expected_mask != official_mask ||
            target.complete_load_ready_mask != official_mask ||
            target.p2p_expected_mask != official_mask ||
            target.p2p_ready_mask != official_mask || target.load_epoch == 0u ||
            target.expected_mask != official_mask ||
            (target.loaded_mask & static_cast<uint8_t>(~official_mask)) != 0u;
        if (!invalid) {
            for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
                CUser* const user =
                    ResolveOriginalTargetMember(target, slot);
                CParty* const party = target.map_parties[slot];
                std::map<CParty*, MapPartyOwnerRef>::iterator owner =
                    g_target_map_parties.find(party);
                if (!target.present[slot] ||
                    target.members[slot].global_slot != slot || !user ||
                    target.room->get_user_seat(static_cast<int>(slot)) !=
                        user ||
                    user->GetPVPRoom() != target.room ||
                    user->GetPvpIndex() != target.room_index || !party ||
                    target.map_party_indices[slot] < 0 ||
                    party->GetPartyIndex() !=
                        target.map_party_indices[slot] ||
                    party->get_member_count() != 1 ||
                    party->get_user(0) != user || user->GetParty() != party ||
                    owner == g_target_map_parties.end() ||
                    owner->second.cohort_id != cohort_id ||
                    owner->second.slot != slot ||
                    owner->second.party_index !=
                        target.map_party_indices[slot] ||
                    owner->second.load_epoch != target.load_epoch ||
                    owner->second.detaching) {
                    invalid = true;
                    break;
                }
                users.push_back(user);
            }
        }
        if (!invalid) {
            // This lease is acquired before exporting any raw owner pointer.
            // It also makes duplicate final CMD296 reports harmless.
            target.bootstrap_publish_in_progress = true;
            load_epoch = target.load_epoch;
            expected_mask = target.p2p_expected_mask;
            loaded_mask = target.loaded_mask;
            target.state = TARGET_LOADING;
            target.deadline_at = now + kTargetLoadDeadlineSeconds;
        }
    }
    if (invalid) return P2P_PUBLISH_INVALID;

    // CLIENT_ORDER_CONTRACT (0828): retained DUNGEON_INFO/START_MAP activates
    // the controller that emits CMD295.  After CMD295/NOTI278 and every
    // CMD296 report, the empty NOTI279 releases the already-loaded P2P scene.
    const bool ready_sent = SendZombieConnectP2PReady(users);
    cMyTrace trace(
        "channel_zombie_matching_authority::TryPublishZombieP2PBarrier",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][P2P_RELEASE] room=%d cohort=%u ready=0x%02x expected=0x%02x loaded=0x%02x noti279=%d maps_early=1 load_epoch=%u load_timeout=%d",
          room ? room->get_index() : -1,
          static_cast<unsigned int>(cohort_id),
          static_cast<unsigned int>(expected_mask),
          static_cast<unsigned int>(expected_mask),
          static_cast<unsigned int>(loaded_mask),
          ready_sent ? 1 : 0,
          static_cast<unsigned int>(load_epoch),
          kTargetLoadDeadlineSeconds);
    if (!ready_sent) {
        cMyTrace failure_trace(
            "channel_zombie_matching_authority::TryPublishZombieP2PBarrier",
            __LINE__, 5);
        failure_trace("[ZOMBIE_MATCH][P2P_RELEASE_FAIL] room=%d cohort=%u noti279=%d action=abort",
                      room ? room->get_index() : -1,
                      static_cast<unsigned int>(cohort_id),
                      ready_sent ? 1 : 0);
        if (!AbortBootstrapPublishLease(cohort_id, room)) {
            failure_trace("[ZOMBIE_MATCH][P2P_LEASE_ABORT_FAIL] cohort=%u",
                          static_cast<unsigned int>(cohort_id));
            AbortTargetCohort(cohort_id);
        }
        return P2P_PUBLISH_ABORTED;
    }
    bool start_now = false;
    const P2PLeaseFinalizeStatus finalize_status =
        FinalizeP2PPublishLease(cohort_id, room, load_epoch, start_now);
    if (finalize_status == P2P_LEASE_FINALIZE_INVALID) {
        cMyTrace lease_trace(
            "channel_zombie_matching_authority::TryPublishZombieP2PBarrier",
            __LINE__, 5);
        lease_trace("[ZOMBIE_MATCH][P2P_LEASE_FINALIZE_FAIL] cohort=%u",
                    static_cast<unsigned int>(cohort_id));
        AbortTargetCohort(cohort_id);
        return P2P_PUBLISH_INVALID;
    }
    if (finalize_status == P2P_LEASE_FINALIZE_ABORTED) {
        return P2P_PUBLISH_ABORTED;
    }
    if (finalize_status == P2P_LEASE_FINALIZE_STARTING &&
        (!start_now || !StartTargetCohort(cohort_id))) {
        AbortTargetCohort(cohort_id);
        return P2P_PUBLISH_ABORTED;
    }
    return P2P_PUBLISH_DONE;
}

bool BuildZombieMapOwners(
    uint32_t cohort_id,
    PvP_Room* room,
    const std::vector<CUser*>& users,
    const zombie_mode::ZombiePlayModePolicy& policy,
    uint32_t ready_time_ms,
    uint32_t play_time_ms,
    std::time_t now) {
    if (!room ||
        users.size() != static_cast<size_t>(
                            zombie_matching_runtime_policy::
                                kOfficialMatchingMembers) ||
        !ValidateZombiePlayPolicy(policy, users.size()) ||
        policy.dungeon_index != 3101u || !ready_time_ms || !play_time_ms) {
        return FailBootstrapBuild(cohort_id, room);
    }

    std::vector<std::pair<int, int> > starts;
    BuildZombieStartPermutation(starts);
    if (starts.size() < users.size())
        return FailBootstrapBuild(cohort_id, room);

    std::vector<CParty*> parties;
    for (size_t slot = 0u; slot < users.size(); ++slot) {
        CUser* const user = users[slot];
        if (!user || user->get_state() != 3 || user->CheckInParty() ||
            user->GetPVPRoom() != room ||
            user->GetPvpIndex() != room->get_index()) {
            return FailBootstrapBuild(cohort_id, room);
        }
        CParty* const party = G_CGameManager()->GetParty();
        if (!party) return FailBootstrapBuild(cohort_id, room);
        if (party->set_single_play(user) != 1 ||
            party->get_member_count() != 1 || party->get_user(0) != user ||
            user->GetParty() != party || user->GetPVPRoom() != room) {
            ReleaseUnattachedMapParty(party, user);
            return FailBootstrapBuild(cohort_id, room);
        }
        if (!AttachMapParty(cohort_id, slot, room, party)) {
            ReleaseUnattachedMapParty(party, user);
            return FailBootstrapBuild(cohort_id, room);
        }
        parties.push_back(party);
    }

    zombie_map_runtime::SeatBinding map_bindings[kRoomSeats];
    for (size_t slot = 0u; slot < users.size(); ++slot) {
        map_bindings[slot].global_slot = static_cast<uint8_t>(slot);
        map_bindings[slot].user = users[slot];
        map_bindings[slot].party = parties[slot];
        map_bindings[slot].field = &parties[slot]->battle_field();
        map_bindings[slot].party_index = parties[slot]->GetPartyIndex();
    }
    const uint8_t bootstrap_mask = static_cast<uint8_t>(
        (1u << users.size()) - 1u);
    if (!zombie_map_runtime::RegisterBootstrap(
            room,
            room->get_index(),
            cohort_id,
            bootstrap_mask,
            map_bindings,
            users.size())) {
        return FailBootstrapBuild(cohort_id, room);
    }

    // Every singleton CParty remains a hidden server-side map adapter.  Move
    // all six users into the dungeon state without emitting its ordinary
    // one-member NOTI2/NOTI26 presentation before publishing the one central
    // combat roster.  A failure at any point is consumed by the caller's
    // whole-cohort rollback; no partial adapter becomes authoritative.
    for (size_t slot = 0u; slot < users.size(); ++slot) {
        CUser* const user = users[slot];
        CParty* const party = parties[slot];
        if (!party || !party->game_start_zombie_owner(user) ||
            user->get_state() != 5 || user->GetParty() != party ||
            user->GetPVPRoom() != room ||
            user->GetPvpIndex() != room->get_index()) {
            return FailBootstrapBuild(cohort_id, room);
        }
    }

    // Assemble all six hidden map owners before exposing any successful
    // matching presentation.  RouteDungeonInfoPacket/RouteStartMapPacket hold
    // the initial NOTI28/NOTI29 frames during ROOM_BOOTSTRAP.  A SelectDungeon
    // or ConsistMap failure therefore returns to the queue without publishing
    // actor snapshots, NOTI458, combat seats, peers, or map-start frames.
    for (size_t slot = 0u; slot < users.size(); ++slot) {
        CUser* const user = users[slot];
        CParty* const party = parties[slot];
        const std::pair<int, int>& start = starts[slot];
        if (!zombie_preferred_start_runtime::Register(
                &party->battle_field(), start.first, start.second)) {
            return FailBootstrapBuild(cohort_id, room);
        }
        const int dungeon_result = party->dungeon_start(
            static_cast<int>(policy.dungeon_index),
            0,
            static_cast<ENUM_DUNGEON_TYPE>(0));
        zombie_preferred_start_runtime::Cancel(&party->battle_field());
        CDungeon* const dungeon = party->battle_field().GetDungeon();
        int actual_x = -1;
        int actual_y = -1;
        party->battle_field().getCurPosXY(actual_x, actual_y);
        if (dungeon_result != 0 || party->get_state() != 2 ||
            party->get_member_count() != 1 || party->get_user(0) != user ||
            user->get_state() != 5 || user->GetParty() != party ||
            user->GetPVPRoom() != room ||
            user->GetPvpIndex() != room->get_index() || !dungeon ||
            dungeon->get_index() != static_cast<int>(policy.dungeon_index) ||
            actual_x != start.first || actual_y != start.second) {
            return FailBootstrapBuild(cohort_id, room);
        }
    }

    // Freeze the complete NOTI28/NOTI29 set while rollback is still legal.
    // No matching-complete presentation has been emitted at this point.
    if (!zombie_map_runtime::SealBootstrapPackets(room, cohort_id)) {
        return FailBootstrapBuild(cohort_id, room);
    }

    // Finish every fallible ownership validation before the first
    // client-visible frame.  CONNECTING is the visible transaction commit.
    // The load epoch is allocated here because the retained START_MAP must
    // activate the 0828 controller before it can emit CMD295/CMD296.
    const uint8_t p2p_expected_mask = static_cast<uint8_t>(
        (1u << zombie_matching_runtime_policy::kOfficialMatchingMembers) -
        1u);
    uint32_t load_epoch = 0u;
    bool commit_ready = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found != g_targets.end() &&
            found->second.state == TARGET_AUTHORIZING &&
            found->second.authorized && found->second.room == room &&
            found->second.cohort_size ==
                zombie_matching_runtime_policy::kOfficialMatchingMembers &&
            found->second.bootstrap_publish_in_progress &&
            !found->second.abort_pending) {
            commit_ready = true;
            for (size_t slot = 0u; slot < users.size(); ++slot) {
                CParty* const party = found->second.map_parties[slot];
                std::map<CParty*, MapPartyOwnerRef>::iterator owner =
                    g_target_map_parties.find(party);
                if (!found->second.present[slot] ||
                    found->second.members[slot].global_slot != slot ||
                    !party || owner == g_target_map_parties.end() ||
                    owner->second.cohort_id != cohort_id ||
                    owner->second.slot != slot ||
                    owner->second.party_index !=
                        found->second.map_party_indices[slot] ||
                    owner->second.detaching) {
                    commit_ready = false;
                    break;
                }
            }
            if (commit_ready) {
                load_epoch = NextNonZero(g_next_load_epoch);
                for (size_t slot = 0u; slot < users.size(); ++slot) {
                    CParty* const party = found->second.map_parties[slot];
                    std::map<CParty*, MapPartyOwnerRef>::iterator owner =
                        g_target_map_parties.find(party);
                    owner->second.load_epoch = load_epoch;
                    found->second.map_loaded[slot] = false;
                }
                found->second.play_policy = policy;
                found->second.ready_duration_ms = ready_time_ms;
                found->second.play_duration_ms = play_time_ms;
                found->second.ready_deadline_ms = 0u;
                found->second.play_deadline_ms = 0u;
                found->second.complete_load_expected_mask =
                    p2p_expected_mask;
                found->second.complete_load_ready_mask = 0u;
                found->second.p2p_expected_mask = p2p_expected_mask;
                found->second.p2p_ready_mask = 0u;
                found->second.load_epoch = load_epoch;
                found->second.expected_mask = p2p_expected_mask;
                found->second.loaded_mask = 0u;
                found->second.active_mask = 0u;
                found->second.active_count = 0u;
                found->second.degraded_from_six = false;
                std::memset(found->second.global_to_active, 0xff,
                            sizeof(found->second.global_to_active));
                std::memset(found->second.active_to_global, 0xff,
                            sizeof(found->second.active_to_global));
                found->second.matching_scene_committed = true;
                found->second.state = TARGET_CONNECTING;
                found->second.deadline_at =
                    now + kTargetP2PDeadlineSeconds;
            }
        }
    }
    if (!commit_ready) return FailBootstrapBuild(cohort_id, room);

    // CLIENT_ORDER_CONTRACT: the final queue NOTI595 is committed only after
    // CMD518 success.  Once every hidden owner has a valid 3101 map, publish
    // full actors -> MATCHING_COMPLETE -> combat seats -> full peers -> the
    // retained DUNGEON_INFO/START_MAP transaction.  START_MAP activates the
    // client controller; CMD295/NOTI278 and CMD296/NOTI279 follow it.
    const bool actors_sent =
        SendZombieOwnerFullActorSnapshots(room, users);
    const bool complete_sent = SendMatchingComplete(room);
    const bool seats_sent =
        SendZombieOwnerCombatSeatMappings(room, users, parties);
    const bool peers_sent =
        SendZombieOwnerFullPeerSnapshots(room, users);
    const bool maps_sent = actors_sent && complete_sent && seats_sent &&
        peers_sent &&
        zombie_map_runtime::PublishBootstrapPackets(room, cohort_id);
    if (!actors_sent || !complete_sent || !seats_sent || !peers_sent ||
        !maps_sent) {
        cMyTrace publish_trace(
            "channel_zombie_matching_authority::BuildZombieMapOwners",
            __LINE__, 5);
        publish_trace("[ZOMBIE_MATCH][P2P_PRESENT_PARTIAL] room=%d actors=%d complete=%d seats=%d peers=%d maps=%d action=abort",
                      room->get_index(),
                      actors_sent ? 1 : 0,
                      complete_sent ? 1 : 0,
                      seats_sent ? 1 : 0,
                      peers_sent ? 1 : 0,
                      maps_sent ? 1 : 0);
        return FailBootstrapBuild(cohort_id, room);
    }

    cMyTrace trace(
        "channel_zombie_matching_authority::BuildZombieMapOwners",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][MAP_OWNERS] room=%d members=%u dungeon=%u p2p_expected=0x%02x p2p_timeout=%d maps_deferred=0 load_epoch=%u",
          room->get_index(),
          static_cast<unsigned int>(users.size()),
          static_cast<unsigned int>(policy.dungeon_index),
          static_cast<unsigned int>(p2p_expected_mask),
          kTargetP2PDeadlineSeconds,
          static_cast<unsigned int>(load_epoch));
    if (!ReleaseBootstrapPublishLease(cohort_id, room)) {
        cMyTrace lease_trace(
            "channel_zombie_matching_authority::BuildZombieMapOwners",
            __LINE__, 5);
        lease_trace("[ZOMBIE_MATCH][PUBLISH_LEASE_RELEASE_FAIL] cohort=%u",
                    static_cast<unsigned int>(cohort_id));
        AbortTargetCohort(cohort_id);
        return false;
    }
    // A very fast local client may complete CMD296 while the presentation
    // lease is still held.  Re-check after releasing it so the final reporter
    // never needs to retransmit merely to trigger NOTI279.
    const P2PPublishStatus p2p_publish = TryPublishZombieP2PBarrier(
        cohort_id, room, std::time(0));
    if (p2p_publish == P2P_PUBLISH_INVALID) {
        AbortTargetCohort(cohort_id);
        return false;
    }
    if (p2p_publish == P2P_PUBLISH_ABORTED) return false;
    return true;
}

bool BeginZombieSidecarBattle(
    PvP_Room* room,
    const zombie_mode::ZombiePlayModePolicy& policy,
    uint8_t active_mask) {
    if (!room || !ValidateZombiePlayPolicy(
                     policy, static_cast<size_t>(room->get_player_count())) ||
        CountGlobalSlots(active_mask) != policy.player_count ||
        room->get_state() != 1 || !zombie_mode::IsActiveRoom(room)) {
        return false;
    }
    room->lock();
    if (room->battle_state_ != 1 ||
        room->member_count_ != static_cast<int>(policy.player_count)) {
        room->unlock();
        return false;
    }
    for (int seat = 0; seat < 8; ++seat) {
        CUser* const user = room->users_[seat];
        const bool active =
            (active_mask & GlobalSlotBit(static_cast<size_t>(seat))) != 0u;
        if (active) {
            if (!user || user->get_state() != 5 ||
                user->GetPVPRoom() != room) {
                room->unlock();
                return false;
            }
            room->battle_live_flags_[seat] = 1u;
            room->battle_teams_[seat] =
                static_cast<uint8_t>(room->teams_[seat]);
            user->reset_pvp_masterid_walkingout_me();
        } else if (user) {
            room->unlock();
            return false;
        }
    }
    // This is deliberately an eight-seat PvP_Room sidecar, not a fabricated
    // CParty/CDungeon owner.  NOTI458 makes the 140828 client consume the
    // selected matching record and its ZombieMode play-mode dungeon.  The
    // ordinary NOTI45 random-PvP-map path must never run for this room.
    room->battle_player_count_ = policy.player_count;
    room->battle_state_ = 2;
    room->battle_start_time_sec_ = GlobalData::s_systemTime_.getCurSec();
    room->timed_out_ = 0u;
    room->pvp_battle_ended_ = 0u;
    room->unlock();
    return true;
}

bool BuildSourceRoom(SourceCohort& source) {
    std::vector<CUser*> users;
    for (std::vector<MemberRef>::const_iterator it = source.members.begin();
         it != source.members.end(); ++it) {
        CUser* const user = ResolveMember(*it);
        if (!IsQueueEligible(user)) return false;
        users.push_back(user);
    }
    if (!IsMatchingCohortSize(users.size()))
        return false;

    PvP_Room* const room = G_CGameManager()->GetPvp();
    if (!room) return false;
    char empty[1] = {0};
    if (room->create_room(users[0], 1, empty, 0, false, empty, 0,
                          static_cast<PVP_BATTLE_MODE>(2)) > 0) {
        RollBackRoom(room, source.members);
        return false;
    }
    room->change_team(0, 1);
    for (size_t slot = 1u; slot < users.size(); ++slot) {
        int seat = -1;
        if (room->join_room(users[slot], seat, false) > 0 ||
            seat != static_cast<int>(slot)) {
            RollBackRoom(room, source.members);
            return false;
        }
        room->change_team(seat, slot < 3u ? 1 : 2);
    }
    source.room = room;
    source.room_index = room->get_index();
    return true;
}

bool CheckOutPreformedGroups(const SourceCohort& source) {
    for (std::vector<QueueGroup>::const_iterator group =
             source.groups.begin(); group != source.groups.end(); ++group) {
        if (group->party_index < 0) continue;
        // Preserve the ordinary CParty manager invariant: members leave in
        // reverse capture order, which keeps the stored leader until last.
        for (size_t index = group->members.size(); index > 0u; --index) {
            CUser* const user = ResolveMember(group->members[index - 1u]);
            if (!user) return false;
            if (user->CheckInParty() &&
                user->GetPartyIndex() != group->party_index) return false;
            if (user->CheckInParty()) {
                G_CGameManager()->CheckOutParty(user, false);
            }
        }
        for (std::vector<MemberRef>::const_iterator member =
                 group->members.begin(); member != group->members.end();
             ++member) {
            CUser* const user = ResolveMember(*member);
            if (!user || user->CheckInParty()) {
                return false;
            }
        }
    }
    for (std::vector<MemberRef>::const_iterator member =
             source.members.begin(); member != source.members.end();
         ++member) {
        CUser* const user = ResolveMember(*member);
        if (!user || user->CheckInParty()) return false;
    }
    return true;
}

void FillControlBase(Message& request,
                     const MemberRef& leader,
                     uint32_t session_token,
                     uint32_t request_token,
                     uint8_t operation,
                     uint32_t cohort_id,
                     uint8_t cohort_size) {
    std::memset(&request, 0, sizeof(request));
    request.account_id = leader.account_id;
    request.charac_no = leader.charac_no;
    request.session_token = session_token;
    request.request_token = request_token;
    request.field1 = cohort_id;
    request.matching_bound = kBound;
    request.matching_mode = kMode;
    request.operation = operation;
    request.result = matching_channel_handoff_db::RESULT_SUCCESS;
    request.reply_required = 1u;
    request.cohort_size = cohort_size;
}

bool QueueControl(const MemberRef& leader, const Message& request) {
    CUser* const user = ResolveMember(leader);
    return user && matching_channel_handoff_db::DBMatchingChannelHandoff::
                       QueueRequest(user->GetUID(), request);
}

bool QueueRouteRequest(uint32_t token, std::time_t now) {
    Message request;
    MemberRef leader;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, SourceCohort>::iterator found =
            g_sources.find(token);
        if (found == g_sources.end() ||
            found->second.state != SOURCE_BUILDING ||
            !found->second.room || found->second.pending_request_token) {
            return false;
        }
        SourceCohort& source = found->second;
        const uint32_t request_token =
            NextNonZero(source.next_request_token);
        FillControlBase(request, source.leader, source.session_token,
                        request_token,
                        matching_channel_handoff_db::OPERATION_ROUTE_COHORT,
                        0u, static_cast<uint8_t>(source.members.size()));
        CEnvironment* const environment = G_CEnvironment();
        if (!environment) return false;
        request.source_server_group = environment->get_server_group();
        request.source_channel = environment->get_channel_no();
        source.source_server_group = request.source_server_group;
        source.source_channel = request.source_channel;
        ++source.route_attempts;
        source.pending_request_token = request_token;
        source.state = SOURCE_ROUTE_PENDING;
        source.deadline_at = now + kRouteResponseDeadlineSeconds;
        leader = source.leader;
    }
    if (QueueControl(leader, request)) return true;

    MutexGuard guard(&g_mutex);
    std::map<uint32_t, SourceCohort>::iterator found = g_sources.find(token);
    if (found != g_sources.end() &&
        found->second.pending_request_token == request.request_token) {
        found->second.pending_request_token = 0u;
        found->second.state = SOURCE_BUILDING;
    }
    return false;
}

void QueueAbortControl(const SourceCohort& source) {
    if (!source.cohort_id) return;
    Message request;
    FillControlBase(request, source.leader, source.session_token,
                    source.next_request_token,
                    matching_channel_handoff_db::OPERATION_ABORT_COHORT,
                    source.cohort_id,
                    static_cast<uint8_t>(source.members.size()));
    request.source_server_group = source.source_server_group;
    request.source_channel = source.source_channel;
    if (IsServerGroup(source.route.target_server_group) &&
        IsChannel(source.route.target_channel)) {
        request.target_server_group = source.route.target_server_group;
        request.target_channel = source.route.target_channel;
        request.field0 = source.route.field0;
        request.endpoint_key = source.route.endpoint_key;
        request.endpoint_port_like = source.route.endpoint_port;
    }
    (void)QueueControl(source.leader, request);
}

void QueueTargetAbortControl(const TargetCohort& target) {
    if (target.local_same_channel || !target.cohort_id ||
        !target.cohort_size || !target.present[0] ||
        !target.handoffs[0].handoff_id) return;
    Message request;
    FillControlBase(request, target.members[0], target.session_token,
                    target.next_request_token,
                    matching_channel_handoff_db::OPERATION_ABORT_COHORT,
                    target.cohort_id, target.cohort_size);
    request.source_server_group = target.source_server_group;
    request.source_channel = target.source_channel;
    request.target_server_group = target.target_server_group;
    request.target_channel = target.target_channel;
    request.field0 = target.field0;
    request.endpoint_key = target.endpoint_key;
    request.endpoint_port_like = target.endpoint_port_like;
    request.handoff_id = target.handoffs[0].handoff_id;
    request.field4 = target.handoffs[0].handoff_id;
    request.state = matching_channel_handoff_db::STATE_CLAIMED;
    request.source_cleaned = 1u;
    (void)QueueControl(target.members[0], request);
}

void AbortSourceToken(uint32_t token) {
    SourceCohort source;
    {
        MutexGuard guard(&g_mutex);
        if (!TakeSourceLocked(token, source)) return;
    }
    QueueAbortControl(source);
    for (std::vector<MemberRef>::const_iterator it = source.members.begin();
         it != source.members.end(); ++it) {
        CUser* const user = ResolveMember(*it);
        if (user) matching_channel_handoff::ResetSession(user);
    }
    matching_channel_handoff::UnbindMatchingRoom(source.room);
    RollBackRoom(source.room, source.members);
}

void RequeueSourceToken(uint32_t token, std::time_t now) {
    SourceCohort source;
    LobbyUiUpdates ui_updates;
    {
        MutexGuard guard(&g_mutex);
        if (!TakeSourceLocked(token, source)) return;
    }
    matching_channel_handoff::UnbindMatchingRoom(source.room);
    RollBackRoom(source.room, source.members);
    {
        MutexGuard guard(&g_mutex);
        RestoreGroupsLocked(source);
        PurgeInvalidQueueLocked(ui_updates);
        g_route_retry_at = now + kRouteRetrySeconds;
    }
    DispatchLobbyUiUpdates(ui_updates);
}

void AbortTargetCohort(uint32_t cohort_id) {
    TargetCohort target;
    {
        MutexGuard guard(&g_mutex);
        if (!TakeTargetLocked(cohort_id, target)) return;
    }
    QueueTargetAbortControl(target);
    RollBackTargetRoom(target.room, target);
}

void AbortZombieMapRuntimeRoom(PvP_Room* room,
                               uint32_t generation,
                               const char* reason) {
    uint32_t cohort_id = 0u;
    {
        MutexGuard guard(&g_mutex);
        std::map<PvP_Room*, uint32_t>::const_iterator mapped =
            g_target_rooms.find(room);
        if (mapped != g_target_rooms.end() &&
            mapped->second == generation) {
            std::map<uint32_t, TargetCohort>::const_iterator target =
                g_targets.find(generation);
            if (target != g_targets.end() && target->second.room == room)
                cohort_id = generation;
        }
    }
    cMyTrace trace(
        "channel_zombie_matching_authority::AbortZombieMapRuntimeRoom",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][MAP_RUNTIME_ABORT] room=%d generation=%u reason=%s accepted=%d",
          room ? room->get_index() : -1,
          static_cast<unsigned int>(generation),
          reason ? reason : "unknown",
          cohort_id ? 1 : 0);
    if (cohort_id) AbortTargetCohort(cohort_id);
}

bool PromoteSourceRoomToLocalTarget(uint32_t source_token,
                                    std::time_t now) {
    SourceCohort source;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, SourceCohort>::const_iterator found =
            g_sources.find(source_token);
        if (found == g_sources.end() ||
            found->second.state != SOURCE_BUILDING ||
            !found->second.room) {
            return false;
        }
        source = found->second;
    }

    CEnvironment* const environment = G_CEnvironment();
    if (!environment ||
        !IsServerGroup(environment->get_server_group()) ||
        !IsChannel(environment->get_channel_no()) ||
        !IsMatchingCohortSize(source.members.size())) {
        return false;
    }

    std::vector<CUser*> users;
    for (std::vector<MemberRef>::const_iterator member =
             source.members.begin(); member != source.members.end();
         ++member) {
        CUser* const user = ResolveMember(*member);
        if (!IsBuiltSourceRoomMember(user, source.room)) {
            TraceLocalPromotionFailure("built_member", source);
            return false;
        }
        users.push_back(user);
    }

    zombie_mode::ZombiePlayModePolicy play_policy;
    const uint32_t ready_time_ms = zombie_mode::GetReadyTimeMs();
    const uint32_t play_time_ms = zombie_mode::GetPlayTimeMs();
    const bool mode_ok = zombie_mode::GetPlayModePolicy(
        static_cast<uint8_t>(users.size()), play_policy);
    if (!mode_ok ||
        !ready_time_ms || !play_time_ms ||
        !ValidateZombiePlayPolicy(play_policy, users.size())) {
        TraceLocalPromotionPolicyFailure(
            mode_ok, ready_time_ms, play_time_ms, play_policy, users.size());
        TraceLocalPromotionFailure("policy", source);
        return false;
    }

    TargetCohort target;
    target.cohort_id = source.session_token;
    target.cohort_size = static_cast<uint8_t>(users.size());
    target.session_token = source.session_token;
    target.state = TARGET_AUTHORIZING;
    target.room = source.room;
    target.room_index = source.room_index;
    target.local_same_channel = true;
    target.source_server_group = environment->get_server_group();
    target.source_channel = environment->get_channel_no();
    target.target_server_group = target.source_server_group;
    target.target_channel = target.source_channel;
    target.field0 = static_cast<uint8_t>(target.target_server_group);
    target.deadline_at = now + kTargetAuthorizeDeadlineSeconds;
    // Publishing this target and acquiring its raw-owner lease are one locked
    // transaction below; no timeout/cancel path can observe an unleased room.
    target.bootstrap_publish_in_progress = true;
    for (size_t slot = 0u; slot < source.members.size(); ++slot) {
        target.present[slot] = true;
        target.members[slot] = source.members[slot];
        target.original_users[slot] = users[slot];
    }

    bool promoted = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, SourceCohort>::const_iterator found =
            g_sources.find(source_token);
        if (found != g_sources.end() &&
            found->second.state == SOURCE_BUILDING &&
            found->second.room == source.room &&
            g_targets.find(target.cohort_id) == g_targets.end()) {
            SourceCohort consumed;
            if (TakeSourceLocked(source_token, consumed)) {
                g_targets[target.cohort_id] = target;
                g_target_rooms[target.room] = target.cohort_id;
                promoted = true;
            }
        }
    }
    if (!promoted) {
        TraceLocalPromotionFailure("promote_map", source);
        return false;
    }

    bool activated =
        channel_zombie_matching_producer::ActivateTargetRoom(target.room);
    if (!activated) {
        TraceLocalPromotionFailure("activate", source);
        (void)AbortBootstrapPublishLease(target.cohort_id, target.room);
    } else if (!BuildZombieMapOwners(
                   target.cohort_id, target.room, users, play_policy,
                   ready_time_ms, play_time_ms, now)) {
        TraceLocalPromotionFailure("map_owners", source);
        activated = false;
    }
    bool post_invariant_failed = false;
    bool publish_room_state = false;
    PacketGuard state_packet;
    if (activated) {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(target.cohort_id);
        if (found == g_targets.end() || !found->second.authorized ||
            !found->second.local_same_channel ||
            found->second.room != target.room ||
            found->second.state < TARGET_CONNECTING) {
            post_invariant_failed = true;
        } else {
            // Materialize NOTI42 while the exact target entry still owns this
            // PvP_Room.  Once g_mutex is released a terminal path may reset
            // and pool the room, so only the independent packet may escape.
            found->second.room->make_state_info(
                reinterpret_cast<char*>(&state_packet));
            publish_room_state = state_packet.is_finallized();
            if (!publish_room_state) post_invariant_failed = true;
        }
    }
    if (post_invariant_failed)
        TraceLocalPromotionFailure("post_invariant", source);

    if (!activated) {
        LobbyUiUpdates ui_updates;
        // Activate failure releases/takes through AbortBootstrapPublishLease;
        // every BuildZombieMapOwners pre-commit failure does the same through
        // FailBootstrapBuild.  Never Take the target a second time here.
        {
            MutexGuard guard(&g_mutex);
            RestoreGroupsLocked(source);
            PurgeInvalidQueueLocked(ui_updates);
            g_route_retry_at = now + kRouteRetrySeconds;
        }
        DispatchLobbyUiUpdates(ui_updates);
        return false;
    }

    // A terminal owner may remove the committed target while transport is in
    // progress.  In that case the room can already be returning to its pool;
    // never dereference the stale snapshot merely to publish observer state.
    if (publish_room_state) {
        G_GameWorld()->send_all(state_packet);
    }
    cMyTrace trace(
        "channel_zombie_matching_authority::PromoteSourceRoomToLocalTarget",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][LOCAL_COHORT] channel=%d members=%u dungeon=%u connecting=1 ready_ms=%u play_ms=%u",
          target.target_channel,
          static_cast<unsigned int>(target.cohort_size),
          static_cast<unsigned int>(play_policy.dungeon_index),
          static_cast<unsigned int>(ready_time_ms),
          static_cast<unsigned int>(play_time_ms));
    return true;
}

uint32_t TakeQueuedCohortLocked(std::time_t now) {
    std::vector<size_t> selected;
    if (!SelectCohortLocked(selected)) return 0u;
    SourceCohort source;
    source.session_token = NextNonZero(g_next_authority_token);
    source.deadline_at = now + kSourceBuildDeadlineSeconds;
    source.leader = g_queue[selected.front()].leader;
    for (std::vector<size_t>::const_iterator index = selected.begin();
         index != selected.end(); ++index) {
        source.groups.push_back(g_queue[*index]);
        source.members.insert(source.members.end(),
                              g_queue[*index].members.begin(),
                              g_queue[*index].members.end());
    }
    // Room seats, NOTI595 sides and the later 3+3 team assignment share the
    // same immutable six-slot order, independent of QueueGroup insertion
    // order.  The groups themselves stay intact in source.groups for restore.
    std::sort(source.members.begin(), source.members.end(), LobbySlotLess());
    std::vector<size_t> erase_indices = selected;
    std::sort(erase_indices.begin(), erase_indices.end());
    for (std::vector<size_t>::reverse_iterator index = erase_indices.rbegin();
         index != erase_indices.rend(); ++index) {
        g_queue.erase(g_queue.begin() + *index);
    }
    const uint32_t cohort_token = source.session_token;
    g_sources[cohort_token] = source;
    // Draining one ready cohort must not put another complete six-user lobby to
    // sleep.  Launch at most one per Tick, then schedule the next cheap pass.
    std::vector<size_t> remaining;
    g_route_retry_at = SelectCohortLocked(remaining)
                           ? now + kQueueDrainSeconds : 0;
    return cohort_token;
}

void LaunchSourceCohort(uint32_t cohort_token, std::time_t now) {
    if (!cohort_token) return;
    SourceCohort built;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, SourceCohort>::const_iterator found =
            g_sources.find(cohort_token);
        if (found == g_sources.end()) return;
        built = found->second;
    }
    if (!BuildSourceRoom(built)) {
        MutexGuard guard(&g_mutex);
        SourceCohort failed;
        if (TakeSourceLocked(cohort_token, failed)) {
            RestoreGroupsLocked(failed);
            g_route_retry_at = now + kRouteRetrySeconds;
        }
        return;
    }

    bool attached = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, SourceCohort>::iterator found =
            g_sources.find(cohort_token);
        if (found != g_sources.end() &&
            found->second.state == SOURCE_BUILDING) {
            found->second.room = built.room;
            found->second.room_index = built.room_index;
            g_source_rooms[built.room] = cohort_token;
            attached = true;
        }
    }
    if (!attached) {
        RollBackRoom(built.room, built.members);
        return;
    }
    // ZombieMode.etc selects Zombi2.dgn (dungeon.lst index 3101) after
    // NOTI458.  Matching is a
    // same-channel queue: keep the already materialized PvP_Room in this
    // Type19 process and enter its dungeon instead of routing to ch187/188.
    if (!PromoteSourceRoomToLocalTarget(cohort_token, now)) {
        SourceCohort failed;
        {
            MutexGuard guard(&g_mutex);
            if (TakeSourceLocked(cohort_token, failed)) {
                RestoreGroupsLocked(failed);
                g_route_retry_at = now + kRouteRetrySeconds;
            }
        }
        RollBackRoom(failed.room, failed.members);
    }
}

bool Enqueue(CUser* user, uint8_t bound, uint16_t mode) {
    if (bound != kBound || mode != kMode) return false;
    QueueGroup requested;
    if (!CaptureQueueGroup(user, requested)) {
        TraceQueueEligibilityReject(user);
        return false;
    }

    uint32_t cohort_token = 0u;
    bool accepted = false;
    LobbyUiUpdates ui_updates;
    {
        MutexGuard guard(&g_mutex);
        if (!g_started) return false;
        // One accepted CMD518 owns exactly one post-ACK publication.  The
        // dispatcher commits it synchronously before another request from the
        // same live CUser can be accepted.
        if (g_pending_enqueue_commits.find(user) !=
            g_pending_enqueue_commits.end()) {
            return false;
        }
        PurgeInvalidQueueLocked(ui_updates);
        bool already_queued = false;
        const QueueGroup* queued_group = 0;
        bool rejected = false;
        for (std::deque<QueueGroup>::const_iterator it = g_queue.begin();
             it != g_queue.end(); ++it) {
            if (!GroupOverlaps(*it, requested)) continue;
            if (!SameQueueGroup(*it, requested)) {
                rejected = true;
                break;
            }
            already_queued = true;
            queued_group = &*it;
            break;
        }
        if (!rejected) {
            for (std::vector<MemberRef>::const_iterator it =
                     requested.members.begin(); it != requested.members.end();
                 ++it) {
                if (MemberInManagedCohortLocked(*it)) {
                    rejected = true;
                    break;
                }
            }
        }

        if (!rejected && already_queued && queued_group) {
            if (!QueueCanonicalLobbyReadyLocked(
                    queued_group->members[0].lobby_token, ui_updates)) {
                rejected = true;
            } else {
                accepted = true;
            }
        } else if (!rejected && !already_queued) {
            if (g_queue.size() >= kMaximumQueuedGroups ||
                !AssignLobbyPlacementLocked(requested)) {
                rejected = true;
            } else {
                const uint32_t lobby_token =
                    requested.members[0].lobby_token;
                g_queue.push_back(requested);
                if (!QueueCanonicalLobbyReadyLocked(lobby_token,
                                                    ui_updates)) {
                    g_queue.pop_back();
                    rejected = true;
                } else {
                    accepted = true;
                }
            }
        }

        if (accepted) {
            const std::time_t now = std::time(0);
            if (g_route_retry_at == 0 || now >= g_route_retry_at)
                cohort_token = TakeQueuedCohortLocked(now);

            PendingEnqueueCommit pending;
            pending.requester = MakeMember(user);
            pending.ui_updates = ui_updates;
            pending.cohort_token = cohort_token;
            g_pending_enqueue_commits[user] = pending;
        }
    }
    if (accepted) return true;

    // Eligibility purges discovered by a rejected request still publish their
    // NOTI596 snapshots.  Accepted requests publish nothing until the CMD518
    // success response has cleared the client's ready-user list.
    DispatchLobbyUiUpdates(ui_updates);
    return false;
}

bool CommitEnqueue(CUser* user, uint8_t bound, uint16_t mode) {
    if (!user || bound != kBound || mode != kMode) return false;

    PendingEnqueueCommit pending;
    {
        MutexGuard guard(&g_mutex);
        std::map<CUser*, PendingEnqueueCommit>::iterator found =
            g_pending_enqueue_commits.find(user);
        if (!g_started || found == g_pending_enqueue_commits.end() ||
            !SameMember(found->second.requester, MakeMember(user))) {
            return false;
        }
        pending = found->second;
        g_pending_enqueue_commits.erase(found);
    }

    // CLIENT_ORDER_CONTRACT: accepted CMD518 has already been sent by the
    // dispatcher.  Its handler clears the ready list, so only now may the full
    // canonical NOTI595 snapshot rebuilds that list for every lobby member.
    // The sixth member's final NOTI595 must still succeed before room
    // creation/NOTI458; a partial UI publication is never allowed to launch.
    if (!DispatchLobbyUiUpdates(pending.ui_updates)) {
        LogManager::logFormat(
            1,
            "channel_zombie_matching_authority.cpp",
            "channel_zombie_matching_authority::CommitEnqueue",
            __LINE__,
            "[ZOMBIE_MATCH][LOBBY_READY_COMMIT_FAIL] ready_batches=%u cohort_token=%u launch_blocked=1",
            static_cast<unsigned int>(pending.ui_updates.ready.size()),
            static_cast<unsigned int>(pending.cohort_token));
        return false;
    }
    LaunchSourceCohort(pending.cohort_token, std::time(0));
    return true;
}

bool CancelMember(CUser* user, bool require_pending_enqueue) {
    if (!user) return false;
    const MemberRef member = MakeMember(user);
    uint32_t source_token = 0u;
    uint32_t target_cohort = 0u;
    bool pending_cleared = false;
    bool queue_cancelled = false;
    bool cancel_accepted = false;
    LobbyUiUpdates ui_updates;
    {
        MutexGuard guard(&g_mutex);
        std::map<CUser*, PendingEnqueueCommit>::iterator pending =
            g_pending_enqueue_commits.find(user);
        if (require_pending_enqueue &&
            (pending == g_pending_enqueue_commits.end() ||
             !SameMember(pending->second.requester, member))) {
            return false;
        }
        if (pending != g_pending_enqueue_commits.end()) {
            // Ready updates were never published before the CMD518 ACK, so an
            // abort must discard them.  Preserve only unrelated NOTI596 work
            // captured by the enqueue-time invalid-member purge.
            if (SameMember(pending->second.requester, member)) {
                ui_updates.cancel.insert(
                    ui_updates.cancel.end(),
                    pending->second.ui_updates.cancel.begin(),
                    pending->second.ui_updates.cancel.end());
            }
            g_pending_enqueue_commits.erase(pending);
            pending_cleared = true;
        }
        for (size_t index = 0u; index < g_queue.size(); ++index) {
            if (ContainsMember(g_queue[index].members, member)) {
                QueueLobbyCancelForGroupLocked(g_queue[index], ui_updates);
                g_queue.erase(g_queue.begin() + index);
                queue_cancelled = true;
                break;
            }
        }
        if (!queue_cancelled) {
            for (std::map<uint32_t, SourceCohort>::const_iterator source =
                     g_sources.begin(); source != g_sources.end(); ++source) {
                if (!ContainsMember(source->second.members, member)) continue;
                if (source->second.state == SOURCE_ARMED) {
                    cancel_accepted = true;
                } else {
                    source_token = source->first;
                    QueueLobbyCancelForMembers(source->second.members,
                                               ui_updates);
                }
                break;
            }
        }
        if (!queue_cancelled && !source_token && !cancel_accepted) {
            for (std::map<uint32_t, TargetCohort>::const_iterator target =
                     g_targets.begin(); target != g_targets.end(); ++target) {
                for (size_t slot = 0u; slot < target->second.cohort_size;
                     ++slot) {
                    if (target->second.present[slot] &&
                        SameMember(target->second.members[slot], member)) {
                        // CMD519 is a queue cancel.  Once the target scene is
                        // materialized, a stale town-state cancel cannot tear
                        // down the other five users.  State 5 is the actual
                        // matching-dungeon exit/reset path (CMD520 or session
                        // teardown) and remains whole-cohort atomic.
                        if (target->second.state >= TARGET_READY &&
                            user->get_state() != 5) {
                            cancel_accepted = true;
                        } else {
                            target_cohort = target->first;
                        }
                        break;
                    }
                }
                if (target_cohort || cancel_accepted) break;
            }
        }
    }
    DispatchLobbyUiUpdates(ui_updates);
    if (source_token) AbortSourceToken(source_token);
    if (target_cohort) AbortTargetCohort(target_cohort);
    return pending_cleared || queue_cancelled || cancel_accepted ||
           source_token != 0u || target_cohort != 0u;
}

bool AbortEnqueue(CUser* user, uint8_t bound, uint16_t mode) {
    if (!user || bound != kBound || mode != kMode) return false;
    // Requiring the matching pending identity prevents a late failure callback
    // from canceling a newer queue generation owned by a recycled CUser slot.
    return CancelMember(user, true);
}

bool Cancel(CUser* user, uint8_t bound, uint16_t mode) {
    if (!user || bound != kBound || mode != kMode) return false;
    // CMD519, CMD520 and disconnect all consume a pending enqueue as part of
    // the same idempotent cancellation.  A later ACK-failure abort then sees
    // no transaction and cannot launch its staged cohort.
    return CancelMember(user, false);
}

bool SelectTargetRoute(PvP_Room* source_room,
                       CUser* const* members,
                       size_t member_count,
                       TargetRoute* route) {
    if (!source_room || !members || !route ||
        !IsMatchingCohortSize(member_count)) return false;
    MutexGuard guard(&g_mutex);
    std::map<PvP_Room*, uint32_t>::const_iterator room =
        g_source_rooms.find(source_room);
    if (room == g_source_rooms.end()) return false;
    std::map<uint32_t, SourceCohort>::iterator source =
        g_sources.find(room->second);
    if (source == g_sources.end() ||
        source->second.state != SOURCE_ROUTE_READY ||
        source->second.members.size() != member_count ||
        !source->second.cohort_id ||
        std::difftime(std::time(0), source->second.route_cached_at) < 0.0 ||
        std::difftime(std::time(0), source->second.route_cached_at) >
            kRouteCacheLifetimeSeconds) {
        return false;
    }
    for (size_t index = 0u; index < member_count; ++index) {
        if (!members[index] ||
            !SameMember(MakeMember(members[index]),
                        source->second.members[index])) return false;
    }
    *route = source->second.route;
    // Publish consumes this cache entry exactly once.  Move to the handoff
    // state before any per-user RESERVE can reach the DB response thread.
    source->second.state = SOURCE_HANDOFF_PENDING;
    source->second.deadline_at =
        std::time(0) + kSourceHandoffDeadlineSeconds;
    return true;
}

void AbortSourceRoom(PvP_Room* source_room) {
    uint32_t token = 0u;
    {
        MutexGuard guard(&g_mutex);
        std::map<PvP_Room*, uint32_t>::const_iterator found =
            g_source_rooms.find(source_room);
        if (found != g_source_rooms.end()) token = found->second;
    }
    if (token) AbortSourceToken(token);
}

bool SameClaimRoute(const TargetCohort& target,
                    const ClaimedHandoff& handoff) {
    return target.cohort_id == handoff.cohort_id &&
           target.cohort_size == handoff.cohort_size &&
           target.source_server_group == handoff.source_server_group &&
           target.source_channel == handoff.source_channel &&
           target.target_server_group == handoff.target_server_group &&
           target.target_channel == handoff.target_channel &&
           target.field0 == handoff.field0 &&
           target.endpoint_key == handoff.endpoint_key &&
           target.endpoint_port_like == handoff.endpoint_port_like;
}

bool QueueActivate(uint32_t cohort_id) {
    Message request;
    MemberRef leader;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_COLLECTING ||
            found->second.pending_request_token ||
            !found->second.present[0]) return false;
        TargetCohort& target = found->second;
        for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
            if (!target.present[slot]) return false;
        }
        const uint32_t request_token =
            NextNonZero(target.next_request_token);
        FillControlBase(request, target.members[0], target.session_token,
                        request_token,
                        matching_channel_handoff_db::
                            OPERATION_ACTIVATE_COHORT,
                        target.cohort_id, target.cohort_size);
        request.source_server_group = target.source_server_group;
        request.source_channel = target.source_channel;
        request.target_server_group = target.target_server_group;
        request.target_channel = target.target_channel;
        request.field0 = target.field0;
        request.endpoint_key = target.endpoint_key;
        request.endpoint_port_like = target.endpoint_port_like;
        request.handoff_id = target.handoffs[0].handoff_id;
        request.field4 = target.handoffs[0].handoff_id;
        request.state = matching_channel_handoff_db::STATE_CLAIMED;
        request.source_cleaned = 1u;
        target.pending_request_token = request_token;
        target.state = TARGET_ACTIVATE_PENDING;
        target.deadline_at =
            std::time(0) + kControlResponseDeadlineSeconds;
        leader = target.members[0];
    }
    if (QueueControl(leader, request)) return true;
    MutexGuard guard(&g_mutex);
    std::map<uint32_t, TargetCohort>::iterator found =
        g_targets.find(cohort_id);
    if (found != g_targets.end() &&
        found->second.pending_request_token == request.request_token) {
        found->second.pending_request_token = 0u;
        found->second.state = TARGET_COLLECTING;
    }
    return false;
}

bool AcceptClaim(CUser* user, const ClaimedHandoff& handoff) {
    CEnvironment* const environment = G_CEnvironment();
    if (!user || !environment || handoff.cohort_id == 0u ||
        !IsMatchingCohortSize(handoff.cohort_size) ||
        handoff.cohort_slot >= handoff.cohort_size ||
        (handoff.cohort_flags &
             ~matching_channel_handoff_db::COHORT_FLAG_ALL) != 0u ||
        handoff.matching_bound != kBound || handoff.matching_mode != kMode ||
        handoff.target_server_group != environment->get_server_group() ||
        handoff.target_channel != environment->get_channel_no() ||
        handoff.field0 != static_cast<uint8_t>(
                              handoff.target_server_group) ||
        !IsQueueEligible(user)) {
        return false;
    }

    const MemberRef member = MakeMember(user);
    bool complete = false;
    {
        MutexGuard guard(&g_mutex);
        TargetCohort& target = g_targets[handoff.cohort_id];
        if (!target.cohort_id) {
            target.cohort_id = handoff.cohort_id;
            target.cohort_size = handoff.cohort_size;
            target.cohort_flags = handoff.cohort_flags;
            target.session_token = NextNonZero(g_next_authority_token);
            target.source_server_group = handoff.source_server_group;
            target.source_channel = handoff.source_channel;
            target.target_server_group = handoff.target_server_group;
            target.target_channel = handoff.target_channel;
            target.field0 = handoff.field0;
            target.endpoint_key = handoff.endpoint_key;
            target.endpoint_port_like = handoff.endpoint_port_like;
            // Fixed from the first durable claim; later partial claims do not
            // extend this window and therefore cannot pin the cohort forever.
            target.deadline_at =
                std::time(0) + kTargetClaimDeadlineSeconds;
        } else if (!SameClaimRoute(target, handoff) ||
                   target.state != TARGET_COLLECTING) {
            return false;
        }
        target.cohort_flags |= handoff.cohort_flags;
        const size_t slot = handoff.cohort_slot;
        if (target.present[slot]) {
            return SameMember(target.members[slot], member) &&
                   target.original_users[slot] == user &&
                   target.handoffs[slot].handoff_id == handoff.handoff_id;
        }
        for (size_t other = 0u; other < target.cohort_size; ++other) {
            if (target.present[other] &&
                SameMember(target.members[other], member)) return false;
        }
        target.present[slot] = true;
        target.members[slot] = member;
        target.original_users[slot] = user;
        target.handoffs[slot] = handoff;
        complete = true;
        for (size_t index = 0u; index < target.cohort_size; ++index) {
            if (!target.present[index]) {
                complete = false;
                break;
            }
        }
        complete = complete &&
            (target.cohort_flags &
                 matching_channel_handoff_db::COHORT_FLAG_ALL_CLAIMED) != 0u;
    }
    if (!complete) return true;
    if (QueueActivate(handoff.cohort_id)) return true;
    AbortTargetCohort(handoff.cohort_id);
    return false;
}

void SendCreatorRoomPackets(CUser* creator, PvP_Room* room) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(1);
    packet.put_short(1);
    creator->make_basic_info(packet, 1);
    packet.finalize(true);
    creator->Send(packet);

    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_ROOM_INFO);
    packet.put_short(1);
    room->make_room_info(reinterpret_cast<char*>(&packet));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

bool BuildTargetRoom(uint32_t cohort_id) {
    TargetCohort snapshot;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::const_iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_AUTHORIZING) return false;
        snapshot = found->second;
    }

    std::vector<CUser*> users;
    for (size_t slot = 0u; slot < snapshot.cohort_size; ++slot) {
        CUser* const user = ResolveMember(snapshot.members[slot]);
        if (!IsQueueEligible(user) ||
            user != snapshot.original_users[slot]) return false;
        users.push_back(user);
    }
    zombie_mode::ZombiePlayModePolicy play_policy;
    const uint32_t ready_time_ms = zombie_mode::GetReadyTimeMs();
    const uint32_t play_time_ms = zombie_mode::GetPlayTimeMs();
    if (!zombie_mode::GetPlayModePolicy(
            static_cast<uint8_t>(users.size()), play_policy) ||
        !ready_time_ms || !play_time_ms ||
        !ValidateZombiePlayPolicy(play_policy, users.size())) {
        return false;
    }
    PvP_Room* const room = G_CGameManager()->GetPvp();
    if (!room) return false;
    char empty[1] = {0};
    if (room->create_room(users[0], 1, empty, 0, false, empty, 0,
                          static_cast<PVP_BATTLE_MODE>(2)) > 0) {
        RollBackTargetRoom(room, snapshot);
        return false;
    }

    // Register the newly created room and acquire its raw-owner lease before
    // the first creator/seat presentation.  From this point every failure is
    // one FailBootstrapBuild transaction; a concurrent reset waits at the
    // PvP_Room::reset entry instead of reclaiming this room underneath us.
    bool attached = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found != g_targets.end() &&
            found->second.state == TARGET_AUTHORIZING &&
            !found->second.bootstrap_publish_in_progress &&
            !found->second.abort_pending &&
            !found->second.external_reset_pending) {
            found->second.room = room;
            found->second.room_index = room->get_index();
            found->second.authorized = false;
            found->second.bootstrap_publish_in_progress = true;
            g_target_rooms[room] = cohort_id;
            attached = true;
        }
    }
    if (!attached) {
        RollBackTargetRoom(room, snapshot);
        return false;
    }

    room->change_team(0, 1);
    SendCreatorRoomPackets(users[0], room);
    for (size_t slot = 1u; slot < users.size(); ++slot) {
        int seat = -1;
        if (room->join_room(users[slot], seat, false) > 0 ||
            seat != static_cast<int>(slot)) {
            return FailBootstrapBuild(cohort_id, room);
        }
        room->change_team(seat, slot < 3u ? 1 : 2);
        PacketGuard seat_packet;
        room->make_seat_info(reinterpret_cast<char*>(&seat_packet), seat);
        G_GameWorld()->send_all(seat_packet);
    }

    if (!channel_zombie_matching_producer::ActivateTargetRoom(room)) {
        (void)AbortBootstrapPublishLease(cohort_id, room);
        return false;
    }
    if (!BuildZombieMapOwners(cohort_id, room, users, play_policy,
                              ready_time_ms, play_time_ms, std::time(0))) {
        return false;
    }
    PacketGuard state_packet;
    bool publish_room_state = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() || !found->second.authorized ||
            found->second.room != room ||
            found->second.state < TARGET_CONNECTING) {
            return false;
        }
        // Materialize NOTI42 while the exact target entry still owns room.
        // A terminal path may detach/reset it immediately after this lock is
        // released, so only the independent PacketGuard may escape.
        found->second.room->make_state_info(
            reinterpret_cast<char*>(&state_packet));
        publish_room_state = state_packet.is_finallized();
    }
    if (!publish_room_state) return false;
    G_GameWorld()->send_all(state_packet);
    return true;
}

bool AuthorizeTargetRoom(PvP_Room* target_room) {
    if (!target_room) return false;
    MutexGuard guard(&g_mutex);
    std::map<PvP_Room*, uint32_t>::const_iterator room =
        g_target_rooms.find(target_room);
    if (room == g_target_rooms.end()) return false;
    std::map<uint32_t, TargetCohort>::iterator target =
        g_targets.find(room->second);
    if (target == g_targets.end() ||
        target->second.state != TARGET_AUTHORIZING ||
        target->second.authorized ||
        target_room->get_player_count() != target->second.cohort_size) {
        return false;
    }
    CEnvironment* const environment = G_CEnvironment();
    if (!environment ||
        environment->get_server_group() !=
            target->second.target_server_group ||
        environment->get_channel_no() != target->second.target_channel) {
        return false;
    }
    for (size_t slot = 0u; slot < target->second.cohort_size; ++slot) {
        CUser* const user = target_room->get_user_seat(
            static_cast<int>(slot));
        if (!user || !SameMember(MakeMember(user),
                                 target->second.members[slot])) return false;
    }
    target->second.authorized = true;
    return true;
}

bool FreezeSixPlayerLoadLocked(TargetCohort& target) {
    if (target.state != TARGET_LOADING || !target.load_epoch ||
        target.cohort_size !=
            zombie_matching_runtime_policy::kOfficialMatchingMembers ||
        target.expected_mask != static_cast<uint8_t>(0x3fu) ||
        target.loaded_mask != target.expected_mask ||
        !InstallActiveSlotMapping(
            target, target.loaded_mask,
            zombie_matching_runtime_policy::kOfficialMatchingMembers)) {
        return false;
    }
    target.degraded_from_six = false;
    target.deadline_at = 0;
    target.state = TARGET_STARTING;
    return true;
}

bool StartTargetCohort(uint32_t cohort_id) {
    TargetCohort snapshot;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::const_iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_STARTING ||
            !found->second.room) return false;
        snapshot = found->second;
    }

    if (snapshot.cohort_size != kRoomSeats || !snapshot.load_epoch ||
        snapshot.expected_mask != static_cast<uint8_t>(0x3fu) ||
        snapshot.loaded_mask != snapshot.active_mask ||
        CountGlobalSlots(snapshot.active_mask) != snapshot.active_count ||
        !ValidateZombiePlayPolicy(snapshot.play_policy,
                                  snapshot.active_count) ||
        snapshot.room->get_player_count() != snapshot.active_count) {
        return false;
    }

    uint8_t logical_slot = 0u;
    for (size_t slot = 0u; slot < kRoomSeats; ++slot) {
        const bool active =
            (snapshot.active_mask & GlobalSlotBit(slot)) != 0u;
        CUser* const user = ResolveMember(snapshot.members[slot]);
        CParty* const party = snapshot.map_parties[slot];
        if (!active) {
            if (snapshot.present[slot] || party ||
                snapshot.map_party_indices[slot] >= 0 ||
                snapshot.room->get_user_seat(static_cast<int>(slot))) {
                return false;
            }
            continue;
        }
        if (!snapshot.present[slot] || !snapshot.map_loaded[slot] ||
            !user || !party ||
            snapshot.global_to_active[slot] != logical_slot ||
            snapshot.active_to_global[logical_slot] != slot ||
            snapshot.map_party_indices[slot] < 0 ||
            party->GetPartyIndex() != snapshot.map_party_indices[slot] ||
            user->GetPartyIndex() != snapshot.map_party_indices[slot] ||
            party->get_state() != 2 || party->get_member_count() != 1 ||
            party->get_user(0) != user || user->GetParty() != party ||
            user->get_state() != 5 || user->GetPVPRoom() != snapshot.room ||
            user->GetPvpIndex() != snapshot.room_index ||
            snapshot.room->get_user_seat(static_cast<int>(slot)) != user ||
            party->battle_data().member_start_map_finish_loading(0) != 1u ||
            party->battle_data().member_finish_loading(0) != 0u ||
            party->battle_field().get_dungeon_index() !=
                static_cast<int>(snapshot.play_policy.dungeon_index)) {
            return false;
        }
        ++logical_slot;
    }
    if (logical_slot != snapshot.active_count) return false;

    if (!zombie_map_runtime::ArmRoom(
            snapshot.room,
            cohort_id,
            snapshot.load_epoch,
            snapshot.active_mask)) {
        return false;
    }

    // The owner-first CMD040 path deliberately withheld singleton NOTI30.
    // Only after the immutable global-seat mask is frozen do the active map
    // adapters complete their original loading transaction.  The pre-set
    // start-map flag makes this skip duplicate do_after/ACK work while each
    // active client receives exactly one NOTI30.
    for (size_t slot = 0u; slot < kRoomSeats; ++slot) {
        if ((snapshot.active_mask & GlobalSlotBit(slot)) == 0u) continue;
        CParty* const party = snapshot.map_parties[slot];
        CUser* const user = ResolveMember(snapshot.members[slot]);
        if (!party || !user) return false;
        (void)party->finish_loading(user);
        if (party->battle_data().member_finish_loading(0) != 1u)
            return false;
    }

    // BeginRoom has exactly one matching-owner caller.  It is intentionally
    // after every START_MAP and after the final five/six policy is known.
    if (!zombie_mode::BeginRoom(snapshot.room, snapshot.active_count) ||
        !BeginZombieSidecarBattle(snapshot.room, snapshot.play_policy,
                                  snapshot.active_mask)) {
        return false;
    }
    if (!zombie_map_runtime::ActivateRoom(snapshot.room, cohort_id) ||
        !ZombieItemBox::StartRoom(snapshot.room, cohort_id)) {
        return false;
    }

    const uint32_t now_ms = CurrentTickMilliseconds();
    const uint32_t ready_deadline = AddTickDelay(
        now_ms, snapshot.ready_duration_ms);
    const uint32_t play_deadline = AddTickDelay(
        ready_deadline, snapshot.play_duration_ms);

    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_STARTING ||
            found->second.room != snapshot.room ||
            found->second.load_epoch != snapshot.load_epoch ||
            found->second.active_mask != snapshot.active_mask) return false;
        found->second.state = TARGET_READY;
        found->second.ready_deadline_ms = ready_deadline;
        found->second.play_deadline_ms = play_deadline;
        found->second.deadline_at =
            std::time(0) +
            MillisecondsToCeilingSeconds(snapshot.ready_duration_ms) +
            MillisecondsToCeilingSeconds(snapshot.play_duration_ms) +
            kZombieSettlementGraceSeconds;
    }
    cMyTrace trace(
        "channel_zombie_matching_authority::StartTargetCohort",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][ROUND_ARMED] room=%d load_epoch=%u active=0x%02x members=%u ready_ms=%u",
          snapshot.room_index,
          static_cast<unsigned int>(snapshot.load_epoch),
          static_cast<unsigned int>(snapshot.active_mask),
          static_cast<unsigned int>(snapshot.active_count),
          static_cast<unsigned int>(snapshot.ready_duration_ms));
    return true;
}

bool SelectTargetCohortInitialRoles(uint32_t cohort_id) {
    TargetCohort snapshot;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::const_iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_SELECTING ||
            !found->second.room) return false;
        snapshot = found->second;
    }

    std::vector<uint32_t> zombie_seats;
    bool newly_selected = false;
    if (zombie_mode::SelectInitialZombies(
            snapshot.room, zombie_seats, newly_selected) != 0 ||
        zombie_seats.size() !=
            snapshot.play_policy.initial_zombie_count) {
        return false;
    }

    MutexGuard guard(&g_mutex);
    std::map<uint32_t, TargetCohort>::iterator found =
        g_targets.find(cohort_id);
    if (found == g_targets.end() ||
        found->second.state != TARGET_SELECTING ||
        found->second.room != snapshot.room ||
        found->second.load_epoch != snapshot.load_epoch) return false;
    found->second.state = TARGET_RUNNING;
    found->second.ready_deadline_ms = 0u;
    return true;
}

bool DegradeTargetCohortToFive(uint32_t cohort_id) {
    TargetCohort snapshot;
    size_t missing_slot = kRoomSeats;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::const_iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_DEGRADING ||
            !found->second.room || !found->second.load_epoch ||
            found->second.expected_mask != static_cast<uint8_t>(0x3fu) ||
            CountGlobalSlots(found->second.loaded_mask) !=
                zombie_matching_runtime_policy::kOfficialMinimumMembers) {
            return false;
        }
        snapshot = found->second;
        for (size_t slot = 0u; slot < snapshot.cohort_size; ++slot) {
            if ((snapshot.loaded_mask & GlobalSlotBit(slot)) == 0u) {
                if (missing_slot != kRoomSeats) return false;
                missing_slot = slot;
            }
        }
    }
    if (missing_slot >= snapshot.cohort_size) return false;

    zombie_mode::ZombiePlayModePolicy fallback_policy;
    if (!zombie_mode::GetPlayModePolicy(
            zombie_matching_runtime_policy::kOfficialMinimumMembers,
            fallback_policy) ||
        !ValidateZombiePlayPolicy(
            fallback_policy,
            zombie_matching_runtime_policy::kOfficialMinimumMembers)) {
        return false;
    }

    CUser* const missing_user =
        ResolveOriginalTargetMember(snapshot, missing_slot);
    CParty* const missing_party = snapshot.map_parties[missing_slot];
    const int missing_party_index = snapshot.map_party_indices[missing_slot];
    if (!missing_user || !missing_party || missing_party_index < 0 ||
        snapshot.members[missing_slot].global_slot != missing_slot ||
        snapshot.room->get_user_seat(static_cast<int>(missing_slot)) !=
            missing_user ||
        missing_user->GetPVPRoom() != snapshot.room ||
        missing_user->GetPvpIndex() != snapshot.room_index ||
        missing_user->GetParty() != missing_party ||
        missing_user->GetPartyIndex() != missing_party_index ||
        missing_party->GetPartyIndex() != missing_party_index ||
        missing_party->get_member_count() != 1 ||
        missing_party->get_user(0) != missing_user ||
        missing_party->battle_data().member_start_map_finish_loading(0) !=
            0u ||
        missing_party->battle_data().member_finish_loading(0) != 0u ||
        snapshot.room->get_player_count() !=
            zombie_matching_runtime_policy::kOfficialMatchingMembers) {
        return false;
    }

    // Preserve the immutable global-seat hole before the missing adapter is
    // returned to its pool.  A later failure aborts the whole room through
    // the two-phase runtime teardown; the remaining five are never compacted.
    if (!zombie_map_runtime::DropSeat(
            snapshot.room,
            cohort_id,
            static_cast<uint8_t>(missing_slot))) {
        return false;
    }

    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        std::map<CParty*, MapPartyOwnerRef>::iterator owner =
            g_target_map_parties.find(missing_party);
        if (found == g_targets.end() ||
            found->second.state != TARGET_DEGRADING ||
            found->second.load_epoch != snapshot.load_epoch ||
            found->second.loaded_mask != snapshot.loaded_mask ||
            found->second.map_parties[missing_slot] != missing_party ||
            owner == g_target_map_parties.end() ||
            owner->second.cohort_id != cohort_id ||
            owner->second.slot != missing_slot ||
            owner->second.party_index != missing_party_index ||
            owner->second.load_epoch != snapshot.load_epoch ||
            owner->second.detaching) {
            return false;
        }
        // Keep the pointer/index edge intact until ReturnToVillage enters the
        // lifecycle hook; the detaching bit makes that one release legal
        // while late CMD040 packets remain consumed.
        owner->second.detaching = true;
    }

    bool manager_changed = false;
    if (!snapshot.room->detach_zombie_loading_seat(
            missing_user, static_cast<int>(missing_slot),
            manager_changed)) {
        MutexGuard guard(&g_mutex);
        std::map<CParty*, MapPartyOwnerRef>::iterator owner =
            g_target_map_parties.find(missing_party);
        if (owner != g_target_map_parties.end() &&
            owner->second.cohort_id == cohort_id &&
            owner->second.load_epoch == snapshot.load_epoch) {
            owner->second.detaching = false;
        }
        return false;
    }

    PacketGuard seat_packet;
    snapshot.room->make_seat_info(
        reinterpret_cast<char*>(&seat_packet),
        static_cast<int>(missing_slot));
    G_GameWorld()->send_all(seat_packet);
    if (manager_changed) {
        PacketGuard state_packet;
        snapshot.room->make_state_info(
            reinterpret_cast<char*>(&state_packet));
        G_GameWorld()->send_all(state_packet);
    }

    missing_party->ReturnToVillage();
    const bool released_cleanly =
        missing_user->CheckInParty() == 0 &&
        !missing_user->GetPVPRoom() &&
        !snapshot.room->get_user_seat(static_cast<int>(missing_slot)) &&
        snapshot.room->get_player_count() ==
            zombie_matching_runtime_policy::kOfficialMinimumMembers;

    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_DEGRADING ||
            found->second.room != snapshot.room ||
            found->second.load_epoch != snapshot.load_epoch ||
            found->second.loaded_mask != snapshot.loaded_mask ||
            found->second.map_parties[missing_slot] != missing_party ||
            g_target_map_parties.find(missing_party) !=
                g_target_map_parties.end()) {
            return false;
        }
        found->second.present[missing_slot] = false;
        found->second.original_users[missing_slot] = 0;
        found->second.map_parties[missing_slot] = 0;
        found->second.map_party_indices[missing_slot] = -1;
        found->second.map_loaded[missing_slot] = false;
        if (!released_cleanly) return false;
        found->second.play_policy = fallback_policy;
        found->second.degraded_from_six = true;
        if (!InstallActiveSlotMapping(
                found->second, found->second.loaded_mask,
                zombie_matching_runtime_policy::kOfficialMinimumMembers)) {
            return false;
        }
        found->second.deadline_at = 0;
        found->second.state = TARGET_STARTING;
    }

    // The dropped sixth client has already consumed NOTI458.  It is no longer
    // present in the live five-player target, so complete its official
    // give-up UI transition exactly once before the remaining cohort starts.
    ResetDroppedClientSceneAfterFallback(snapshot, missing_slot);

    cMyTrace trace(
        "channel_zombie_matching_authority::DegradeTargetCohortToFive",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][FALLBACK5] room=%d load_epoch=%u missing_slot=%u active=0x%02x",
          snapshot.room_index,
          static_cast<unsigned int>(snapshot.load_epoch),
          static_cast<unsigned int>(missing_slot),
          static_cast<unsigned int>(snapshot.loaded_mask));
    return StartTargetCohort(cohort_id);
}

bool FinishTargetCohortByTimeout(uint32_t cohort_id, std::time_t now) {
    TargetCohort snapshot;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::const_iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_FINISHING ||
            !found->second.room) return false;
        snapshot = found->second;
    }
    if (!zombie_mode::IsFinishedRoom(snapshot.room) &&
        !zombie_mode::FinishByTimeout(snapshot.room)) return false;
    snapshot.room->pvp_request_pvp_rank();

    MutexGuard guard(&g_mutex);
    std::map<uint32_t, TargetCohort>::iterator found =
        g_targets.find(cohort_id);
    if (found == g_targets.end() ||
        found->second.state != TARGET_FINISHING ||
        found->second.room != snapshot.room) return false;
    found->second.state = TARGET_SETTLING;
    found->second.play_deadline_ms = 0u;
    found->second.deadline_at =
        now + kZombieSettlementGraceSeconds;
    return true;
}

bool ValidRouteResponse(const SourceCohort& source,
                        const Message& response) {
    return response.result == matching_channel_handoff_db::RESULT_SUCCESS &&
           response.operation ==
               matching_channel_handoff_db::OPERATION_ROUTE_COHORT &&
           response.handoff_id == 0u && response.field4 == 0u &&
           response.account_id == source.leader.account_id &&
           response.charac_no == source.leader.charac_no &&
           response.session_token == source.session_token &&
           response.field1 != 0u && response.cohort_slot == 0u &&
           response.cohort_size == source.members.size() &&
           response.cohort_flags == 0u &&
           response.matching_bound == kBound &&
           response.matching_mode == kMode &&
           response.source_server_group == source.source_server_group &&
           response.source_channel == source.source_channel &&
           response.target_server_group == source.leader.server_group &&
           IsChannel(response.target_channel) &&
           response.field0 ==
               static_cast<uint8_t>(response.target_server_group) &&
           response.endpoint_key != 0u &&
           response.endpoint_key != 0xffffffffu &&
           response.endpoint_port_like > 0u &&
           response.endpoint_port_like <= 65535u;
}

bool HandleRouteResponse(CUser* user, const Message& response) {
    uint32_t token = response.session_token;
    PvP_Room* room = 0;
    bool valid = false;
    bool retryable = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, SourceCohort>::iterator found =
            g_sources.find(token);
        if (found == g_sources.end() ||
            found->second.state != SOURCE_ROUTE_PENDING ||
            found->second.pending_request_token != response.request_token ||
            !SameMember(MakeMember(user), found->second.leader)) {
            return false;
        }
        SourceCohort& source = found->second;
        source.pending_request_token = 0u;
        valid = ValidRouteResponse(source, response);
        retryable = response.field1 == 0u &&
            (response.result == matching_channel_handoff_db::RESULT_BUSY ||
             response.result ==
                 matching_channel_handoff_db::RESULT_DATABASE_FAILURE);
        if (valid) {
            source.cohort_id = response.field1;
            source.route.target_server_group = response.target_server_group;
            source.route.target_channel = response.target_channel;
            source.route.field0 = response.field0;
            source.route.cohort_id = response.field1;
            source.route.endpoint_key = response.endpoint_key;
            source.route.endpoint_port = static_cast<uint16_t>(
                response.endpoint_port_like);
            source.route_cached_at = std::time(0);
            source.state = SOURCE_ROUTE_READY;
            source.deadline_at =
                source.route_cached_at + kRouteCacheLifetimeSeconds;
            g_source_cohorts[source.cohort_id] = token;
            room = source.room;
        } else if (response.result ==
                       matching_channel_handoff_db::RESULT_SUCCESS &&
                   response.field1 != 0u) {
            // Correlation and durable id are sufficient to issue a source
            // authority abort even when the returned route itself is
            // malformed.  No endpoint from this branch is published.
            source.cohort_id = response.field1;
            g_source_cohorts[source.cohort_id] = token;
        }
    }
    if (!valid) {
        if (retryable) {
            RequeueSourceToken(token, std::time(0));
        } else {
            AbortSourceToken(token);
        }
        return true;
    }
    const bool published =
        channel_zombie_matching_producer::PublishMatchedSourceRoom(room);
    if (!published) {
        AbortSourceToken(token);
        return true;
    }
    return true;
}

bool ValidActivateResponse(const TargetCohort& target,
                           const Message& response) {
    return response.result == matching_channel_handoff_db::RESULT_SUCCESS &&
           response.operation ==
               matching_channel_handoff_db::OPERATION_ACTIVATE_COHORT &&
           response.account_id == target.members[0].account_id &&
           response.charac_no == target.members[0].charac_no &&
           response.session_token == target.session_token &&
           response.field1 == target.cohort_id &&
           response.handoff_id == target.handoffs[0].handoff_id &&
           response.field4 == response.handoff_id &&
           response.state == matching_channel_handoff_db::STATE_CLAIMED &&
           response.source_cleaned &&
           response.cohort_size == target.cohort_size &&
           response.cohort_slot == 0u &&
           (response.cohort_flags &
                matching_channel_handoff_db::COHORT_FLAG_ALL) ==
               matching_channel_handoff_db::COHORT_FLAG_ALL &&
           response.source_server_group == target.source_server_group &&
           response.source_channel == target.source_channel &&
           response.target_server_group == target.target_server_group &&
           response.target_channel == target.target_channel &&
           response.field0 == target.field0 &&
           response.endpoint_key == target.endpoint_key &&
           response.endpoint_port_like == target.endpoint_port_like &&
           response.matching_bound == kBound &&
           response.matching_mode == kMode;
}

bool HandleActivateResponse(CUser* user, const Message& response) {
    const uint32_t cohort_id = response.field1;
    bool valid = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_ACTIVATE_PENDING ||
            found->second.pending_request_token != response.request_token ||
            found->second.session_token != response.session_token ||
            !found->second.present[0] ||
            !SameMember(MakeMember(user), found->second.members[0])) {
            return false;
        }
        found->second.pending_request_token = 0u;
        valid = ValidActivateResponse(found->second, response);
        if (valid) {
            found->second.state = TARGET_AUTHORIZING;
            found->second.deadline_at =
                std::time(0) + kTargetAuthorizeDeadlineSeconds;
        }
    }
    if (!valid || !BuildTargetRoom(cohort_id))
        AbortTargetCohort(cohort_id);
    return true;
}

void QueueComplete(const TargetCohort& target) {
    if (target.local_same_channel || !target.cohort_id ||
        !target.cohort_size || !target.present[0]) return;
    Message request;
    FillControlBase(request, target.members[0], target.session_token,
                    target.next_request_token,
                    matching_channel_handoff_db::OPERATION_COMPLETE_COHORT,
                    target.cohort_id, target.cohort_size);
    request.source_server_group = target.source_server_group;
    request.source_channel = target.source_channel;
    request.target_server_group = target.target_server_group;
    request.target_channel = target.target_channel;
    request.field0 = target.field0;
    request.endpoint_key = target.endpoint_key;
    request.endpoint_port_like = target.endpoint_port_like;
    request.handoff_id = target.handoffs[0].handoff_id;
    request.field4 = target.handoffs[0].handoff_id;
    request.state = matching_channel_handoff_db::STATE_CLAIMED;
    request.source_cleaned = 1u;
    (void)QueueControl(target.members[0], request);
}

void CompleteTargetCohort(uint32_t cohort_id) {
    TargetCohort snapshot;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::const_iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_SETTLING ||
            !found->second.room) return;
        snapshot = found->second;
    }
    // Result owner calls acquire its own mutex and may queue DB work.  Never
    // invoke them while the matching authority mutex is held.
    if (!zombie_mode::IsFinishedRoom(snapshot.room) ||
        !zombie_mode::IsResultDeliveryComplete(snapshot.room)) {
        return;
    }

    TargetCohort target;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, TargetCohort>::const_iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() ||
            found->second.state != TARGET_SETTLING ||
            found->second.room != snapshot.room ||
            !TakeTargetLocked(cohort_id, target)) return;
    }
    QueueComplete(target);
    RollBackTargetRoom(target.room, target);
}

}  // namespace

bool Initialize() {
    {
        MutexGuard guard(&g_mutex);
        if (g_started) return
            channel_zombie_matching_producer::CanRegisterPackets();
        if (g_next_authority_token == 0u)
            g_next_authority_token = MakeProcessAuthoritySeed();
        if (g_next_ready_key == 0u) {
            g_next_ready_key = g_next_authority_token ^ 0x72ead001u;
            if (g_next_ready_key == 0u) g_next_ready_key = 1u;
        }
        if (g_next_lobby_token == 0u) {
            g_next_lobby_token = g_next_authority_token ^ 0x10bb1001u;
            if (g_next_lobby_token == 0u) g_next_lobby_token = 1u;
        }
        if (g_next_load_epoch == 0u) {
            g_next_load_epoch = g_next_authority_token ^ 0x10ad0001u;
            if (g_next_load_epoch == 0u) g_next_load_epoch = 1u;
        }
    }
    if (!channel_zombie_matching_producer::Initialize()) return false;

    AuthorityV2 authority;
    std::memset(&authority, 0, sizeof(authority));
    authority.abi_version =
        channel_zombie_matching_producer::kAuthorityAbiVersion;
    authority.struct_size = sizeof(authority);
    authority.evidence_mask =
        channel_zombie_matching_producer::EVIDENCE_EXACT_PAIR |
        channel_zombie_matching_producer::EVIDENCE_MATCHINGMODE_PVF |
        channel_zombie_matching_producer::EVIDENCE_ZOMBIEMODE_PVF |
        channel_zombie_matching_producer::EVIDENCE_EIGHT_SEAT_ROOM |
        channel_zombie_matching_producer::EVIDENCE_SOURCE_CONTAINER |
        channel_zombie_matching_producer::EVIDENCE_PROJECT_ROUTE_SELECTOR |
        channel_zombie_matching_producer::EVIDENCE_FIELD0_TARGET_GROUP |
        channel_zombie_matching_producer::EVIDENCE_PROJECT_COHORT_ID |
        channel_zombie_matching_producer::EVIDENCE_ENDPOINT_GAME_CHANNEL |
        channel_zombie_matching_producer::EVIDENCE_FIELD4_CLAIM_KEY |
        channel_zombie_matching_producer::
            EVIDENCE_PROJECT_TARGET_COHORT_REBUILD;
    authority.enqueue = &Enqueue;
    authority.commit_enqueue = &CommitEnqueue;
    authority.abort_enqueue = &AbortEnqueue;
    authority.cancel = &Cancel;
    authority.select_target_route = &SelectTargetRoute;
    authority.abort_source_cohort = &AbortSourceRoom;
    authority.accept_claim = &AcceptClaim;
    authority.authorize_target_room = &AuthorizeTargetRoom;
    if (!channel_zombie_matching_producer::InstallAuthority(authority))
        return false;
    zombie_map_runtime::InstallAbortRoomCallback(
        &AbortZombieMapRuntimeRoom);
    {
        MutexGuard guard(&g_mutex);
        g_started = true;
    }
    return channel_zombie_matching_producer::CanRegisterPackets();
}

void Shutdown() {
    zombie_map_runtime::InstallAbortRoomCallback(0);
    channel_zombie_matching_producer::RemoveAuthority();
    std::vector<SourceCohort> sources;
    std::vector<TargetCohort> targets;
    std::vector<uint32_t> target_ids;
    LobbyUiUpdates ui_updates;
    {
        MutexGuard guard(&g_mutex);
        g_started = false;
        for (std::deque<QueueGroup>::const_iterator group = g_queue.begin();
             group != g_queue.end(); ++group) {
            QueueLobbyCancelForGroupLocked(*group, ui_updates);
        }
        g_queue.clear();
        for (std::map<uint32_t, SourceCohort>::const_iterator it =
                 g_sources.begin(); it != g_sources.end(); ++it)
            sources.push_back(it->second);
        for (std::map<uint32_t, TargetCohort>::const_iterator it =
                 g_targets.begin(); it != g_targets.end(); ++it)
            target_ids.push_back(it->first);
        for (std::vector<uint32_t>::const_iterator it = target_ids.begin();
             it != target_ids.end(); ++it) {
            TargetCohort target;
            // A bootstrap publisher keeps its registry and raw owners until
            // ReleaseBootstrapPublishLease observes abort_pending.  Every
            // other target is detached now and cleaned below, outside g_mutex.
            if (TakeTargetLocked(*it, target)) targets.push_back(target);
        }
        g_sources.clear();
        g_source_rooms.clear();
        g_source_cohorts.clear();
        g_pending_enqueue_commits.clear();
        g_route_retry_at = 0;
    }
    DispatchLobbyUiUpdates(ui_updates);
    for (std::vector<SourceCohort>::const_iterator it = sources.begin();
         it != sources.end(); ++it) {
        QueueAbortControl(*it);
        for (std::vector<MemberRef>::const_iterator member =
                 it->members.begin(); member != it->members.end(); ++member) {
            CUser* const user = ResolveMember(*member);
            if (user) matching_channel_handoff::ResetSession(user);
        }
        matching_channel_handoff::UnbindMatchingRoom(it->room);
        RollBackRoom(it->room, it->members);
    }
    for (std::vector<TargetCohort>::const_iterator it = targets.begin();
         it != targets.end(); ++it) {
        QueueTargetAbortControl(*it);
        RollBackTargetRoom(it->room, *it);
    }
}

bool ConsumeMapOwnerFinishLoading(CParty* party,
                                  CUser* user,
                                  int32_t inventory_checksum,
                                  int32_t skill_checksum) {
    if (!party || !user) return false;

    uint32_t completed_map_epoch = 0u;
    const zombie_map_runtime::HookResult map_finish =
        zombie_map_runtime::ConsumeSeatFinishLoading(
            party,
            user,
            inventory_checksum,
            skill_checksum,
            &completed_map_epoch);
    if (map_finish != zombie_map_runtime::kHookNotManaged) {
        cMyTrace trace(
            "channel_zombie_matching_authority::ConsumeMapOwnerFinishLoading",
            __LINE__, 5);
        trace("[ZOMBIE_MATCH][MAP_FINISH_ROUTE] map_epoch=%u result=%d",
              static_cast<unsigned int>(completed_map_epoch),
              static_cast<int>(map_finish));
        return true;
    }

    MapPartyOwnerRef owner;
    uint32_t cohort_id = 0u;
    int room_index = -1;
    bool accept_ingress = false;
    bool duplicate = false;
    bool abort_invalid = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<CParty*, MapPartyOwnerRef>::const_iterator mapped =
            g_target_map_parties.find(party);
        if (mapped == g_target_map_parties.end()) return false;
        owner = mapped->second;
        cohort_id = owner.cohort_id;
        std::map<uint32_t, TargetCohort>::iterator target =
            g_targets.find(cohort_id);
        if (target == g_targets.end()) {
            abort_invalid = true;
        } else {
            room_index = target->second.room_index;
            const uint8_t bit = GlobalSlotBit(owner.slot);
            const bool timeout_detaching_owner = owner.detaching &&
                target->second.state == TARGET_DEGRADING &&
                owner.slot < target->second.cohort_size &&
                owner.load_epoch != 0u &&
                owner.load_epoch == target->second.load_epoch &&
                target->second.map_parties[owner.slot] == party &&
                target->second.map_party_indices[owner.slot] ==
                    owner.party_index;
            const bool exact_owner = owner.slot < target->second.cohort_size &&
                owner.slot < kRoomSeats && bit != 0u &&
                owner.load_epoch != 0u &&
                owner.load_epoch == target->second.load_epoch &&
                target->second.present[owner.slot] &&
                target->second.members[owner.slot].global_slot == owner.slot &&
                SameMember(target->second.members[owner.slot], owner.member) &&
                target->second.map_parties[owner.slot] == party &&
                target->second.map_party_indices[owner.slot] ==
                    owner.party_index &&
                target->second.room &&
                target->second.room->get_user_seat(
                    static_cast<int>(owner.slot)) == user &&
                ResolveMember(owner.member) == user &&
                party->GetPartyIndex() == owner.party_index &&
                party->get_member_count() == 1 &&
                party->get_user(0) == user && user->GetParty() == party &&
                user->GetPartyIndex() == owner.party_index &&
                user->GetPVPRoom() == target->second.room &&
                user->GetPvpIndex() == target->second.room_index;
            if (timeout_detaching_owner) {
                // The room edge may already be severed while the missing
                // adapter is returning to village.  It remains recognized by
                // its immutable pointer/index/epoch tuple and is consumed.
                duplicate = true;
            } else if (!exact_owner) {
                abort_invalid = true;
            } else if (target->second.state == TARGET_DEGRADING) {
                // The timeout decision already froze the five-seat mask.
                duplicate = true;
            } else if ((target->second.loaded_mask & bit) != 0u) {
                duplicate = true;
            } else if ((target->second.state == TARGET_CONNECTING ||
                        target->second.state == TARGET_LOADING) &&
                       (target->second.expected_mask & bit) != 0u) {
                accept_ingress = true;
            } else {
                // A mapped owner is still consumed after the load phase; an
                // out-of-order/replayed CMD040 cannot reach ordinary CParty.
                duplicate = true;
            }
        }
    }

    cMyTrace ingress_trace(
        "channel_zombie_matching_authority::ConsumeMapOwnerFinishLoading",
        __LINE__, 5);
    ingress_trace("[ZOMBIE_MATCH][LOAD_INGRESS] room=%d load_epoch=%u slot=%u accepted=%d duplicate=%d",
                  room_index,
                  static_cast<unsigned int>(owner.load_epoch),
                  static_cast<unsigned int>(owner.slot),
                  accept_ingress ? 1 : 0,
                  duplicate ? 1 : 0);

    if (abort_invalid) {
        AbortTargetCohort(cohort_id);
        return true;
    }
    if (!accept_ingress) return true;

    if (user->getCurCharacInvenCheckSum() != inventory_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x25b),
            1u, 0u, 0u);
    }
    if (user->getCurCharacSkillCheckSum() != skill_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x25c),
            1u, 0u, 0u);
    }

    if (party->battle_data().member_finish_loading(0) != 0u) {
        AbortTargetCohort(cohort_id);
        return true;
    }
    if (party->battle_data().member_start_map_finish_loading(0) != 1u) {
        if (!party->OnStartMapFinishLoading(user)) {
            AbortTargetCohort(cohort_id);
            return true;
        }
        party->battle_data().member_start_map_finish_loading(0) = 1u;
    }

    bool start_now = false;
    bool start_decision = false;
    uint8_t loaded_mask = 0u;
    uint8_t expected_mask = 0u;
    {
        MutexGuard guard(&g_mutex);
        std::map<CParty*, MapPartyOwnerRef>::const_iterator mapped =
            g_target_map_parties.find(party);
        std::map<uint32_t, TargetCohort>::iterator target =
            g_targets.find(cohort_id);
        if (mapped == g_target_map_parties.end() ||
            target == g_targets.end()) {
            return true;
        }
        const uint8_t bit = GlobalSlotBit(owner.slot);
        if (mapped->second.cohort_id != cohort_id ||
            mapped->second.slot != owner.slot ||
            mapped->second.party_index != owner.party_index ||
            mapped->second.load_epoch != owner.load_epoch ||
            mapped->second.detaching ||
            (target->second.state != TARGET_CONNECTING &&
             target->second.state != TARGET_LOADING) ||
            target->second.load_epoch != owner.load_epoch ||
            target->second.map_parties[owner.slot] != party ||
            target->second.map_party_indices[owner.slot] !=
                owner.party_index ||
            (target->second.expected_mask & bit) == 0u) {
            // A concurrent timeout decision wins; the packet is consumed but
            // cannot widen its already-frozen active mask.
            return true;
        }
        target->second.loaded_mask = static_cast<uint8_t>(
            target->second.loaded_mask | bit);
        target->second.map_loaded[owner.slot] = true;
        loaded_mask = target->second.loaded_mask;
        expected_mask = target->second.expected_mask;
        if (target->second.state == TARGET_LOADING &&
            !target->second.bootstrap_publish_in_progress &&
            loaded_mask == expected_mask) {
            start_decision = true;
            start_now = FreezeSixPlayerLoadLocked(target->second);
            if (!start_now) {
                target->second.deadline_at = 0;
            }
        }
    }

    cMyTrace mask_trace(
        "channel_zombie_matching_authority::ConsumeMapOwnerFinishLoading",
        __LINE__, 5);
    mask_trace("[ZOMBIE_MATCH][LOAD_MASK] room=%d load_epoch=%u slot=%u loaded=0x%02x expected=0x%02x count=%u",
               room_index,
               static_cast<unsigned int>(owner.load_epoch),
               static_cast<unsigned int>(owner.slot),
               static_cast<unsigned int>(loaded_mask),
               static_cast<unsigned int>(expected_mask),
               static_cast<unsigned int>(CountGlobalSlots(loaded_mask)));

    if (start_decision) {
        if (!start_now || !StartTargetCohort(cohort_id))
            AbortTargetCohort(cohort_id);
    }
    return true;
}

bool ConsumeMapOwnerLoadTimeout(CParty* party) {
    if (!party) return false;
    bool consume = false;
    bool tick_loading_authority = false;
    bool abort_connecting_authority = false;
    uint32_t cohort_id = 0u;
    int room_index = -1;
    uint32_t load_epoch = 0u;
    size_t slot = 0u;
    {
        MutexGuard guard(&g_mutex);
        std::map<CParty*, MapPartyOwnerRef>::const_iterator mapped =
            g_target_map_parties.find(party);
        if (mapped == g_target_map_parties.end()) return false;
        std::map<uint32_t, TargetCohort>::const_iterator target =
            g_targets.find(mapped->second.cohort_id);
        if (target == g_targets.end() ||
            mapped->second.slot >= target->second.cohort_size ||
            target->second.map_parties[mapped->second.slot] != party ||
            target->second.map_party_indices[mapped->second.slot] !=
                mapped->second.party_index) {
            return false;
        }
        const bool connecting_epoch =
            target->second.state == TARGET_CONNECTING &&
            mapped->second.load_epoch != 0u &&
            mapped->second.load_epoch == target->second.load_epoch;
        const bool loading_epoch =
            target->second.state >= TARGET_LOADING &&
            mapped->second.load_epoch != 0u &&
            mapped->second.load_epoch == target->second.load_epoch;
        if (!connecting_epoch && !loading_epoch) return false;
        consume = true;
        cohort_id = mapped->second.cohort_id;
        abort_connecting_authority =
            target->second.state == TARGET_CONNECTING;
        tick_loading_authority =
            target->second.state == TARGET_LOADING;
        room_index = target->second.room_index;
        load_epoch = target->second.load_epoch;
        slot = mapped->second.slot;
    }
    if (!consume) return false;
    cMyTrace trace(
        "channel_zombie_matching_authority::ConsumeMapOwnerLoadTimeout",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][LOAD_OWNER_TIMER] room=%d load_epoch=%u slot=%u authority_owned=1 phase=%s action=%s",
          room_index,
          static_cast<unsigned int>(load_epoch),
          static_cast<unsigned int>(slot),
          abort_connecting_authority ? "connecting" : "loading",
          abort_connecting_authority ? "abort" :
              (tick_loading_authority ? "tick" : "consume"));
    // START_MAP is visible before CMD295 by protocol contract.  Therefore a
    // hidden owner can report its native load timeout while the cohort is
    // still CONNECTING.  Waiting for the 180-second P2P deadline here leaves
    // every client trapped in the completed matching scene.  Fail the whole
    // six-seat transaction immediately; exact-five degradation remains
    // exclusive to TARGET_LOADING after NOTI279.
    if (abort_connecting_authority) {
        AbortTargetCohort(cohort_id);
    } else if (tick_loading_authority) {
        Tick();
    }
    return true;
}

bool ConsumeCompleteLoad(CUser* user, PvP_Room* room) {
    if (!user || !room) return false;
    uint32_t cohort_id = 0u;
    const int room_index = room->get_index();
    size_t reporter_slot = kRoomSeats;
    uint8_t ready_mask = 0u;
    uint8_t expected_mask = 0u;
    bool managed = false;
    bool accepted = false;
    bool duplicate = false;
    bool invalid = false;
    std::vector<CUser*> users;
    {
        MutexGuard guard(&g_mutex);
        std::map<PvP_Room*, uint32_t>::const_iterator mapped =
            g_target_rooms.find(room);
        if (mapped == g_target_rooms.end()) return false;
        cohort_id = mapped->second;
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() || found->second.room != room) {
            return false;
        }
        managed = true;
        TargetCohort& target = found->second;
        expected_mask = target.complete_load_expected_mask;
        for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
            if (ResolveOriginalTargetMember(target, slot) == user &&
                room->get_user_seat(static_cast<int>(slot)) == user) {
                reporter_slot = slot;
                break;
            }
        }
        const uint8_t official_mask = static_cast<uint8_t>(
            (1u << zombie_matching_runtime_policy::
                       kOfficialMatchingMembers) - 1u);
        if (reporter_slot >= target.cohort_size ||
            target.cohort_size !=
                zombie_matching_runtime_policy::kOfficialMatchingMembers ||
            !target.authorized || !target.matching_scene_committed ||
            target.abort_pending || target.external_reset_pending ||
            expected_mask != official_mask) {
            invalid = true;
        } else if (target.state != TARGET_CONNECTING) {
            // Managed late/replayed CMD295 reports are consumed without
            // re-broadcasting NOTI278 or entering an unrelated IMatch.
            duplicate = true;
        } else {
            const uint8_t reporter_bit = GlobalSlotBit(reporter_slot);
            duplicate =
                (target.complete_load_ready_mask & reporter_bit) != 0u;
            if (!duplicate) {
                for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
                    CUser* const recipient =
                        ResolveOriginalTargetMember(target, slot);
                    if (!target.present[slot] ||
                        target.members[slot].global_slot != slot ||
                        !recipient ||
                        room->get_user_seat(static_cast<int>(slot)) !=
                            recipient ||
                        recipient->GetPVPRoom() != room ||
                        recipient->GetPvpIndex() != target.room_index) {
                        invalid = true;
                        break;
                    }
                    users.push_back(recipient);
                }
                if (!invalid) {
                    target.complete_load_ready_mask = static_cast<uint8_t>(
                        target.complete_load_ready_mask | reporter_bit);
                    accepted = true;
                }
            }
            ready_mask = target.complete_load_ready_mask;
        }
        if (!ready_mask) ready_mask = target.complete_load_ready_mask;
    }

    cMyTrace trace(
        "channel_zombie_matching_authority::ConsumeCompleteLoad",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][COMPLETE_LOAD_INGRESS] room=%d cohort=%u slot=%u accepted=%d duplicate=%d invalid=%d ready=0x%02x expected=0x%02x",
          room_index,
          static_cast<unsigned int>(cohort_id),
          static_cast<unsigned int>(reporter_slot),
          accepted ? 1 : 0,
          duplicate ? 1 : 0,
          invalid ? 1 : 0,
          static_cast<unsigned int>(ready_mask),
          static_cast<unsigned int>(expected_mask));

    if (!managed) return false;
    if (invalid) {
        AbortTargetCohort(cohort_id);
        return true;
    }
    if (!accepted) return true;

    const bool sent = SendZombieCompleteLoad(users, user);
    trace("[ZOMBIE_MATCH][COMPLETE_LOAD_NOTIFY] room=%d cohort=%u slot=%u ready=0x%02x expected=0x%02x noti278=%d",
          room_index,
          static_cast<unsigned int>(cohort_id),
          static_cast<unsigned int>(reporter_slot),
          static_cast<unsigned int>(ready_mask),
          static_cast<unsigned int>(expected_mask),
          sent ? 1 : 0);
    if (!sent) AbortTargetCohort(cohort_id);
    return true;
}

bool ConsumeConnectP2P(CUser* user,
                       PvP_Room* room,
                       const char results[8],
                       int count) {
    if (!user || !room || !results) return false;
    uint32_t cohort_id = 0u;
    int room_index = room->get_index();
    size_t reporter_slot = kRoomSeats;
    uint8_t ready_mask = 0u;
    uint8_t expected_mask = 0u;
    uint8_t complete_load_ready_mask = 0u;
    uint8_t complete_load_expected_mask = 0u;
    uint8_t failure_mask = 0u;
    bool managed = false;
    bool accepted = false;
    bool duplicate = false;
    bool invalid = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<PvP_Room*, uint32_t>::const_iterator mapped =
            g_target_rooms.find(room);
        if (mapped == g_target_rooms.end()) return false;
        cohort_id = mapped->second;
        std::map<uint32_t, TargetCohort>::iterator found =
            g_targets.find(cohort_id);
        if (found == g_targets.end() || found->second.room != room) {
            return false;
        }
        managed = true;
        TargetCohort& target = found->second;
        expected_mask = target.p2p_expected_mask;
        complete_load_ready_mask = target.complete_load_ready_mask;
        complete_load_expected_mask = target.complete_load_expected_mask;
        for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
            if (ResolveOriginalTargetMember(target, slot) == user &&
                room->get_user_seat(static_cast<int>(slot)) == user) {
                reporter_slot = slot;
                break;
            }
        }
        if (reporter_slot >= target.cohort_size ||
            target.cohort_size !=
                zombie_matching_runtime_policy::kOfficialMatchingMembers ||
            expected_mask != static_cast<uint8_t>(0x3fu)) {
            invalid = true;
        } else if (target.state != TARGET_CONNECTING) {
            // The managed room consumes late duplicates after NOTI279 and all
            // out-of-order reports before/after the exact P2P phase.
            duplicate = true;
        } else if (complete_load_expected_mask !=
                       static_cast<uint8_t>(0x3fu) ||
                   complete_load_ready_mask !=
                       complete_load_expected_mask) {
            // Official Fair/Online clients enter CMD296 only after all
            // participant NOTI278 frames.  A premature report is a broken
            // cohort transaction and cannot advance the P2P mask.
            invalid = true;
        } else {
            for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
                const uint8_t bit = GlobalSlotBit(slot);
                if (count <= static_cast<int>(slot) || results[slot] == 0) {
                    failure_mask = static_cast<uint8_t>(failure_mask | bit);
                }
            }
            if (failure_mask != 0u || count < target.cohort_size ||
                count > static_cast<int>(kRoomSeats)) {
                invalid = true;
            } else {
                const uint8_t reporter_bit = GlobalSlotBit(reporter_slot);
                duplicate =
                    (target.p2p_ready_mask & reporter_bit) != 0u;
                if (!duplicate) {
                    target.p2p_ready_mask = static_cast<uint8_t>(
                        target.p2p_ready_mask | reporter_bit);
                    accepted = true;
                }
                ready_mask = target.p2p_ready_mask;
            }
        }
        if (!ready_mask) ready_mask = target.p2p_ready_mask;
    }

    cMyTrace trace(
        "channel_zombie_matching_authority::ConsumeConnectP2P",
        __LINE__, 5);
    trace("[ZOMBIE_MATCH][P2P_INGRESS] room=%d cohort=%u slot=%u count=%d accepted=%d duplicate=%d invalid=%d failure=0x%02x complete=0x%02x/0x%02x ready=0x%02x expected=0x%02x",
          room_index,
          static_cast<unsigned int>(cohort_id),
          static_cast<unsigned int>(reporter_slot),
          count,
          accepted ? 1 : 0,
          duplicate ? 1 : 0,
          invalid ? 1 : 0,
          static_cast<unsigned int>(failure_mask),
          static_cast<unsigned int>(complete_load_ready_mask),
          static_cast<unsigned int>(complete_load_expected_mask),
          static_cast<unsigned int>(ready_mask),
          static_cast<unsigned int>(expected_mask));

    if (!managed) return false;
    if (invalid) {
        AbortTargetCohort(cohort_id);
        return true;
    }
    if (!accepted || ready_mask != expected_mask) return true;

    const P2PPublishStatus publish = TryPublishZombieP2PBarrier(
        cohort_id, room, std::time(0));
    if (publish == P2P_PUBLISH_INVALID) AbortTargetCohort(cohort_id);
    return true;
}

bool AbortMapOwnerBeforeLifecycle(CParty* party, CUser* actor) {
    if (!party) return false;
    MapPartyOwnerRef owner;
    bool valid_owner = false;
    bool target_present = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<CParty*, MapPartyOwnerRef>::iterator mapped =
            g_target_map_parties.find(party);
        if (mapped == g_target_map_parties.end()) return false;
        owner = mapped->second;
        std::map<uint32_t, TargetCohort>::iterator target =
            g_targets.find(owner.cohort_id);
        target_present = target != g_targets.end();
        CUser* const expected_user = ResolveMember(owner.member);
        if (owner.detaching && target_present &&
            target->second.state == TARGET_DEGRADING &&
            target->second.load_epoch == owner.load_epoch &&
            owner.slot < target->second.cohort_size &&
            target->second.map_parties[owner.slot] == party &&
            target->second.map_party_indices[owner.slot] ==
                owner.party_index && expected_user &&
            party->GetPartyIndex() == owner.party_index &&
            party->get_member_count() == 1 &&
            party->get_user(0) == expected_user &&
            expected_user->GetParty() == party &&
            expected_user->GetPartyIndex() == owner.party_index &&
            (!actor || actor == expected_user)) {
            // The exact timeout-missing adapter is the only owner lifecycle
            // allowed to detach without aborting the other five seats.
            g_target_map_parties.erase(mapped);
            return false;
        }
        valid_owner = target_present && owner.slot < kRoomSeats &&
            owner.slot < target->second.cohort_size &&
            target->second.map_parties[owner.slot] == party &&
            target->second.map_party_indices[owner.slot] ==
                owner.party_index &&
            party->GetPartyIndex() == owner.party_index && expected_user &&
            party->get_member_count() == 1 &&
            party->get_user(0) == expected_user &&
            expected_user->GetParty() == party &&
            expected_user->GetPartyIndex() == owner.party_index &&
            (!actor || actor == expected_user);
        if (!valid_owner) {
            // A pooled CParty pointer was recycled or its user edge already
            // drifted.  Never destroy that pointer as the old cohort owner.
            g_target_map_parties.erase(mapped);
            if (target_present && owner.slot < kRoomSeats &&
                target->second.map_parties[owner.slot] == party) {
                target->second.map_parties[owner.slot] = 0;
                target->second.map_party_indices[owner.slot] = -1;
                target->second.map_loaded[owner.slot] = false;
            }
        }
    }

    if (!valid_owner) {
        if (target_present) AbortTargetCohort(owner.cohort_id);
        CUser* const old_user = ResolveMember(owner.member);
        if (old_user && old_user->GetPartyIndex() == owner.party_index) {
            if (old_user->get_state() == 5 && G_GameWorld())
                G_GameWorld()->out_from_dungeon(old_user);
            old_user->SetPartyIndex(-1);
        }
        return false;
    }

    // AbortTargetCohort -> TakeTargetLocked removes the registry entry before
    // RollBackTargetRoom calls ReturnToVillage/destroy, so the nested hook is
    // intentionally a no-op and cannot recurse or double-return the pool.
    AbortTargetCohort(owner.cohort_id);
    return true;
}

bool OnDatabaseResponse(CUser* user, const Message& response) {
    if (!user) return false;
    switch (response.operation) {
    case matching_channel_handoff_db::OPERATION_ROUTE_COHORT:
        return HandleRouteResponse(user, response);
    case matching_channel_handoff_db::OPERATION_ACTIVATE_COHORT:
        return HandleActivateResponse(user, response);
    case matching_channel_handoff_db::OPERATION_ABORT_COHORT:
    case matching_channel_handoff_db::OPERATION_COMPLETE_COHORT:
        // These are terminal best-effort acknowledgements.  Runtime state is
        // removed before the request, so a stale/replayed response has no
        // authority to recreate a cohort.
        return response.result == matching_channel_handoff_db::RESULT_SUCCESS;
    default:
        return false;
    }
}

namespace {

enum TargetLoadStatus {
    TARGET_LOAD_INVALID = -1,
    TARGET_LOAD_PENDING = 0,
    TARGET_LOAD_COMPLETE = 1,
};

TargetLoadStatus RefreshTargetMapLoadingLocked(TargetCohort& target) {
    if (!target.room || !IsMatchingCohortSize(target.cohort_size) ||
        target.state != TARGET_LOADING || !target.load_epoch ||
        target.expected_mask != static_cast<uint8_t>(0x3fu) ||
        (target.loaded_mask &
         static_cast<uint8_t>(~target.expected_mask)) != 0u) {
        return TARGET_LOAD_INVALID;
    }
    for (size_t slot = 0u; slot < target.cohort_size; ++slot) {
        const uint8_t bit = GlobalSlotBit(slot);
        const bool ingress_loaded = (target.loaded_mask & bit) != 0u;
        CUser* const user = ResolveMember(target.members[slot]);
        CParty* const party = target.map_parties[slot];
        std::map<CParty*, MapPartyOwnerRef>::const_iterator owner =
            g_target_map_parties.find(party);
        if (!target.present[slot] || !user || !party ||
            target.members[slot].global_slot != slot ||
            target.map_party_indices[slot] < 0 ||
            owner == g_target_map_parties.end() ||
            owner->second.cohort_id != target.cohort_id ||
            owner->second.slot != slot ||
            owner->second.party_index != target.map_party_indices[slot] ||
            owner->second.load_epoch != target.load_epoch ||
            owner->second.detaching ||
            party->GetPartyIndex() != target.map_party_indices[slot] ||
            user->GetPartyIndex() != target.map_party_indices[slot] ||
            party->get_state() != 2 || party->get_member_count() != 1 ||
            party->get_user(0) != user || user->GetParty() != party ||
            user->get_state() != 5 || user->GetPVPRoom() != target.room ||
            user->GetPvpIndex() != target.room_index ||
            target.room->get_user_seat(static_cast<int>(slot)) != user ||
            party->battle_data().member_finish_loading(0) != 0u ||
            (party->battle_data().member_start_map_finish_loading(0) == 1u) !=
                ingress_loaded ||
            target.map_loaded[slot] != ingress_loaded ||
            party->battle_field().get_dungeon_index() !=
                static_cast<int>(target.play_policy.dungeon_index)) {
            return TARGET_LOAD_INVALID;
        }
    }
    return target.loaded_mask == target.expected_mask
               ? TARGET_LOAD_COMPLETE
               : TARGET_LOAD_PENDING;
}

void TickAt(std::time_t now, uint32_t now_ms) {
    std::vector<uint32_t> retry_routes;
    std::vector<uint32_t> requeue_sources;
    std::vector<uint32_t> abort_sources;
    std::vector<uint32_t> start_targets;
    std::vector<uint32_t> degrade_targets;
    std::vector<uint32_t> select_targets;
    std::vector<uint32_t> finish_targets;
    std::vector<uint32_t> complete_targets;
    std::vector<uint32_t> abort_targets;
    std::vector<std::pair<uint32_t, PvP_Room*> > result_targets;
    std::vector<std::pair<uint32_t, PvP_Room*> >
        delivery_timeout_targets;

    {
        MutexGuard guard(&g_mutex);
        if (!g_started) return;

        for (std::map<uint32_t, SourceCohort>::iterator source =
                 g_sources.begin(); source != g_sources.end(); ++source) {
            if (source->second.deadline_at == 0 ||
                now < source->second.deadline_at) continue;

            // Clear before dropping the mutex so concurrent packet-triggered
            // Tick calls cannot schedule the same terminal action twice.
            source->second.deadline_at = 0;
            if (source->second.state == SOURCE_ROUTE_PENDING &&
                source->second.route_attempts < kMaximumRouteAttempts) {
                source->second.pending_request_token = 0u;
                source->second.state = SOURCE_BUILDING;
                retry_routes.push_back(source->first);
            } else if (source->second.state == SOURCE_BUILDING ||
                       (source->second.state == SOURCE_ROUTE_PENDING &&
                        source->second.cohort_id == 0u)) {
                // No durable cohort id exists, so preserve valid queue groups
                // and roll back the temporary room after the bounded retry.
                requeue_sources.push_back(source->first);
            } else {
                // ROUTE_READY and every handoff state already own durable or
                // externally visible state; fail closed through cohort abort.
                abort_sources.push_back(source->first);
            }
        }

        for (std::map<uint32_t, TargetCohort>::iterator target =
                 g_targets.begin(); target != g_targets.end(); ++target) {
            if (target->second.room &&
                (target->second.state == TARGET_RUNNING ||
                 target->second.state == TARGET_FINISHING ||
                 target->second.state == TARGET_SETTLING)) {
                result_targets.push_back(
                    std::make_pair(target->first, target->second.room));
            }
            if (target->second.state == TARGET_CONNECTING &&
                target->second.deadline_at != 0 &&
                now >= target->second.deadline_at) {
                target->second.deadline_at = 0;
                cMyTrace trace(
                    "channel_zombie_matching_authority::TickAt",
                    __LINE__, 5);
                trace("[ZOMBIE_MATCH][P2P_TIMEOUT] room=%d cohort=%u ready=0x%02x expected=0x%02x count=%u action=abort",
                      target->second.room_index,
                      static_cast<unsigned int>(target->first),
                      static_cast<unsigned int>(
                          target->second.p2p_ready_mask),
                      static_cast<unsigned int>(
                          target->second.p2p_expected_mask),
                      static_cast<unsigned int>(CountGlobalSlots(
                          target->second.p2p_ready_mask)));
                abort_targets.push_back(target->first);
                continue;
            }
            if (target->second.state == TARGET_LOADING) {
                // TryPublishZombieP2PBarrier changes the phase before sending
                // NOTI279 so early CMD40 packets can be retained.  While its
                // publication lease is held, neither the main-loop tick nor a
                // hidden-owner timer may freeze/start (or degrade) the cohort
                // ahead of that official P2P completion frame.
                if (target->second.bootstrap_publish_in_progress) {
                    continue;
                }
                const TargetLoadStatus load_status =
                    RefreshTargetMapLoadingLocked(target->second);
                if (load_status == TARGET_LOAD_INVALID) {
                    target->second.deadline_at = 0;
                    abort_targets.push_back(target->first);
                    continue;
                }
                if (load_status == TARGET_LOAD_COMPLETE) {
                    if (!FreezeSixPlayerLoadLocked(target->second)) {
                        target->second.deadline_at = 0;
                        abort_targets.push_back(target->first);
                        continue;
                    }
                    start_targets.push_back(target->first);
                } else if (target->second.deadline_at != 0 &&
                           now >= target->second.deadline_at) {
                    const size_t loaded_count =
                        CountGlobalSlots(target->second.loaded_mask);
                    target->second.deadline_at = 0;
                    cMyTrace trace(
                        "channel_zombie_matching_authority::TickAt",
                        __LINE__, 5);
                    if (loaded_count ==
                        zombie_matching_runtime_policy::
                            kOfficialMinimumMembers) {
                        target->second.state = TARGET_DEGRADING;
                        degrade_targets.push_back(target->first);
                        trace("[ZOMBIE_MATCH][LOAD_TIMEOUT] room=%d load_epoch=%u loaded=0x%02x expected=0x%02x count=%u action=fallback5",
                              target->second.room_index,
                              static_cast<unsigned int>(
                                  target->second.load_epoch),
                              static_cast<unsigned int>(
                                  target->second.loaded_mask),
                              static_cast<unsigned int>(
                                  target->second.expected_mask),
                              static_cast<unsigned int>(loaded_count));
                    } else {
                        abort_targets.push_back(target->first);
                        trace("[ZOMBIE_MATCH][LOAD_TIMEOUT] room=%d load_epoch=%u loaded=0x%02x expected=0x%02x count=%u action=abort",
                              target->second.room_index,
                              static_cast<unsigned int>(
                                  target->second.load_epoch),
                              static_cast<unsigned int>(
                                  target->second.loaded_mask),
                              static_cast<unsigned int>(
                                  target->second.expected_mask),
                              static_cast<unsigned int>(loaded_count));
                    }
                    continue;
                }
            }
            if (target->second.state == TARGET_READY &&
                TickDeadlineReached(now_ms,
                                    target->second.ready_deadline_ms)) {
                target->second.ready_deadline_ms = 0u;
                target->second.state = TARGET_SELECTING;
                select_targets.push_back(target->first);
            }
            if (target->second.state == TARGET_RUNNING &&
                TickDeadlineReached(now_ms,
                                    target->second.play_deadline_ms)) {
                target->second.play_deadline_ms = 0u;
                target->second.state = TARGET_FINISHING;
                finish_targets.push_back(target->first);
                continue;
            }
            if (target->second.state == TARGET_SETTLING) {
                if (target->second.deadline_at != 0 &&
                    now >= target->second.deadline_at) {
                    target->second.deadline_at = 0;
                    delivery_timeout_targets.push_back(
                        std::make_pair(target->first,
                                       target->second.room));
                }
                continue;
            }
            if (target->second.deadline_at == 0 ||
                now < target->second.deadline_at) continue;
            target->second.deadline_at = 0;
            abort_targets.push_back(target->first);
        }
    }

    for (size_t index = 0u; index < retry_routes.size(); ++index) {
        if (!QueueRouteRequest(retry_routes[index], now))
            RequeueSourceToken(retry_routes[index], now);
    }
    for (size_t index = 0u; index < requeue_sources.size(); ++index)
        RequeueSourceToken(requeue_sources[index], now);
    for (size_t index = 0u; index < abort_sources.size(); ++index)
        AbortSourceToken(abort_sources[index]);
    for (size_t index = 0u; index < start_targets.size(); ++index) {
        if (!StartTargetCohort(start_targets[index]))
            AbortTargetCohort(start_targets[index]);
    }
    for (size_t index = 0u; index < degrade_targets.size(); ++index) {
        if (!DegradeTargetCohortToFive(degrade_targets[index]))
            AbortTargetCohort(degrade_targets[index]);
    }
    for (size_t index = 0u; index < select_targets.size(); ++index) {
        if (!SelectTargetCohortInitialRoles(select_targets[index]))
            AbortTargetCohort(select_targets[index]);
    }
    for (size_t index = 0u; index < finish_targets.size(); ++index) {
        if (!FinishTargetCohortByTimeout(finish_targets[index], now))
            AbortTargetCohort(finish_targets[index]);
    }
    // Result telemetry and delivery own a different mutex/DB queue.  Work on
    // room snapshots outside g_mutex, then re-lock only to transition the
    // exact still-matching cohort.  TickAtForTest follows this same path.
    for (size_t index = 0u; index < result_targets.size(); ++index) {
        zombie_mode::TickResultTelemetry(
            result_targets[index].second, now_ms);
    }
    for (size_t index = 0u;
         index < delivery_timeout_targets.size();
         ++index) {
        zombie_mode::ForceResultDeliveryTimeout(
            delivery_timeout_targets[index].second, now_ms);
    }
    for (size_t index = 0u; index < result_targets.size(); ++index) {
        const uint32_t cohort_id = result_targets[index].first;
        PvP_Room* const room = result_targets[index].second;
        const bool finished = zombie_mode::IsFinishedRoom(room);
        const bool delivered =
            zombie_mode::IsResultDeliveryComplete(room);
        bool schedule_complete = false;
        {
            MutexGuard guard(&g_mutex);
            std::map<uint32_t, TargetCohort>::iterator target =
                g_targets.find(cohort_id);
            if (target == g_targets.end() ||
                target->second.room != room) continue;
            if (finished &&
                (target->second.state == TARGET_RUNNING ||
                 target->second.state == TARGET_FINISHING)) {
                target->second.state = TARGET_SETTLING;
                target->second.play_deadline_ms = 0u;
                target->second.deadline_at =
                    now + kZombieSettlementGraceSeconds;
            }
            if (delivered &&
                target->second.state == TARGET_SETTLING) {
                target->second.deadline_at = 0;
                schedule_complete = true;
            } else if (!delivered &&
                       target->second.state == TARGET_SETTLING &&
                       target->second.deadline_at == 0) {
                // A transient identity/send race at the hard deadline must
                // retry the terminal zero-reward delivery instead of leaving
                // the cohort permanently resident.
                target->second.deadline_at = now + 1;
            }
        }
        if (schedule_complete &&
            std::find(complete_targets.begin(), complete_targets.end(),
                      cohort_id) == complete_targets.end()) {
            complete_targets.push_back(cohort_id);
        }
    }
    for (size_t index = 0u; index < complete_targets.size(); ++index)
        CompleteTargetCohort(complete_targets[index]);
    for (size_t index = 0u; index < abort_targets.size(); ++index)
        AbortTargetCohort(abort_targets[index]);

    uint32_t cohort_token = 0u;
    LobbyUiUpdates ui_updates;
    {
        MutexGuard guard(&g_mutex);
        if (!g_started || g_route_retry_at == 0 ||
            now < g_route_retry_at) return;
        PurgeInvalidQueueLocked(ui_updates);
        cohort_token = TakeQueuedCohortLocked(now);
        if (!cohort_token) g_route_retry_at = 0;
    }
    DispatchLobbyUiUpdates(ui_updates);
    LaunchSourceCohort(cohort_token, now);
}

}  // namespace

void Tick() {
    TickAt(std::time(0), CurrentTickMilliseconds());
}

void TickAtForTest(uint32_t now_seconds) {
    TickAt(static_cast<std::time_t>(now_seconds), now_seconds * 1000u);
}

void OnRoomReset(PvP_Room* room) {
    if (!room) return;
    // THREADING_CONTRACT: local same-channel bootstrap publication runs from
    // MainLoop::Tick, while gameplay room resets run on the one DispatchThread
    // and can overlap it.  Remote activation already runs on DispatchThread.
    // The single dispatcher permits at most one external reset waiter for a
    // room.  Wait here, before PvP_Room::reset mutates any field; the publisher
    // never holds g_mutex while sending and always clears/broadcasts its lease.
    // A future worker-thread direct reset must preserve this one-waiter rule or
    // introduce a generation-bearing room-lifecycle owner before pool return.
    const bool finished_zombie_room = zombie_mode::IsFinishedRoom(room);
    const bool result_delivery_complete =
        zombie_mode::IsResultDeliveryComplete(room);
    uint32_t abort_source = 0u;
    TargetCohort completed_target;
    bool target_completed = false;
    g_mutex.lock();
    std::map<PvP_Room*, uint32_t>::iterator publishing_room =
        g_target_rooms.find(room);
    while (publishing_room != g_target_rooms.end()) {
        std::map<uint32_t, TargetCohort>::iterator publishing_target =
            g_targets.find(publishing_room->second);
        if (publishing_target == g_targets.end() ||
            publishing_target->second.room != room ||
            !publishing_target->second.bootstrap_publish_in_progress) {
            break;
        }
        publishing_target->second.abort_pending = true;
        publishing_target->second.external_reset_pending = true;
        publishing_target->second.deadline_at = 0;
        pthread_cond_wait(&g_bootstrap_publish_cond, g_mutex.native());
        publishing_room = g_target_rooms.find(room);
    }
    {
        std::map<PvP_Room*, uint32_t>::iterator source_room =
            g_source_rooms.find(room);
        if (source_room != g_source_rooms.end()) {
            const uint32_t source_token = source_room->second;
            std::map<uint32_t, SourceCohort>::iterator source =
                g_sources.find(source_token);
            if (source != g_sources.end() &&
                source->second.state == SOURCE_ARMED) {
                // Only the durable ALL_ARMED barrier makes an empty/reset
                // source room an expected transfer.  Keep the cohort authority
                // until every NOTI606 send succeeds: the final CheckOutPvp
                // resets this room synchronously, and a later send failure must
                // still be able to queue OPERATION_ABORT_COHORT.  The explicit
                // release hook removes this state after all notifications.
                source->second.room = 0;
                source->second.room_index = -1;
                g_source_rooms.erase(source_room);
            } else {
                if (source != g_sources.end()) {
                    source->second.room = 0;
                    source->second.room_index = -1;
                    source->second.deadline_at = 0;
                    abort_source = source_token;
                }
                g_source_rooms.erase(source_room);
            }
        }
        std::map<PvP_Room*, uint32_t>::iterator target_room =
            g_target_rooms.find(room);
        if (target_room != g_target_rooms.end()) {
            target_completed =
                TakeTargetLocked(target_room->second, completed_target);
        }
    }
    g_mutex.unlock();
    matching_channel_handoff::UnbindMatchingRoom(room);
    if (abort_source) AbortSourceToken(abort_source);
    if (target_completed) {
        if (finished_zombie_room && result_delivery_complete) {
            QueueComplete(completed_target);
        } else {
            QueueTargetAbortControl(completed_target);
        }
        ReleaseTargetMapOwnersForExternalRoomReset(
            room, completed_target);
        ResetClientSceneAfterTargetRollback(
            completed_target,
            completed_target.state != TARGET_SETTLING &&
                (!finished_zombie_room || !result_delivery_complete));
        // Non-finished resets use the slot-0 CLAIMED proof and never emit OP8.
    }
}

bool OnSourceCohortDetachReady(uint32_t cohort_id) {
    SourceCohort snapshot;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, uint32_t>::const_iterator token =
            g_source_cohorts.find(cohort_id);
        if (token == g_source_cohorts.end()) return false;
        std::map<uint32_t, SourceCohort>::iterator source =
            g_sources.find(token->second);
        if (source == g_sources.end() ||
            source->second.state != SOURCE_HANDOFF_PENDING) return false;
        source->second.state = SOURCE_DETACHING;
        source->second.deadline_at =
            std::time(0) + kSourceHandoffDeadlineSeconds;
        snapshot = source->second;
    }
    return CheckOutPreformedGroups(snapshot);
}

bool OnSourceCohortArmed(uint32_t cohort_id) {
    MutexGuard guard(&g_mutex);
    std::map<uint32_t, uint32_t>::const_iterator token =
        g_source_cohorts.find(cohort_id);
    if (token == g_source_cohorts.end()) return false;
    std::map<uint32_t, SourceCohort>::iterator source =
        g_sources.find(token->second);
    if (source == g_sources.end() ||
        source->second.state != SOURCE_DETACHING) return false;
    source->second.state = SOURCE_ARMED;
    source->second.deadline_at =
        std::time(0) + kSourceReleaseDeadlineSeconds;
    return true;
}

void OnSourceCohortReleased(uint32_t cohort_id) {
    MutexGuard guard(&g_mutex);
    std::map<uint32_t, uint32_t>::iterator token =
        g_source_cohorts.find(cohort_id);
    if (token == g_source_cohorts.end()) return;
    std::map<uint32_t, SourceCohort>::iterator source =
        g_sources.find(token->second);
    if (source == g_sources.end() ||
        source->second.state != SOURCE_ARMED) return;
    if (source->second.room) g_source_rooms.erase(source->second.room);
    g_sources.erase(source);
    g_source_cohorts.erase(token);
}

void AbortCohort(uint32_t cohort_id) {
    uint32_t source_token = 0u;
    bool target = false;
    {
        MutexGuard guard(&g_mutex);
        std::map<uint32_t, uint32_t>::const_iterator source =
            g_source_cohorts.find(cohort_id);
        if (source != g_source_cohorts.end()) source_token = source->second;
        target = g_targets.find(cohort_id) != g_targets.end();
    }
    if (source_token) AbortSourceToken(source_token);
    if (target) AbortTargetCohort(cohort_id);
}

}  // namespace channel_zombie_matching_authority
