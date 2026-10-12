#ifndef OLDSRV_OLD_GAME_PVP_MATCH_CHANNEL_ZOMBIE_MATCHING_PRODUCER_H_
#define OLDSRV_OLD_GAME_PVP_MATCH_CHANNEL_ZOMBIE_MATCHING_PRODUCER_H_

#include "old_game/pvp/match/matching_channel_handoff.h"
#include "old_game/pvp/match/zombie_matching_runtime_policy.h"

#include <stddef.h>
#include <stdint.h>

class CUser;
class PvP_Room;

namespace channel_zombie_matching_producer {

enum {
    kAuthorityAbiVersion = 2,
    kChannelMatchingBound = 1,
    kZombieMatchingMode = 15,
    kMinimumMembers =
        zombie_matching_runtime_policy::kOfficialMatchingMembers,
    kMaximumMembers =
        zombie_matching_runtime_policy::kOfficialMatchingMembers,
    kRoomSeats = 8,
    kZombieChannelType = 19,
};

// Evidence bits deliberately distinguish inspected wire/PVF facts from the
// explicitly 140828 project-owned route/cohort contract.  A runtime caller
// cannot widen that reviewed contract by installing arbitrary callbacks or
// self-attesting unknown evidence bits.
enum EvidenceBit {
    EVIDENCE_EXACT_PAIR = 1u << 0,
    EVIDENCE_MATCHINGMODE_PVF = 1u << 1,
    EVIDENCE_ZOMBIEMODE_PVF = 1u << 2,
    EVIDENCE_EIGHT_SEAT_ROOM = 1u << 3,
    EVIDENCE_SOURCE_CONTAINER = 1u << 4,
    EVIDENCE_PROJECT_ROUTE_SELECTOR = 1u << 5,
    EVIDENCE_FIELD0_TARGET_GROUP = 1u << 6,
    // NOTI606.field1 is a 140828 project-owned durable cohort_id.  This bit
    // deliberately does not claim a surviving official field1 semantic.
    EVIDENCE_PROJECT_COHORT_ID = 1u << 7,
    EVIDENCE_ENDPOINT_GAME_CHANNEL = 1u << 8,
    EVIDENCE_FIELD4_CLAIM_KEY = 1u << 9,
    EVIDENCE_PROJECT_TARGET_COHORT_REBUILD = 1u << 10,
};

enum ProducerResult {
    PRODUCER_NOT_OWNED = 0,
    PRODUCER_ACCEPTED = 1,
    PRODUCER_REJECTED = 2,
    PRODUCER_UNAVAILABLE = 3,
};

// A selected route is a DB-thread snapshot of one live game_channel row.
// target_channel is its logical gc_ch_group, not gc_no or display gc_channel.
// cohort_id is the 140828 project-owned NOTI606.field1 value allocated by the
// durable cohort transaction; it is never a channel number or room index.
struct TargetRoute {
    TargetRoute();

    int32_t target_server_group;
    int32_t target_channel;
    uint8_t field0;
    uint8_t reserved_15[3];
    uint32_t cohort_id;
    uint32_t endpoint_key;
    uint16_t endpoint_port;
    uint16_t reserved_22;
};

// The queue/cohort/route implementation is intentionally supplied as one
// authority.  Splitting these callbacks among unrelated owners would permit a
// queue decision, route snapshot and target-room claim from different epochs.
struct AuthorityV2 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t evidence_mask;
    uint32_t reserved;

    bool (*enqueue)(CUser* user, uint8_t bound, uint16_t mode);
    // Completes an accepted enqueue only after CMD518's success response has
    // entered the user's ordered transport.  The split is required because
    // the client handler clears the ready-user list before later NOTI595
    // records rebuild it.
    bool (*commit_enqueue)(CUser* user, uint8_t bound, uint16_t mode);
    // Rolls back the exact accepted enqueue when CMD518's success response
    // cannot enter the user's transport.  No staged NOTI595 may escape.
    bool (*abort_enqueue)(CUser* user, uint8_t bound, uint16_t mode);
    bool (*cancel)(CUser* user, uint8_t bound, uint16_t mode);
    bool (*select_target_route)(PvP_Room* source_room,
                                CUser* const* members,
                                size_t member_count,
                                TargetRoute* route);
    void (*abort_source_cohort)(PvP_Room* source_room);
    bool (*accept_claim)(CUser* user,
                         const matching_channel_handoff::
                             ClaimedHandoff& handoff);
    bool (*authorize_target_room)(PvP_Room* target_room);
};

struct Readiness {
    uint32_t repository_evidence_mask;
    uint32_t authority_evidence_mask;
    uint32_t missing_evidence_mask;
    bool matching_policy_loaded;
    bool zombie_policy_loaded;
    bool authority_installed;
    bool registerable;
};

// Loads both official policies.  The project-owned authority is installed by
// channel_zombie_matching_authority::Initialize; packet registration is
// allowed only after both steps succeed.
bool Initialize();

bool InstallAuthority(const AuthorityV2& authority);
void RemoveAuthority();
Readiness GetReadiness();
bool CanRegisterPackets();

// The 140828 Zombie client presents the channel mode as wire pair (0,0),
// while the PVF/authority identity remains (1,15).  CMD518, CMD519 and CMD520
// must all cross the same channel-scoped bridge or the completed-room Leave
// request is silently routed to the unrelated integrated-mode owner.
bool NormalizeClientPair(CUser* user,
                         uint8_t wire_bound,
                         uint16_t wire_mode,
                         uint8_t& authority_bound,
                         uint16_t& authority_mode);

// CMD518/519/520 routing entrypoints.  Only authority pair (1,15) is owned
// here; integrated (0,0..3) remains owned by integrated_matching_runtime.
ProducerResult Request(CUser* user, uint8_t bound, uint16_t mode);
// Must be called only after the accepted CMD518 success response is sent.
// It publishes the staged NOTI595 updates and may then launch an exact-six
// source cohort, preserving ACK -> NOTI595 -> NOTI458 on the client wire.
bool CommitRequest(CUser* user, uint8_t bound, uint16_t mode);
// Terminates an accepted request whose CMD518 response was not queued.  The
// authority discards staged NOTI595 records and cancels the tentative group or
// source cohort before the pooled CUser can be reused.
bool AbortRequest(CUser* user, uint8_t bound, uint16_t mode);
ProducerResult Cancel(CUser* user, uint8_t bound, uint16_t mode);

// Called by the installed queue owner after it has materialized one complete
// Exact-six source cohort in the fixed eight-seat PvP_Room.  The boundary binds
// that exact container before selecting a route or starting durable handoffs.
bool PublishMatchedSourceRoom(PvP_Room* source_room);

// Authorizes a materialized matching room.  The target authority creates the
// Zombie round only after its six-seat load barrier resolves as six or as the
// official five-player timeout fallback.
bool ActivateTargetRoom(PvP_Room* target_room);

// Disconnect/reset bridge.  The installed authority distinguishes an
// expected armed channel handoff from a real queue/cohort cancellation.
void ResetUser(CUser* user);

}  // namespace channel_zombie_matching_producer

#endif  // OLDSRV_OLD_GAME_PVP_MATCH_CHANNEL_ZOMBIE_MATCHING_PRODUCER_H_
