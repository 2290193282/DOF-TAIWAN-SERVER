#include "old_game/pvp/match/channel_zombie_matching_producer.h"

#include "core/config/env/environment.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/log_manager.h"
#include "old_data/db/pvp_party/db_matching_channel_handoff.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/world/region/game_world.h"

#include <cstring>
#include <vector>

namespace channel_zombie_matching_producer {
namespace {

const uint32_t kAllEvidence =
    EVIDENCE_EXACT_PAIR |
    EVIDENCE_MATCHINGMODE_PVF |
    EVIDENCE_ZOMBIEMODE_PVF |
    EVIDENCE_EIGHT_SEAT_ROOM |
    EVIDENCE_SOURCE_CONTAINER |
    EVIDENCE_PROJECT_ROUTE_SELECTOR |
    EVIDENCE_FIELD0_TARGET_GROUP |
    EVIDENCE_PROJECT_COHORT_ID |
    EVIDENCE_ENDPOINT_GAME_CHANNEL |
    EVIDENCE_FIELD4_CLAIM_KEY |
    EVIDENCE_PROJECT_TARGET_COHORT_REBUILD;

// The wire/PVF facts are evidence-backed.  Route selection, field1 cohort_id,
// and target reconstruction are explicitly 140828 project-owned contracts;
// their names keep them separate from claims about the surviving official
// producer.
const uint32_t kRepositoryEvidence =
    EVIDENCE_EXACT_PAIR |
    EVIDENCE_MATCHINGMODE_PVF |
    EVIDENCE_ZOMBIEMODE_PVF |
    EVIDENCE_EIGHT_SEAT_ROOM |
    EVIDENCE_SOURCE_CONTAINER |
    EVIDENCE_PROJECT_ROUTE_SELECTOR |
    EVIDENCE_FIELD0_TARGET_GROUP |
    EVIDENCE_PROJECT_COHORT_ID |
    EVIDENCE_ENDPOINT_GAME_CHANNEL |
    EVIDENCE_FIELD4_CLAIM_KEY |
    EVIDENCE_PROJECT_TARGET_COHORT_REBUILD;

const uint32_t kAuthorityRequiredEvidence =
    EVIDENCE_PROJECT_ROUTE_SELECTOR |
    EVIDENCE_PROJECT_COHORT_ID |
    EVIDENCE_PROJECT_TARGET_COHORT_REBUILD;

Mutex g_authority_mutex;
AuthorityV2 g_authority;
bool g_authority_installed = false;

bool IsExactPair(uint8_t bound, uint16_t mode) {
    return bound == kChannelMatchingBound && mode == kZombieMatchingMode;
}

bool IsServerGroup(int value) {
    return value >= 0 && value < 18;
}

bool IsChannel(int value) {
    return value > 0 && value <= 65535;
}

bool IsRuntimeCohortSize(size_t value) {
    return zombie_matching_runtime_policy::IsRuntimeCohortSize(value);
}

bool HasAllCallbacks(const AuthorityV2& authority) {
    return authority.enqueue && authority.commit_enqueue &&
           authority.abort_enqueue && authority.cancel &&
           authority.select_target_route &&
           authority.abort_source_cohort && authority.accept_claim &&
           authority.authorize_target_room;
}

bool SnapshotAuthority(AuthorityV2& authority) {
    MutexGuard guard(&g_authority_mutex);
    if (!g_authority_installed) return false;
    authority = g_authority;
    return true;
}

bool PoliciesLoaded() {
    return matching_channel_handoff::IsMatchingModePolicyLoaded() &&
           zombie_mode::IsConfigLoaded();
}

bool IsIdentityLive(CUser* user) {
    return user && user->get_state() == 3 && user->getCurCharacR() &&
           !user->IsCurCharacGhost() && user->get_acc_id() > 0 &&
           user->getCurCharacNo() > 0 && user->get_unique_id() > 0 &&
           user->get_unique_id() < 0xffff;
}

bool CaptureRoom(PvP_Room* room,
                 std::vector<CUser*>& members,
                 int& source_server_group,
                 int& source_channel) {
    members.clear();
    source_server_group = -1;
    source_channel = -1;
    if (!room || room->get_state() != 1 ||
        !IsRuntimeCohortSize(
            static_cast<size_t>(room->get_player_count()))) {
        return false;
    }

    const int room_index = room->get_index();
    const int player_count = room->get_player_count();
    for (int seat = 0; seat < kRoomSeats; ++seat) {
        CUser* const user = room->get_user_seat(seat);
        if (seat >= player_count) {
            if (user) return false;
            continue;
        }
        if (!user || room->get_team(seat) != (seat < 3 ? 1 : 2))
            return false;
        if (room->IsPvpObserver(seat) || !IsIdentityLive(user) ||
            user->GetPvpIndex() != room_index) {
            return false;
        }
        if (source_server_group < 0) {
            source_server_group = user->GetServerGroup();
        } else if (source_server_group != user->GetServerGroup()) {
            return false;
        }
        members.push_back(user);
    }

    if (members.size() != static_cast<size_t>(room->get_player_count()) ||
        !IsRuntimeCohortSize(members.size()) ||
        !IsServerGroup(source_server_group)) {
        return false;
    }

    CEnvironment* const environment = G_CEnvironment();
    if (!environment ||
        environment->get_server_group() != source_server_group ||
        !IsChannel(environment->get_channel_no())) {
        return false;
    }
    source_channel = environment->get_channel_no();
    return true;
}

bool ValidateRoute(const TargetRoute& route,
                   int source_server_group,
                   size_t member_count) {
    if (route.reserved_15[0] || route.reserved_15[1] ||
        route.reserved_15[2] || route.reserved_22 ||
        !IsServerGroup(route.target_server_group) ||
        route.target_server_group != source_server_group ||
        !IsChannel(route.target_channel) ||
        !IsRuntimeCohortSize(member_count) ||
        route.field0 !=
            static_cast<uint8_t>(route.target_server_group) ||
        route.endpoint_key == 0u || route.endpoint_key == 0xffffffffu ||
        route.endpoint_port == 0u) {
        return false;
    }

    // 140828 project-owned contract: NOTI606.field1 is the durable cohort id,
    // never target_channel, gc_no, or an in-memory PvP_Room index.
    return route.cohort_id != 0u;
}

bool ValidateClaim(const matching_channel_handoff::
                       ClaimedHandoff& handoff) {
    return handoff.handoff_id != 0u && handoff.cohort_id != 0u &&
           IsRuntimeCohortSize(handoff.cohort_size) &&
           handoff.cohort_slot < handoff.cohort_size &&
           (handoff.cohort_flags &
                ~matching_channel_handoff_db::COHORT_FLAG_ALL) == 0u &&
           IsExactPair(handoff.matching_bound, handoff.matching_mode) &&
           IsServerGroup(handoff.source_server_group) &&
           IsChannel(handoff.source_channel) &&
           IsServerGroup(handoff.target_server_group) &&
           IsChannel(handoff.target_channel) &&
           handoff.source_server_group == handoff.target_server_group &&
           handoff.field0 ==
               static_cast<uint8_t>(handoff.target_server_group) &&
           handoff.endpoint_key != 0u &&
           handoff.endpoint_key != 0xffffffffu &&
           handoff.endpoint_port_like > 0u &&
           handoff.endpoint_port_like <= 65535u;
}

void OnClaimReady(CUser* user) {
    if (!user || !CanRegisterPackets()) return;

    matching_channel_handoff::ClaimedHandoff handoff;
    if (!matching_channel_handoff::ConsumeClaimedHandoff(
            user, handoff) ||
        !ValidateClaim(handoff)) {
        matching_channel_handoff::ResetSession(user);
        return;
    }

    AuthorityV2 authority;
    if (!SnapshotAuthority(authority) ||
        !authority.accept_claim(user, handoff)) {
        matching_channel_handoff::ResetSession(user);
    }
}

}  // namespace

TargetRoute::TargetRoute()
    : target_server_group(-1),
      target_channel(-1),
      field0(0),
      reserved_15(),
      cohort_id(0),
      endpoint_key(0),
      endpoint_port(0),
      reserved_22(0) {}

bool Initialize() {
    if (!matching_channel_handoff::IsMatchingModePolicyLoaded()) {
        (void)matching_channel_handoff::LoadMatchingModePolicy();
    }
    if (!zombie_mode::IsConfigLoaded()) {
        (void)zombie_mode::LoadConfig();
    }
    return PoliciesLoaded();
}

bool InstallAuthority(const AuthorityV2& authority) {
    // Runtime self-attestation cannot substitute for the reviewed repository
    // contract.  The project-owned bits are named as such and must be present
    // on both sides before the packet cohort can register.
    if ((kRepositoryEvidence & kAllEvidence) != kAllEvidence ||
        authority.abi_version != kAuthorityAbiVersion ||
        authority.struct_size != sizeof(AuthorityV2) ||
        authority.reserved != 0u ||
        (authority.evidence_mask & ~kAllEvidence) != 0u ||
        (authority.evidence_mask & kAuthorityRequiredEvidence) !=
            kAuthorityRequiredEvidence ||
        !HasAllCallbacks(authority)) {
        return false;
    }

    {
        MutexGuard guard(&g_authority_mutex);
        if (g_authority_installed) return false;
        g_authority = authority;
        g_authority_installed = true;
    }
    matching_channel_handoff::SetClaimReadyCallback(&OnClaimReady);
    return true;
}

void RemoveAuthority() {
    matching_channel_handoff::SetClaimReadyCallback(0);
    MutexGuard guard(&g_authority_mutex);
    std::memset(&g_authority, 0, sizeof(g_authority));
    g_authority_installed = false;
}

Readiness GetReadiness() {
    Readiness readiness;
    readiness.repository_evidence_mask = kRepositoryEvidence;
    readiness.authority_evidence_mask = 0u;
    readiness.matching_policy_loaded =
        matching_channel_handoff::IsMatchingModePolicyLoaded();
    readiness.zombie_policy_loaded = zombie_mode::IsConfigLoaded();
    readiness.authority_installed = false;
    {
        MutexGuard guard(&g_authority_mutex);
        readiness.authority_installed = g_authority_installed;
        if (g_authority_installed) {
            readiness.authority_evidence_mask = g_authority.evidence_mask;
        }
    }
    readiness.missing_evidence_mask =
        kAllEvidence & ~readiness.repository_evidence_mask;
    readiness.registerable = readiness.matching_policy_loaded &&
                             readiness.zombie_policy_loaded &&
                             readiness.authority_installed &&
                             readiness.missing_evidence_mask == 0u;
    return readiness;
}

bool CanRegisterPackets() {
    return GetReadiness().registerable;
}

bool NormalizeClientPair(CUser* user,
                         uint8_t wire_bound,
                         uint16_t wire_mode,
                         uint8_t& authority_bound,
                         uint16_t& authority_mode) {
    authority_bound = wire_bound;
    authority_mode = wire_mode;
    GameWorld* const world = G_GameWorld();
    if (!user || !world || world->GetChannelType() != kZombieChannelType ||
        wire_bound != 0u || wire_mode != 0u) {
        return false;
    }
    authority_bound = static_cast<uint8_t>(kChannelMatchingBound);
    authority_mode = static_cast<uint16_t>(kZombieMatchingMode);
    return true;
}

ProducerResult Request(CUser* user, uint8_t bound, uint16_t mode) {
    if (!IsExactPair(bound, mode)) return PRODUCER_NOT_OWNED;
    const bool identity_live = IsIdentityLive(user);
    const bool policies_loaded = PoliciesLoaded();
    const Readiness readiness = GetReadiness();
    if (!identity_live || !policies_loaded) {
        LogManager::logFormat(
            1,
            "channel_zombie_matching_producer.cpp",
            "channel_zombie_matching_producer::Request",
            __LINE__,
            "[ZOMBIE_MATCH][REQUEST_GATE] stage=identity identity=%d state=%d matching_policy=%d zombie_policy=%d authority=%d registerable=%d missing=0x%X",
            identity_live ? 1 : 0,
            user ? user->get_state() : -1,
            readiness.matching_policy_loaded ? 1 : 0,
            readiness.zombie_policy_loaded ? 1 : 0,
            readiness.authority_installed ? 1 : 0,
            readiness.registerable ? 1 : 0,
            readiness.missing_evidence_mask);
        return PRODUCER_REJECTED;
    }
    if (!readiness.registerable) {
        LogManager::logFormat(
            1,
            "channel_zombie_matching_producer.cpp",
            "channel_zombie_matching_producer::Request",
            __LINE__,
            "[ZOMBIE_MATCH][REQUEST_GATE] stage=readiness identity=1 state=%d matching_policy=%d zombie_policy=%d authority=%d registerable=0 missing=0x%X repo=0x%X authority_evidence=0x%X",
            user ? user->get_state() : -1,
            readiness.matching_policy_loaded ? 1 : 0,
            readiness.zombie_policy_loaded ? 1 : 0,
            readiness.authority_installed ? 1 : 0,
            readiness.missing_evidence_mask,
            readiness.repository_evidence_mask,
            readiness.authority_evidence_mask);
        return PRODUCER_UNAVAILABLE;
    }

    AuthorityV2 authority;
    const bool authority_snapshot = SnapshotAuthority(authority);
    const bool accepted = authority_snapshot &&
                          authority.enqueue(user, bound, mode);
    LogManager::logFormat(
        1,
        "channel_zombie_matching_producer.cpp",
        "channel_zombie_matching_producer::Request",
        __LINE__,
        "[ZOMBIE_MATCH][REQUEST_GATE] stage=authority identity=1 state=%d snapshot=%d accepted=%d",
        user ? user->get_state() : -1,
        authority_snapshot ? 1 : 0,
        accepted ? 1 : 0);
    if (!authority_snapshot) return PRODUCER_UNAVAILABLE;
    return accepted ? PRODUCER_ACCEPTED : PRODUCER_REJECTED;
}

bool CommitRequest(CUser* user, uint8_t bound, uint16_t mode) {
    if (!IsExactPair(bound, mode)) return false;
    AuthorityV2 authority;
    if (!SnapshotAuthority(authority) || !authority.commit_enqueue) {
        return false;
    }
    const bool committed = authority.commit_enqueue(user, bound, mode);
    LogManager::logFormat(
        1,
        "channel_zombie_matching_producer.cpp",
        "channel_zombie_matching_producer::CommitRequest",
        __LINE__,
        "[ZOMBIE_MATCH][REQUEST_COMMIT] state=%d committed=%d",
        user ? user->get_state() : -1,
        committed ? 1 : 0);
    return committed;
}

bool AbortRequest(CUser* user, uint8_t bound, uint16_t mode) {
    if (!IsExactPair(bound, mode)) return false;
    AuthorityV2 authority;
    if (!SnapshotAuthority(authority) || !authority.abort_enqueue) {
        return false;
    }
    const bool aborted = authority.abort_enqueue(user, bound, mode);
    LogManager::logFormat(
        1,
        "channel_zombie_matching_producer.cpp",
        "channel_zombie_matching_producer::AbortRequest",
        __LINE__,
        "[ZOMBIE_MATCH][REQUEST_ABORT] state=%d aborted=%d",
        user ? user->get_state() : -1,
        aborted ? 1 : 0);
    return aborted;
}

ProducerResult Cancel(CUser* user, uint8_t bound, uint16_t mode) {
    if (!IsExactPair(bound, mode)) return PRODUCER_NOT_OWNED;
    if (!user || !PoliciesLoaded()) return PRODUCER_REJECTED;
    if (!CanRegisterPackets()) return PRODUCER_UNAVAILABLE;

    AuthorityV2 authority;
    if (!SnapshotAuthority(authority)) return PRODUCER_UNAVAILABLE;
    return authority.cancel(user, bound, mode)
               ? PRODUCER_ACCEPTED : PRODUCER_REJECTED;
}

bool PublishMatchedSourceRoom(PvP_Room* source_room) {
    if (!CanRegisterPackets()) return false;

    std::vector<CUser*> members;
    int source_server_group = -1;
    int source_channel = -1;
    if (!CaptureRoom(source_room, members, source_server_group,
                     source_channel) ||
        !matching_channel_handoff::BindMatchingRoom(
            source_room, kChannelMatchingBound, kZombieMatchingMode)) {
        return false;
    }

    AuthorityV2 authority;
    std::memset(&authority, 0, sizeof(authority));
    TargetRoute route;
    if (!SnapshotAuthority(authority) ||
        !authority.select_target_route(source_room, &members[0],
                                       members.size(), &route) ||
        !ValidateRoute(route, source_server_group, members.size())) {
        matching_channel_handoff::UnbindMatchingRoom(source_room);
        if (authority.abort_source_cohort) {
            authority.abort_source_cohort(source_room);
        }
        return false;
    }

    size_t started = 0u;
    for (; started < members.size(); ++started) {
        if (!matching_channel_handoff::BeginChannelMove(
                members[started], kChannelMatchingBound,
                kZombieMatchingMode, route.field0, route.cohort_id,
                route.endpoint_key, route.endpoint_port,
                route.target_server_group, route.target_channel,
                static_cast<uint8_t>(started),
                static_cast<uint8_t>(members.size()))) {
            break;
        }
    }
    if (started != members.size()) {
        for (size_t index = 0u; index < started; ++index) {
            matching_channel_handoff::ResetSession(members[index]);
        }
        matching_channel_handoff::UnbindMatchingRoom(source_room);
        authority.abort_source_cohort(source_room);
        return false;
    }
    return true;
}

bool ActivateTargetRoom(PvP_Room* target_room) {
    if (!CanRegisterPackets()) return false;

    std::vector<CUser*> members;
    int server_group = -1;
    int channel = -1;
    if (!CaptureRoom(target_room, members, server_group, channel)) {
        return false;
    }

    AuthorityV2 authority;
    if (!SnapshotAuthority(authority) ||
        !authority.authorize_target_room(target_room)) {
        return false;
    }
    // Authorization only.  The target authority owns the load epoch and may
    // still degrade one timed-out six-seat load to the official five-player
    // play-mode row.  Creating the Zombie result/role sidecar here would
    // freeze the wrong member count before that barrier is resolved.
    return true;
}

void ResetUser(CUser* user) {
    if (!user) return;
    AuthorityV2 authority;
    if (SnapshotAuthority(authority)) {
        (void)authority.cancel(user, kChannelMatchingBound,
                               kZombieMatchingMode);
    }
    matching_channel_handoff::ResetSession(user);
    zombie_mode::ResetUserSession(user);
}

}  // namespace channel_zombie_matching_producer
