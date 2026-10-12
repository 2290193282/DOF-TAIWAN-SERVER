#ifndef OLDSRV_OLD_GAME_PVP_MATCH_CHANNEL_ZOMBIE_MATCHING_AUTHORITY_H_
#define OLDSRV_OLD_GAME_PVP_MATCH_CHANNEL_ZOMBIE_MATCHING_AUTHORITY_H_

#include <stdint.h>

class CUser;
class CParty;
class PvP_Room;

namespace matching_channel_handoff_db {
struct Message;
}

namespace channel_zombie_matching_authority {

// Installs the single process-lifetime 140828 project-owned AuthorityV2.
// Official matching/PVF facts remain validated by the producer; route,
// cohort_id, and target reconstruction are explicitly project-owned.
bool Initialize();
void Shutdown();

// Taiwan inner-25 Inter bridge for cohort control operations 5..8.  Every
// response is accepted only after immutable leader identity plus local
// session_token/request_token correlation succeeds.  Source op5/op6 use the
// original authority token; target op6/op7/op8 additionally require the
// durable cohort-slot-0 CLAIMED ticket proof carried in handoff_id/field4.
bool OnDatabaseResponse(
    CUser* user,
    const matching_channel_handoff_db::Message& response);

// Cheap main-thread maintenance hook.  A route BUSY/DB failure restores the
// intact queue and this hook retries after a bounded backoff.  It also owns
// every in-memory source/target deadline, so no missing DB response or partial
// target claim can pin a room/cohort forever.
void Tick();

// Deterministic source-level test hook.  Production code calls Tick(); focused
// tests may advance directly past a known deadline without sleeping.
void TickAtForTest(uint32_t now_seconds);

// Per-room lifecycle bridge.  Source rooms abort until ALL_ARMED; an armed
// source reset is the expected channel transfer.  Active target reset queues
// OPERATION_COMPLETE_COHORT exactly once.
void OnRoomReset(PvP_Room* room);

// CParty lifecycle gate for the per-seat Zombie map context.  When true, the
// whole cohort was synchronously detached and cleaned; the caller must return
// immediately because its original CParty operation has already been
// completed by the owner cleanup path.
bool AbortMapOwnerBeforeLifecycle(CParty* party, CUser* actor);

// Owner-first CMD040 bridge for the hidden per-seat 3101 map adapter.  A
// mapped owner is always consumed here (including a rejected/replayed
// request), so it can never fall through to the ordinary singleton loading
// path.  The two client checksums retain the original anti-hack accounting.
bool ConsumeMapOwnerFinishLoading(CParty* party,
                                  CUser* user,
                                  int32_t inventory_checksum,
                                  int32_t skill_checksum);

// The singleton CParty also owns an ordinary ~60s forced-load timer.  Once a
// Zombie load epoch exists, that timer is non-authoritative and must be
// consumed so the authority can make the exact 6 / timeout-5 decision.
bool ConsumeMapOwnerLoadTimeout(CParty* party);

// Official CMD295 -> NOTI278 scene-load barrier for the Type19 Zombie cohort.
// Retained 28/29 is published first because it activates the 0828 client
// controller that emits CMD295.  Every immutable six-seat reporter is then
// announced to the whole cohort exactly once.  Returning true means the
// managed room consumed the request, including rejected, duplicate and late
// reports.
bool ConsumeCompleteLoad(CUser* user, PvP_Room* room);

// Official CMD296 -> NOTI279 P2P barrier for the Type19 Zombie cohort.  Map
// bootstrap is already visible at this point; NOTI279 only releases the P2P
// scene after all six reporters have completed CMD295/NOTI278.
// Returning true means the room is authority-owned and the request was
// consumed, including rejected, duplicate and replayed reports; such packets
// must never fall through to the ordinary IMatch no-op implementation.
bool ConsumeConnectP2P(CUser* user,
                       PvP_Room* room,
                       const char results[8],
                       int count);

// Called by the handoff owner at durable cohort barriers.
bool OnSourceCohortDetachReady(uint32_t cohort_id);
bool OnSourceCohortArmed(uint32_t cohort_id);
void OnSourceCohortReleased(uint32_t cohort_id);
void AbortCohort(uint32_t cohort_id);

}  // namespace channel_zombie_matching_authority

#endif  // OLDSRV_OLD_GAME_PVP_MATCH_CHANNEL_ZOMBIE_MATCHING_AUTHORITY_H_
