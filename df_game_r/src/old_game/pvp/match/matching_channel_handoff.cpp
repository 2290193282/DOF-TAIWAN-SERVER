#include "old_game/pvp/match/matching_channel_handoff.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "core/config/env/environment.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/pvp_party/db_matching_channel_handoff.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace matching_channel_handoff {
namespace {

using matching_channel_handoff_db::Message;

const char kMatchingModePvf[] = "Etc/matchingmode.etc";
const uint8_t kChannelMatchingBound = 1u;
const uint16_t kZombieMatchingMode = 15u;
const int kMinimumMatchedPlayers = 5;
const int kMaximumMatchedPlayers = 6;
const unsigned int kMaximumDatabaseRetries = 3u;

enum ContainerKind {
    CONTAINER_NONE = 0,
    CONTAINER_ZOMBIE_PARTY = 1,
    CONTAINER_PVP_ROOM = 2,
};

enum SessionOrigin {
    SESSION_SOURCE = 1,
    SESSION_TARGET = 2,
};

struct Identity {
    Identity()
        : account_id(0u), charac_no(0u), world_uid(-1), server_group(-1) {}

    uint32_t account_id;
    uint32_t charac_no;
    int world_uid;
    int server_group;
};

struct Binding {
    Binding()
        : kind(CONTAINER_NONE), container(0), container_index(-1),
          matching_bound(0u), matching_mode(0u), detaching(false), owner() {}

    ContainerKind kind;
    void* container;
    int container_index;
    uint8_t matching_bound;
    uint16_t matching_mode;
    bool detaching;
    Identity owner;
};

struct Session {
    Session()
        : origin(SESSION_SOURCE), token(0u), next_request_token(1u),
          pending_request_token(0u), pending_operation(0u),
          database_retries(0u), ticket_owned(false), source_cleaned(false),
          armed(false), cohort_barrier_released(false),
          notification_sent(false), claimed(false), owner(), ticket() {
        std::memset(&ticket, 0, sizeof(ticket));
    }

    SessionOrigin origin;
    uint32_t token;
    uint32_t next_request_token;
    uint32_t pending_request_token;
    uint8_t pending_operation;
    unsigned int database_retries;
    bool ticket_owned;
    bool source_cleaned;
    bool armed;
    bool cohort_barrier_released;
    bool notification_sent;
    bool claimed;
    Identity owner;
    Message ticket;
};

class ScriptUnloadGuard {
public:
    ScriptUnloadGuard() {}
    ~ScriptUnloadGuard() { unloadRDARScriptFile(); }

private:
    ScriptUnloadGuard(const ScriptUnloadGuard&);
    ScriptUnloadGuard& operator=(const ScriptUnloadGuard&);
};

Mutex g_handoff_mutex;
std::map<CUser*, Binding> g_bindings;
std::map<CUser*, Session> g_sessions;
std::map<uint32_t, uint8_t> g_cohort_barriers;
uint32_t g_next_session_token = 1u;
bool g_policy_attempted = false;
bool g_policy_loaded = false;
ClaimReadyCallback g_claim_ready_callback = 0;

void ForgetCohortBarrierIfUnusedLocked(uint32_t cohort_id) {
    if (!cohort_id) return;
    for (std::map<CUser*, Session>::const_iterator it = g_sessions.begin();
         it != g_sessions.end(); ++it) {
        if (it->second.ticket.field1 == cohort_id) return;
    }
    g_cohort_barriers.erase(cohort_id);
}

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) result = value++;
    if (value == 0u) ++value;
    return result;
}

Identity MakeIdentity(CUser* user) {
    Identity identity;
    if (!user) return identity;
    identity.account_id = static_cast<uint32_t>(user->get_acc_id());
    identity.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    identity.world_uid = user->GetUID();
    identity.server_group = user->GetServerGroup();
    return identity;
}

bool SameIdentity(const Identity& lhs, const Identity& rhs) {
    return lhs.account_id == rhs.account_id &&
           lhs.charac_no == rhs.charac_no &&
           lhs.world_uid == rhs.world_uid &&
           lhs.server_group == rhs.server_group;
}

bool IsLiveIdentity(CUser* user, const Identity& expected) {
    return user && user->get_state() >= 3 && expected.account_id != 0u &&
           expected.charac_no != 0u && expected.world_uid >= 0 &&
           SameIdentity(MakeIdentity(user), expected);
}

bool IsChannelPair(uint8_t matching_bound, uint16_t matching_mode) {
    // The existing CMD518/519 owner fixes integrated=0.  The 140828 client
    // matching-mode constructor preserves PVF record order and initializes
    // none=2; therefore the first official "channel" record is exactly 1.
    return matching_bound == kChannelMatchingBound &&
           matching_mode == kZombieMatchingMode;
}

bool IsChannelNumber(int channel) {
    return channel > 0 && channel <= 65535;
}

bool IsRuntimeCohortSize(size_t value) {
    return zombie_matching_runtime_policy::IsRuntimeCohortSize(value);
}

bool ParseOfficialPolicy() {
    if (!loadRDARScriptFile("", kMatchingModePvf)) return false;
    ScriptUnloadGuard unload;

    enum Bits {
        BIT_BOUND = 1u << 0,
        BIT_GAMETYPE = 1u << 1,
        BIT_STYLE = 1u << 2,
        BIT_VERSUS = 1u << 3,
        BIT_SCENE = 1u << 4,
        BIT_SLOTS = 1u << 5,
        BIT_CUTSCENE = 1u << 6,
        BIT_MINIMUM = 1u << 7,
        BIT_MAXIMUM = 1u << 8,
        BIT_JOBS = 1u << 9,
    };
    const unsigned int complete = (1u << 10) - 1u;
    unsigned int mask = 0u;
    bool in_first_record = false;
    bool closed = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (!in_first_record) {
            if (tag != "[matching info]") return false;
            in_first_record = true;
            continue;
        }
        if (tag == "[/matching info]") {
            closed = true;
            break;
        }
        if (tag == "[/slots]" || tag == "[/impossible jobs]") {
            continue;
        }
        if (tag == "[style]") {
            if (mask & BIT_STYLE) return false;
            mask |= BIT_STYLE;
            continue;
        }
        if (tag == "[versus]") {
            if (mask & BIT_VERSUS) return false;
            mask |= BIT_VERSUS;
            continue;
        }

        std::string text;
        bool success = false;
        if (tag == "[matching bound]") {
            if ((mask & BIT_BOUND) || ScanStr(&text) != 1 ||
                text != "channel") return false;
            mask |= BIT_BOUND;
        } else if (tag == "[gametype]") {
            if ((mask & BIT_GAMETYPE) || ScanStr(&text) != 1 ||
                text != "zombie") return false;
            mask |= BIT_GAMETYPE;
        } else if (tag == "[scene type]") {
            if ((mask & BIT_SCENE) || ScanStr(&text) != 1 ||
                text != "zombie") return false;
            mask |= BIT_SCENE;
        } else if (tag == "[slots]") {
            int left = 0;
            int right = 0;
            if ((mask & BIT_SLOTS) || ScanStr(&text) != 1 || text != "left")
                return false;
            left = ScanInt(&success);
            if (!success || left != 3 || ScanStr(&text) != 1 ||
                text != "right") return false;
            right = ScanInt(&success);
            if (!success || right != 3) return false;
            mask |= BIT_SLOTS;
        } else if (tag == "[cutscene]") {
            const int value = ScanInt(&success);
            if ((mask & BIT_CUTSCENE) || !success || value != 0) return false;
            mask |= BIT_CUTSCENE;
        } else if (tag == "[min user count]") {
            const int value = ScanInt(&success);
            if ((mask & BIT_MINIMUM) || !success ||
                value != kMinimumMatchedPlayers) return false;
            mask |= BIT_MINIMUM;
        } else if (tag == "[max user count]") {
            const int value = ScanInt(&success);
            if ((mask & BIT_MAXIMUM) || !success ||
                value != kMaximumMatchedPlayers) return false;
            mask |= BIT_MAXIMUM;
        } else if (tag == "[impossible jobs]") {
            if ((mask & BIT_JOBS) || ScanStr(&text) != 1 ||
                text != "creator mage") return false;
            mask |= BIT_JOBS;
        } else {
            return false;
        }
    }
    return in_first_record && closed && mask == complete;
}

bool IsZombieParty(CParty* party) {
    CDungeon* const dungeon = party ? party->battle_field().get_dungeon() : 0;
    return dungeon && dungeon->isZombieDungeon();
}

int FindPartySeat(CParty* party, CUser* user) {
    if (!party || !user) return -1;
    for (int seat = 0; seat < 4; ++seat) {
        if (party->get_user(seat) == user) return seat;
    }
    return -1;
}

bool IsBindingLive(CUser* user, const Binding& binding) {
    if (!IsLiveIdentity(user, binding.owner) || !binding.container ||
        !IsChannelPair(binding.matching_bound, binding.matching_mode)) {
        return false;
    }
    if (binding.kind == CONTAINER_ZOMBIE_PARTY) {
        CParty* const party = static_cast<CParty*>(binding.container);
        return IsZombieParty(party) &&
               party->GetPartyIndex() == binding.container_index &&
               user->CheckInParty() == 1 &&
               user->GetPartyIndex() == binding.container_index &&
               FindPartySeat(party, user) >= 0;
    }
    if (binding.kind == CONTAINER_PVP_ROOM) {
        PvP_Room* const room = static_cast<PvP_Room*>(binding.container);
        const int seat = room->get_user_seat(user);
        return room->get_index() == binding.container_index &&
               user->CheckInPvp() == 1 &&
               user->GetPvpIndex() == binding.container_index &&
               seat >= 0 && seat < 8 && room->get_user_seat(seat) == user;
    }
    return false;
}

bool QueueTracked(CUser* user, Message request) {
    uint32_t session_token = 0u;
    uint32_t request_token = 0u;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.pending_operation ||
            !IsLiveIdentity(user, found->second.owner)) return false;
        session_token = found->second.token;
        request_token = NextNonZero(found->second.next_request_token);
        request.account_id = found->second.owner.account_id;
        request.charac_no = found->second.owner.charac_no;
        request.session_token = session_token;
        request.request_token = request_token;
        request.result = matching_channel_handoff_db::RESULT_SUCCESS;
        request.reply_required = 0u;
        request.cohort_flags = 0u;
        found->second.pending_operation = request.operation;
        found->second.pending_request_token = request_token;
        found->second.ticket = request;
    }
    if (matching_channel_handoff_db::
            DBMatchingChannelHandoff::QueueRequest(
                user->GetUID(), request)) return true;

    MutexGuard guard(&g_handoff_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() && found->second.token == session_token &&
        found->second.pending_operation == request.operation &&
        found->second.pending_request_token == request_token) {
        found->second.pending_operation = 0u;
        found->second.pending_request_token = 0u;
    }
    return false;
}

void QueueCancelAndErase(CUser* user) {
    Message cancel;
    std::memset(&cancel, 0, sizeof(cancel));
    bool queue = false;
    int user_index = -1;
    uint32_t cohort_id = 0u;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end()) return;
        cancel = found->second.ticket;
        cancel.account_id = found->second.owner.account_id;
        cancel.charac_no = found->second.owner.charac_no;
        cancel.session_token = found->second.token;
        cancel.request_token = NextNonZero(found->second.next_request_token);
        cancel.operation = matching_channel_handoff_db::OPERATION_CANCEL;
        cancel.result = matching_channel_handoff_db::RESULT_SUCCESS;
        cancel.reply_required = 0u;
        cancel.cohort_flags = 0u;
        queue = found->second.ticket_owned && cancel.handoff_id != 0u &&
                 cancel.field4 == cancel.handoff_id &&
                (cancel.state ==
                     matching_channel_handoff_db::STATE_RESERVED ||
                 cancel.state ==
                     matching_channel_handoff_db::STATE_ARMED);
        user_index = found->second.owner.world_uid;
        cohort_id = found->second.ticket.field1;
        g_sessions.erase(found);
        ForgetCohortBarrierIfUnusedLocked(cohort_id);
    }
    if (queue) {
        (void)matching_channel_handoff_db::
            DBMatchingChannelHandoff::QueueRequest(user_index, cancel);
    }
}

bool DetachBoundSource(CUser* user) {
    Binding binding;
    uint32_t session_token = 0u;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator session = g_sessions.find(user);
        std::map<CUser*, Binding>::iterator bound = g_bindings.find(user);
        if (session == g_sessions.end() ||
            session->second.origin != SESSION_SOURCE) return false;
        if (session->second.source_cleaned) return true;
        if (bound == g_bindings.end() || bound->second.detaching ||
            !SameIdentity(bound->second.owner, session->second.owner) ||
            bound->second.matching_bound !=
                session->second.ticket.matching_bound ||
            bound->second.matching_mode !=
                session->second.ticket.matching_mode ||
            !IsBindingLive(user, bound->second)) return false;
        bound->second.detaching = true;
        binding = bound->second;
        session_token = session->second.token;
    }

    if (binding.kind == CONTAINER_ZOMBIE_PARTY) {
        G_CGameManager()->CheckOutParty(user, false);
    } else if (binding.kind == CONTAINER_PVP_ROOM) {
        G_CGameManager()->CheckOutPvp(user, false);
    }
    const bool cleaned = !IsBindingLive(user, binding);

    MutexGuard guard(&g_handoff_mutex);
    std::map<CUser*, Session>::iterator session = g_sessions.find(user);
    std::map<CUser*, Binding>::iterator bound = g_bindings.find(user);
    if (session == g_sessions.end() ||
        session->second.token != session_token) return false;
    if (bound != g_bindings.end() &&
        bound->second.container == binding.container &&
        bound->second.container_index == binding.container_index &&
        bound->second.kind == binding.kind) {
        if (cleaned) {
            g_bindings.erase(bound);
        } else {
            bound->second.detaching = false;
        }
    }
    if (cleaned) session->second.source_cleaned = true;
    return cleaned;
}

bool SendPendingNotification(CUser* user) {
    Message ticket;
    uint32_t token = 0u;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() ||
            found->second.origin != SESSION_SOURCE ||
            !found->second.armed || !found->second.source_cleaned ||
            found->second.claimed || found->second.pending_operation ||
            !IsLiveIdentity(user, found->second.owner)) return false;
        if (found->second.notification_sent) return true;
        ticket = found->second.ticket;
        token = found->second.token;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CHANNEL_MOVE_FOR_MATCHING);
    packet.put_byte(ticket.field0);
    // 140828 project-owned contract: NOTI606.field1 is cohort_id.
    packet.put_int(ticket.field1);
    packet.put_int(ticket.endpoint_key);
    packet.put_int(ticket.endpoint_port_like);
    packet.put_int(ticket.field4);
    packet.finalize(true);
    if (user->Send(packet) != 1) return false;

    MutexGuard guard(&g_handoff_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() && found->second.token == token &&
        found->second.ticket.handoff_id == ticket.handoff_id) {
        found->second.notification_sent = true;
        return true;
    }
    return false;
}

bool QueueArm(CUser* user, const Message& reserved) {
    Message arm = reserved;
    arm.operation = matching_channel_handoff_db::OPERATION_ARM;
    arm.result = matching_channel_handoff_db::RESULT_SUCCESS;
    arm.state = matching_channel_handoff_db::STATE_RESERVED;
    arm.source_cleaned = 1u;
    return QueueTracked(user, arm);
}

enum BarrierResult {
    BARRIER_WAITING = 0,
    BARRIER_ADVANCED = 1,
    BARRIER_FAILED = 2,
};

bool IsCohortShapeValid(const Message& message) {
    return message.field1 != 0u &&
           IsRuntimeCohortSize(message.cohort_size) &&
           message.cohort_slot < message.cohort_size &&
           (message.cohort_flags &
                ~matching_channel_handoff_db::COHORT_FLAG_ALL) == 0u;
}

void RememberCohortFlags(const Message& message) {
    if (!message.field1) return;
    MutexGuard guard(&g_handoff_mutex);
    g_cohort_barriers[message.field1] |= message.cohort_flags;
}

BarrierResult AdvanceReservedCohort(uint32_t cohort_id,
                                    uint8_t cohort_size) {
    std::vector<CUser*> users;
    std::vector<Message> tickets;
    bool slots[kMaximumMatchedPlayers] = {false, false, false,
                                          false, false, false};
    {
        MutexGuard guard(&g_handoff_mutex);
        if ((g_cohort_barriers[cohort_id] &
             matching_channel_handoff_db::COHORT_FLAG_ALL_RESERVED) == 0u)
            return BARRIER_WAITING;
        for (std::map<CUser*, Session>::iterator it = g_sessions.begin();
             it != g_sessions.end(); ++it) {
            Session& session = it->second;
            if (session.origin != SESSION_SOURCE ||
                session.ticket.field1 != cohort_id) continue;
            if (!IsCohortShapeValid(session.ticket) ||
                session.ticket.cohort_size != cohort_size ||
                !session.ticket_owned || session.pending_operation ||
                session.ticket.state !=
                    matching_channel_handoff_db::STATE_RESERVED) {
                return BARRIER_WAITING;
            }
            const uint8_t slot = session.ticket.cohort_slot;
            if (slots[slot]) return BARRIER_FAILED;
            slots[slot] = true;
            users.push_back(it->first);
            tickets.push_back(session.ticket);
        }
        if (users.size() != cohort_size) return BARRIER_WAITING;
        for (size_t index = 0u; index < users.size(); ++index) {
            std::map<CUser*, Session>::iterator session =
                g_sessions.find(users[index]);
            if (session == g_sessions.end()) return BARRIER_FAILED;
            if (session->second.cohort_barrier_released)
                return BARRIER_WAITING;
            session->second.cohort_barrier_released = true;
        }
    }

    if (!channel_zombie_matching_authority::
            OnSourceCohortDetachReady(cohort_id)) {
        channel_zombie_matching_authority::AbortCohort(cohort_id);
        return BARRIER_FAILED;
    }
    // Persist the complete ARM barrier while the source room is still live.
    // No client receives NOTI606 until every response succeeds, so a crash in
    // this window cannot materialize a target claim.  Physical checkout moves
    // to NotifyArmedCohort, where OnRoomReset can distinguish true ALL_ARMED
    // from every earlier failure state.
    for (size_t index = 0u; index < users.size(); ++index) {
        if (!QueueArm(users[index], tickets[index])) {
            channel_zombie_matching_authority::AbortCohort(cohort_id);
            return BARRIER_FAILED;
        }
    }
    return BARRIER_ADVANCED;
}

BarrierResult NotifyArmedCohort(uint32_t cohort_id,
                                uint8_t cohort_size) {
    std::vector<CUser*> users;
    bool slots[kMaximumMatchedPlayers] = {false, false, false,
                                          false, false, false};
    {
        MutexGuard guard(&g_handoff_mutex);
        if ((g_cohort_barriers[cohort_id] &
             matching_channel_handoff_db::COHORT_FLAG_ALL_ARMED) == 0u)
            return BARRIER_WAITING;
        for (std::map<CUser*, Session>::iterator it = g_sessions.begin();
             it != g_sessions.end(); ++it) {
            Session& session = it->second;
            if (session.origin != SESSION_SOURCE ||
                session.ticket.field1 != cohort_id) continue;
            if (!IsCohortShapeValid(session.ticket) ||
                session.ticket.cohort_size != cohort_size ||
                !session.ticket_owned ||
                !session.ticket.source_cleaned ||
                !session.armed || session.pending_operation ||
                session.ticket.state !=
                    matching_channel_handoff_db::STATE_ARMED) {
                return BARRIER_WAITING;
            }
            const uint8_t slot = session.ticket.cohort_slot;
            if (slots[slot]) return BARRIER_FAILED;
            slots[slot] = true;
            session.ticket.cohort_flags = g_cohort_barriers[cohort_id];
            users.push_back(it->first);
        }
    }
    if (users.size() != cohort_size) return BARRIER_WAITING;
    if (!channel_zombie_matching_authority::
            OnSourceCohortArmed(cohort_id)) {
        channel_zombie_matching_authority::AbortCohort(cohort_id);
        return BARRIER_FAILED;
    }
    for (size_t index = 0u; index < users.size(); ++index) {
        if (!DetachBoundSource(users[index])) {
            channel_zombie_matching_authority::AbortCohort(cohort_id);
            return BARRIER_FAILED;
        }
    }
    for (size_t index = 0u; index < users.size(); ++index) {
        if (!SendPendingNotification(users[index])) {
            channel_zombie_matching_authority::AbortCohort(cohort_id);
            return BARRIER_FAILED;
        }
    }
    channel_zombie_matching_authority::OnSourceCohortReleased(cohort_id);
    return BARRIER_ADVANCED;
}

bool SameCohortRoute(const Message& expected, const Message& response) {
    return expected.field0 == response.field0 &&
           expected.field1 == response.field1 &&
           expected.endpoint_key == response.endpoint_key &&
           expected.endpoint_port_like == response.endpoint_port_like &&
           expected.source_server_group == response.source_server_group &&
           expected.source_channel == response.source_channel &&
           expected.target_server_group == response.target_server_group &&
           expected.target_channel == response.target_channel &&
           expected.matching_bound == response.matching_bound &&
           expected.matching_mode == response.matching_mode &&
           expected.cohort_slot == response.cohort_slot &&
           expected.cohort_size == response.cohort_size;
}

bool CorrelateResponse(CUser* user,
                       const Message& response,
                       Session& snapshot) {
    if (!user) return false;
    MutexGuard guard(&g_handoff_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() ||
        !IsLiveIdentity(user, found->second.owner) ||
        found->second.owner.account_id != response.account_id ||
        found->second.owner.charac_no != response.charac_no ||
        found->second.token != response.session_token ||
        found->second.pending_operation != response.operation ||
        found->second.pending_request_token != response.request_token) {
        return false;
    }
    if (response.result == matching_channel_handoff_db::RESULT_SUCCESS &&
        response.operation != matching_channel_handoff_db::OPERATION_CLAIM &&
        !SameCohortRoute(found->second.ticket, response)) {
        return false;
    }
    found->second.pending_operation = 0u;
    found->second.pending_request_token = 0u;
    const bool committed_claim_replay =
        response.operation == matching_channel_handoff_db::OPERATION_CLAIM &&
        response.result == matching_channel_handoff_db::RESULT_REPLAY &&
        response.state == matching_channel_handoff_db::STATE_CLAIMED &&
        response.source_cleaned;
    if (response.result == matching_channel_handoff_db::RESULT_SUCCESS ||
        committed_claim_replay)
        found->second.ticket = response;
    if (response.operation ==
        matching_channel_handoff_db::OPERATION_RESERVE) {
        // A BUSY reserve response contains the pre-existing durable row so
        // the caller can diagnose the conflict.  It is not this session's
        // ticket and must never be canceled by source cleanup.
        found->second.ticket_owned =
            response.result == matching_channel_handoff_db::RESULT_SUCCESS &&
            response.handoff_id != 0u &&
            response.field4 == response.handoff_id &&
            (response.state ==
                 matching_channel_handoff_db::STATE_RESERVED ||
             response.state ==
                 matching_channel_handoff_db::STATE_ARMED);
    }
    snapshot = found->second;
    return true;
}

bool RetryDatabaseFailure(CUser* user, const Message& response) {
    bool retry = false;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found != g_sessions.end() &&
            found->second.token == response.session_token &&
            found->second.database_retries < kMaximumDatabaseRetries) {
            ++found->second.database_retries;
            retry = true;
        }
    }
    if (!retry) return false;
    Message request = response;
    request.result = matching_channel_handoff_db::RESULT_SUCCESS;
    return QueueTracked(user, request);
}

bool SameBeginRequest(const Session& session,
                      uint8_t matching_bound,
                      uint16_t matching_mode,
                      uint8_t field0,
                      uint32_t cohort_id,
                      uint32_t endpoint_key,
                      uint16_t endpoint_port_like,
                      int target_server_group,
                      int target_channel,
                      uint8_t cohort_slot,
                      uint8_t cohort_size) {
    const Message& ticket = session.ticket;
    return session.origin == SESSION_SOURCE &&
           ticket.matching_bound == matching_bound &&
           ticket.matching_mode == matching_mode &&
           ticket.field0 == field0 && ticket.field1 == cohort_id &&
           ticket.endpoint_key == endpoint_key &&
           ticket.endpoint_port_like == endpoint_port_like &&
           ticket.target_server_group == target_server_group &&
           ticket.target_channel == target_channel &&
           ticket.cohort_slot == cohort_slot &&
           ticket.cohort_size == cohort_size;
}

void TraceRejected(const char* operation, CUser* user, uint32_t handoff_id) {
    LogManager::logFormat(
        1, "matching_channel_handoff_0828.cpp", operation, 0,
        "[MATCHING_HANDOFF_0828] uid=%d charac=%u handoff=%u rejected",
        user ? user->GetUID() : -1,
        user ? static_cast<unsigned int>(user->getCurCharacNo()) : 0u,
        static_cast<unsigned int>(handoff_id));
}

}  // namespace

bool LoadMatchingModePolicy() {
    MutexGuard guard(&g_handoff_mutex);
    if (!g_policy_attempted) {
        g_policy_attempted = true;
        g_policy_loaded = ParseOfficialPolicy();
    }
    return g_policy_loaded;
}

bool IsMatchingModePolicyLoaded() {
    MutexGuard guard(&g_handoff_mutex);
    return g_policy_loaded;
}

bool BindZombieParty(CParty* party,
                     uint8_t matching_bound,
                     uint16_t matching_mode) {
    if (!IsMatchingModePolicyLoaded() || !IsChannelPair(
            matching_bound, matching_mode) || !IsZombieParty(party) ||
        party->GetPartyIndex() < 0) return false;
    CUser* users[4] = {0, 0, 0, 0};
    int user_count = 0;
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const user = party->get_user(seat);
        if (!user) continue;
        const Identity identity = MakeIdentity(user);
        if (!IsLiveIdentity(user, identity) || user->CheckInParty() != 1 ||
            user->GetPartyIndex() != party->GetPartyIndex()) return false;
        users[user_count++] = user;
    }
    if (user_count == 0) return false;

    MutexGuard guard(&g_handoff_mutex);
    for (int index = 0; index < user_count; ++index) {
        CUser* const user = users[index];
        const Identity identity = MakeIdentity(user);
        std::map<CUser*, Binding>::const_iterator prior =
            g_bindings.find(user);
        if (g_sessions.find(user) != g_sessions.end() ||
            (prior != g_bindings.end() &&
             (prior->second.container != party ||
              !SameIdentity(prior->second.owner, identity)))) return false;
    }
    for (int index = 0; index < user_count; ++index) {
        CUser* const user = users[index];
        Binding binding;
        binding.kind = CONTAINER_ZOMBIE_PARTY;
        binding.container = party;
        binding.container_index = party->GetPartyIndex();
        binding.matching_bound = matching_bound;
        binding.matching_mode = matching_mode;
        binding.owner = MakeIdentity(user);
        g_bindings[user] = binding;
    }
    return true;
}

bool BindMatchingRoom(PvP_Room* room,
                      uint8_t matching_bound,
                      uint16_t matching_mode) {
    if (!IsMatchingModePolicyLoaded() || !room ||
        !IsChannelPair(matching_bound, matching_mode) ||
        room->get_index() < 0) return false;
    CUser* users[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int count = 0;
    for (int seat = 0; seat < 8; ++seat) {
        CUser* const user = room->get_user_seat(seat);
        if (!user) continue;
        const Identity identity = MakeIdentity(user);
        if (!IsLiveIdentity(user, identity) || user->CheckInPvp() != 1 ||
            user->GetPvpIndex() != room->get_index() ||
            room->get_user_seat(user) != seat) return false;
        users[count++] = user;
    }
    if (!IsRuntimeCohortSize(static_cast<size_t>(count)))
        return false;

    MutexGuard guard(&g_handoff_mutex);
    for (int index = 0; index < count; ++index) {
        CUser* const user = users[index];
        const Identity identity = MakeIdentity(user);
        std::map<CUser*, Binding>::const_iterator prior =
            g_bindings.find(user);
        if (g_sessions.find(user) != g_sessions.end() ||
            (prior != g_bindings.end() &&
             (prior->second.container != room ||
              !SameIdentity(prior->second.owner, identity)))) return false;
    }
    for (int index = 0; index < count; ++index) {
        CUser* const user = users[index];
        Binding binding;
        binding.kind = CONTAINER_PVP_ROOM;
        binding.container = room;
        binding.container_index = room->get_index();
        binding.matching_bound = matching_bound;
        binding.matching_mode = matching_mode;
        binding.owner = MakeIdentity(user);
        g_bindings[user] = binding;
    }
    return true;
}

void UnbindZombieParty(CParty* party) {
    if (!party) return;
    MutexGuard guard(&g_handoff_mutex);
    for (std::map<CUser*, Binding>::iterator it = g_bindings.begin();
         it != g_bindings.end();) {
        if (it->second.kind == CONTAINER_ZOMBIE_PARTY &&
            it->second.container == party) {
            g_bindings.erase(it++);
        } else {
            ++it;
        }
    }
}

void UnbindMatchingRoom(PvP_Room* room) {
    if (!room) return;
    MutexGuard guard(&g_handoff_mutex);
    for (std::map<CUser*, Binding>::iterator it = g_bindings.begin();
         it != g_bindings.end();) {
        if (it->second.kind == CONTAINER_PVP_ROOM &&
            it->second.container == room) {
            g_bindings.erase(it++);
        } else {
            ++it;
        }
    }
}

bool BeginChannelMove(CUser* user,
                      uint8_t matching_bound,
                      uint16_t matching_mode,
                      uint8_t field0,
                      uint32_t cohort_id,
                      uint32_t endpoint_key,
                      uint16_t endpoint_port_like,
                      int target_server_group,
                      int target_channel,
                      uint8_t cohort_slot,
                      uint8_t cohort_size) {
    Identity identity = MakeIdentity(user);
    CEnvironment* const environment = G_CEnvironment();
    const int source_channel = environment ? environment->get_channel_no() : -1;
    if (!IsMatchingModePolicyLoaded() || !IsLiveIdentity(user, identity) ||
        !environment || !IsChannelPair(matching_bound, matching_mode) ||
        !cohort_id || !endpoint_key || !endpoint_port_like ||
        !IsRuntimeCohortSize(cohort_size) ||
        cohort_slot >= cohort_size ||
        !IsChannelNumber(source_channel) || !IsChannelNumber(target_channel) ||
        target_server_group != identity.server_group ||
        environment->get_server_group() != identity.server_group) {
        return false;
    }

    bool retry_notification = false;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator prior = g_sessions.find(user);
        if (prior != g_sessions.end()) {
            retry_notification = prior->second.armed &&
                SameIdentity(prior->second.owner, identity) &&
                SameBeginRequest(prior->second, matching_bound,
                                 matching_mode, field0, cohort_id, endpoint_key,
                                 endpoint_port_like, target_server_group,
                                 target_channel, cohort_slot, cohort_size);
            if (!retry_notification) return false;
        } else {
            std::map<CUser*, Binding>::iterator bound =
                g_bindings.find(user);
            if (bound == g_bindings.end() || bound->second.detaching ||
                bound->second.matching_bound != matching_bound ||
                bound->second.matching_mode != matching_mode ||
                !SameIdentity(bound->second.owner, identity) ||
                !IsBindingLive(user, bound->second)) return false;

            Session session;
            session.origin = SESSION_SOURCE;
            session.token = NextNonZero(g_next_session_token);
            session.owner = identity;
            session.ticket.account_id = identity.account_id;
            session.ticket.charac_no = identity.charac_no;
            session.ticket.field0 = field0;
            session.ticket.field1 = cohort_id;
            session.ticket.endpoint_key = endpoint_key;
            session.ticket.endpoint_port_like = endpoint_port_like;
            session.ticket.source_server_group = identity.server_group;
            session.ticket.source_channel = source_channel;
            session.ticket.target_server_group = target_server_group;
            session.ticket.target_channel = target_channel;
            session.ticket.matching_bound = matching_bound;
            session.ticket.matching_mode = matching_mode;
            session.ticket.cohort_slot = cohort_slot;
            session.ticket.cohort_size = cohort_size;
            session.ticket.cohort_flags = 0u;
            session.ticket.operation =
                matching_channel_handoff_db::OPERATION_RESERVE;
            session.ticket.state =
                matching_channel_handoff_db::STATE_NONE;
            g_sessions[user] = session;
        }
    }
    if (retry_notification) return SendPendingNotification(user);

    Message reserve;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::const_iterator found =
            g_sessions.find(user);
        if (found == g_sessions.end()) return false;
        reserve = found->second.ticket;
    }
    if (QueueTracked(user, reserve)) return true;
    ResetSession(user);
    return false;
}

bool ConsumeClaimedHandoff(CUser* user, ClaimedHandoff& handoff) {
    std::memset(&handoff, 0, sizeof(handoff));
    MutexGuard guard(&g_handoff_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() ||
        found->second.origin != SESSION_TARGET || !found->second.claimed ||
        found->second.pending_operation ||
        !IsLiveIdentity(user, found->second.owner)) return false;
    const Message& ticket = found->second.ticket;
    handoff.handoff_id = ticket.handoff_id;
    handoff.cohort_id = ticket.field1;
    handoff.endpoint_key = ticket.endpoint_key;
    handoff.endpoint_port_like = ticket.endpoint_port_like;
    handoff.source_server_group = ticket.source_server_group;
    handoff.source_channel = ticket.source_channel;
    handoff.target_server_group = ticket.target_server_group;
    handoff.target_channel = ticket.target_channel;
    handoff.field0 = ticket.field0;
    handoff.matching_bound = ticket.matching_bound;
    handoff.matching_mode = ticket.matching_mode;
    handoff.cohort_slot = ticket.cohort_slot;
    handoff.cohort_size = ticket.cohort_size;
    handoff.cohort_flags = ticket.cohort_flags;
    const uint32_t cohort_id = ticket.field1;
    g_sessions.erase(found);
    ForgetCohortBarrierIfUnusedLocked(cohort_id);
    return true;
}

void SetClaimReadyCallback(ClaimReadyCallback callback) {
    MutexGuard guard(&g_handoff_mutex);
    g_claim_ready_callback = callback;
}

void ResetSession(CUser* user) {
    if (!user) return;
    bool cancel = false;
    {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end()) {
            g_bindings.erase(user);
            return;
        }
        cancel = found->second.origin == SESSION_SOURCE &&
                  found->second.ticket_owned &&
                  !found->second.armed &&
                 found->second.ticket.handoff_id != 0u;
    }
    if (cancel) {
        QueueCancelAndErase(user);
    } else {
        MutexGuard guard(&g_handoff_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        const uint32_t cohort_id = found == g_sessions.end()
                                       ? 0u : found->second.ticket.field1;
        if (found != g_sessions.end()) g_sessions.erase(found);
        ForgetCohortBarrierIfUnusedLocked(cohort_id);
    }
    MutexGuard guard(&g_handoff_mutex);
    g_bindings.erase(user);
}

}  // namespace matching_channel_handoff

namespace {

int EmptyRead(PacketBuf&, MSG_BASE&) { return 0; }
int EmptyProcess(CUser*, MSG_BASE&, ParamBase&) { return 0; }

}  // namespace

Dispatcher_MatchingDungeonExit::Dispatcher_MatchingDungeonExit()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_MatchingDungeonExit::~Dispatcher_MatchingDungeonExit() {}

int Dispatcher_MatchingDungeonExit::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    uint8_t matching_bound = 0u;
    uint16_t matching_mode = 0u;
    if (!user || packet.get_byte(matching_bound) != 1 ||
        packet.get_short(matching_mode) != 1) {
        return LineFunc(520, "MatchingDungeonExit0828::dispatch_sig", 0, 0);
    }

    uint8_t authority_bound = matching_bound;
    uint16_t authority_mode = matching_mode;
    const bool normalized =
        channel_zombie_matching_producer::NormalizeClientPair(
            user, matching_bound, matching_mode,
            authority_bound, authority_mode);
    cMyTrace field_trace("MatchingDungeonExit0828::dispatch_sig", 520, 0);
    field_trace("[ZOMBIE_MATCH][CMD520_FIELDS] wire_bound=%u wire_mode=%u authority_bound=%u authority_mode=%u normalized=%d state=%d",
                static_cast<unsigned int>(matching_bound),
                static_cast<unsigned int>(matching_mode),
                static_cast<unsigned int>(authority_bound),
                static_cast<unsigned int>(authority_mode),
                normalized ? 1 : 0,
                user->get_state());

    const channel_zombie_matching_producer::ProducerResult canceled =
        channel_zombie_matching_producer::Cancel(
            user, authority_bound, authority_mode);
    if (canceled != channel_zombie_matching_producer::PRODUCER_NOT_OWNED) {
        // CLIENT_IDA_EVIDENCE: CMD520 has no response handler.  The same
        // authority owns whole-cohort cancellation; never emit a fake ACK.
        return 0;
    }

    matching_channel_handoff::Binding binding;
    bool found_binding = false;
    {
        MutexGuard guard(&matching_channel_handoff::g_handoff_mutex);
        std::map<CUser*, matching_channel_handoff::Session>::iterator
            session = matching_channel_handoff::g_sessions.find(user);
        if (session != matching_channel_handoff::g_sessions.end() &&
            (session->second.pending_operation || session->second.armed)) {
            return 0;
        }
        std::map<CUser*, matching_channel_handoff::Binding>::iterator
            bound = matching_channel_handoff::g_bindings.find(user);
        if (bound != matching_channel_handoff::g_bindings.end() &&
            !bound->second.detaching &&
            bound->second.matching_bound == authority_bound &&
            bound->second.matching_mode == authority_mode &&
            matching_channel_handoff::IsBindingLive(
                user, bound->second)) {
            bound->second.detaching = true;
            binding = bound->second;
            found_binding = true;
        }
    }
    if (!found_binding) return 0;

    if (binding.kind == matching_channel_handoff::
                            CONTAINER_ZOMBIE_PARTY) {
        G_CGameManager()->CheckOutParty(user, false);
    } else if (binding.kind == matching_channel_handoff::
                                   CONTAINER_PVP_ROOM) {
        G_CGameManager()->CheckOutPvp(user, false);
    }
    const bool cleaned = !matching_channel_handoff::IsBindingLive(
        user, binding);
    {
        MutexGuard guard(&matching_channel_handoff::g_handoff_mutex);
        std::map<CUser*, matching_channel_handoff::Binding>::iterator
            bound = matching_channel_handoff::g_bindings.find(user);
        if (bound != matching_channel_handoff::g_bindings.end() &&
            bound->second.container == binding.container &&
            bound->second.container_index == binding.container_index) {
            if (cleaned) {
                matching_channel_handoff::g_bindings.erase(bound);
            } else {
                bound->second.detaching = false;
            }
        }
    }
    // CLIENT_IDA_EVIDENCE: no CMD520 response handler is registered.
    return 0;
}

int Dispatcher_MatchingDungeonExit::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    return EmptyRead(packet, message);
}

int Dispatcher_MatchingDungeonExit::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    return EmptyProcess(user, message, param);
}

Dispatcher_ChannelMoveSuccess::Dispatcher_ChannelMoveSuccess()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_ChannelMoveSuccess::~Dispatcher_ChannelMoveSuccess() {}

int Dispatcher_ChannelMoveSuccess::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    uint8_t field0 = 0u;
    uint32_t field4 = 0u;
    if (!user || packet.get_byte(field0) != 1 ||
        packet.get_int(field4) != 1) {
        return LineFunc(521, "ChannelMoveSuccess0828::dispatch_sig", 0, 0);
    }
    if (!channel_zombie_matching_producer::CanRegisterPackets()) return 0;
    matching_channel_handoff::Identity identity =
        matching_channel_handoff::MakeIdentity(user);
    CEnvironment* const environment = G_CEnvironment();
    const int target_channel = environment ? environment->get_channel_no() : -1;
    if (!field4 || !environment ||
        !matching_channel_handoff::IsLiveIdentity(user, identity) ||
        !matching_channel_handoff::IsChannelNumber(target_channel) ||
        environment->get_server_group() != identity.server_group ||
        field0 != static_cast<uint8_t>(identity.server_group)) {
        return 0;
    }

    matching_channel_handoff_db::Message claim;
    std::memset(&claim, 0, sizeof(claim));
    claim.handoff_id = field4;
    claim.field0 = field0;
    claim.field4 = field4;
    claim.target_server_group = identity.server_group;
    claim.target_channel = target_channel;
    claim.operation = matching_channel_handoff_db::OPERATION_CLAIM;
    claim.state = matching_channel_handoff_db::STATE_NONE;

    {
        MutexGuard guard(&matching_channel_handoff::g_handoff_mutex);
        if (matching_channel_handoff::g_sessions.find(user) !=
            matching_channel_handoff::g_sessions.end()) return 0;
        matching_channel_handoff::Session session;
        session.origin = matching_channel_handoff::SESSION_TARGET;
        session.token = matching_channel_handoff::NextNonZero(
            matching_channel_handoff::g_next_session_token);
        session.owner = identity;
        session.ticket = claim;
        matching_channel_handoff::g_sessions[user] = session;
    }
    if (!matching_channel_handoff::QueueTracked(user, claim)) {
        matching_channel_handoff::ResetSession(user);
    }
    // CLIENT_IDA_EVIDENCE: CMD521 has no response handler.  DB/Inter owns the
    // asynchronous one-shot result; never synthesize an ACK here.
    return 0;
}

int Dispatcher_ChannelMoveSuccess::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    return EmptyRead(packet, message);
}

int Dispatcher_ChannelMoveSuccess::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    return EmptyProcess(user, message, param);
}

InterMatchingChannelHandoff::InterMatchingChannelHandoff()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              matching_channel_handoff_db::kTaiwanInnerCode)) {}

InterMatchingChannelHandoff::~InterMatchingChannelHandoff() {}

int InterMatchingChannelHandoff::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(
            sizeof(matching_channel_handoff_db::Message))) return 0;
    const matching_channel_handoff_db::Message response =
        *reinterpret_cast<const matching_channel_handoff_db::Message*>(
            payload);
    const bool result_valid =
        response.result <= matching_channel_handoff_db::
                               RESULT_NOT_OWNER ||
        response.result == matching_channel_handoff_db::
                               RESULT_DATABASE_FAILURE;
    if (response.operation <
            matching_channel_handoff_db::OPERATION_RESERVE ||
        response.operation >
            matching_channel_handoff_db::OPERATION_COMPLETE_COHORT ||
        !result_valid ||
        response.state > matching_channel_handoff_db::STATE_EXPIRED ||
        response.reply_required > 1u ||
        (response.cohort_flags &
             ~matching_channel_handoff_db::COHORT_FLAG_ALL) != 0u ||
        (response.handoff_id != 0u &&
         response.field4 != response.handoff_id)) {
        return 0;
    }

    if (response.operation >=
            matching_channel_handoff_db::OPERATION_ROUTE_COHORT) {
        (void)channel_zombie_matching_authority::OnDatabaseResponse(
            user, response);
        return 0;
    }

    matching_channel_handoff::Session session;
    if (!matching_channel_handoff::CorrelateResponse(
            user, response, session)) return 0;

    const bool committed_claim =
        response.operation == matching_channel_handoff_db::OPERATION_CLAIM &&
        (response.result == matching_channel_handoff_db::RESULT_SUCCESS ||
         response.result == matching_channel_handoff_db::RESULT_REPLAY) &&
        response.state == matching_channel_handoff_db::STATE_CLAIMED &&
        response.source_cleaned;
    if ((response.result == matching_channel_handoff_db::RESULT_SUCCESS ||
         committed_claim) &&
        response.operation != matching_channel_handoff_db::OPERATION_CANCEL &&
        !matching_channel_handoff::IsCohortShapeValid(response)) {
        matching_channel_handoff::ResetSession(user);
        return 0;
    }
    if (response.result == matching_channel_handoff_db::RESULT_SUCCESS ||
        committed_claim)
        matching_channel_handoff::RememberCohortFlags(response);

    if (response.result == matching_channel_handoff_db::
                               RESULT_DATABASE_FAILURE) {
        if (matching_channel_handoff::RetryDatabaseFailure(
                user, response)) return 0;
        matching_channel_handoff::TraceRejected(
            "InterMatchingChannelHandoff0828::database", user,
            response.handoff_id);
        channel_zombie_matching_authority::AbortCohort(
            session.ticket.field1);
        matching_channel_handoff::QueueCancelAndErase(user);
        return 0;
    }

    if (response.operation ==
            matching_channel_handoff_db::OPERATION_RESERVE) {
        if (response.result != matching_channel_handoff_db::
                                   RESULT_SUCCESS ||
            response.state !=
                matching_channel_handoff_db::STATE_RESERVED ||
            response.source_cleaned) {
            matching_channel_handoff::TraceRejected(
                "InterMatchingChannelHandoff0828::reserve", user,
                response.handoff_id);
            channel_zombie_matching_authority::AbortCohort(
                session.ticket.field1);
            matching_channel_handoff::QueueCancelAndErase(user);
            return 0;
        }
        if (matching_channel_handoff::AdvanceReservedCohort(
                response.field1, response.cohort_size) ==
            matching_channel_handoff::BARRIER_FAILED) {
            channel_zombie_matching_authority::AbortCohort(
                response.field1);
        }
        return 0;
    }

    if (response.operation ==
            matching_channel_handoff_db::OPERATION_ARM) {
        if (response.result != matching_channel_handoff_db::
                                   RESULT_SUCCESS ||
            response.state !=
                matching_channel_handoff_db::STATE_ARMED ||
            !response.source_cleaned) {
            channel_zombie_matching_authority::AbortCohort(
                session.ticket.field1);
            matching_channel_handoff::QueueCancelAndErase(user);
            return 0;
        }
        {
            MutexGuard guard(&matching_channel_handoff::g_handoff_mutex);
            std::map<CUser*, matching_channel_handoff::Session>::iterator
                found = matching_channel_handoff::g_sessions.find(user);
            if (found == matching_channel_handoff::g_sessions.end())
                return 0;
            found->second.armed = true;
            found->second.ticket = response;
            found->second.database_retries = 0u;
        }
        if (matching_channel_handoff::NotifyArmedCohort(
                response.field1, response.cohort_size) ==
            matching_channel_handoff::BARRIER_FAILED) {
            channel_zombie_matching_authority::AbortCohort(
                response.field1);
        }
        return 0;
    }

    if (response.operation ==
            matching_channel_handoff_db::OPERATION_CLAIM) {
        if ((response.result == matching_channel_handoff_db::
                                    RESULT_SUCCESS ||
             response.result == matching_channel_handoff_db::RESULT_REPLAY) &&
            response.state ==
                matching_channel_handoff_db::STATE_CLAIMED &&
            response.source_cleaned) {
            matching_channel_handoff::ClaimReadyCallback callback = 0;
            {
                MutexGuard guard(
                    &matching_channel_handoff::g_handoff_mutex);
                std::map<CUser*, matching_channel_handoff::Session>::
                    iterator found =
                        matching_channel_handoff::g_sessions.find(user);
                if (found !=
                    matching_channel_handoff::g_sessions.end()) {
                    found->second.claimed = true;
                    found->second.ticket = response;
                    found->second.database_retries = 0u;
                    callback = matching_channel_handoff::
                        g_claim_ready_callback;
                }
            }
            if (callback) callback(user);
        } else {
            matching_channel_handoff::TraceRejected(
                "InterMatchingChannelHandoff0828::claim", user,
                response.handoff_id);
            channel_zombie_matching_authority::AbortCohort(
                response.field1);
            matching_channel_handoff::ResetSession(user);
        }
        return 0;
    }

    matching_channel_handoff::ResetSession(user);
    return 0;
}
