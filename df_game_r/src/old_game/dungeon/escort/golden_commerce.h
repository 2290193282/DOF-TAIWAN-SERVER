#ifndef OLDSRV_OLD_GAME_DUNGEON_ESCORT_GOLDEN_COMMERCE_H_
#define OLDSRV_OLD_GAME_DUNGEON_ESCORT_GOLDEN_COMMERCE_H_

#include <stdint.h>

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/concurrent/timer/timer_sig.h"
#include "old_game/dungeon/escort/golden_commerce_types.h"

class CParty;
class CUser;
class PacketBuf;

namespace golden_commerce {

enum {
    kOrdinaryPartySeatCount = 4,
    // The original 175-entry TimerDispatcher leaves message 159 empty
    // (158 is QuickParty matching and 160 is AssaultGhost).  This sidecar
    // owns that exact vacant slot for the Golden-Commerce selection deadline;
    // expiry fills missing free choices and commits the shared CMD547 start.
    kSelectionDeadlineTimerMessage = 159,
};

class TimerSelectionDeadline : public TimerSig {
public:
    TimerSelectionDeadline();
    virtual ~TimerSelectionDeadline();
    virtual int dispatch_sig(int party_index,
                             int run_token_hi,
                             unsigned int run_token_lo);
};

// Read-only diagnostic snapshot.  The four seats are the ordinary CParty
// seats; member_unique_ids are the u16 identities consumed by NOTI479.
struct PartySnapshot {
    PartySnapshot();

    int party_index;
    int dungeon_index;
    bool opened;
    bool started;
    bool completed;
    bool terminal_reached;
    bool terminal_room_bound;
    bool clear_seen;
    uint8_t terminal_grid_x;
    uint8_t terminal_grid_y;
    uint16_t cart_unique_id;
    uint32_t cart_monster_id;
    uint32_t cart_max_hp;
    uint32_t cart_remaining_hp;
    uint8_t authoritative_percent;
    uint16_t member_unique_ids[kOrdinaryPartySeatCount];
    int selected_item_ids[kOrdinaryPartySeatCount];
};

// Called after the current map has instantiated its official [map move
// monster].  The first call captures the immutable roster/cart identity and
// emits the exact empty NOTI480.  Repeated map-load calls rebind only the new
// cart object and preserve the server-owned remaining-HP ratio.
bool BeginSelection(CParty* party);
bool OnMapLoaded(CParty* party);

// NOTI476 is a deferred client record: its handler only queues the cart, and
// NOTI30 consumes that queue while completing the scene transition.  Call
// this after the client has handled NOTI28/29 but immediately before the
// server sends NOTI30 for every room.
bool SendCartBeforeFinishLoading(CParty* party);

// CMD541 integration: call AuthorizeCartMovement after the ordinary move
// request has resolved its current-map entity but before commit/broadcast.
// It returns true for every non-Golden entity, so an unconditional shared
// call cannot disturb ordinary dungeon movement.  The committed hook is
// idempotent.  The committed hook receives the already server-validated
// directional target and latches the official terminal grid before CMD548.
bool AuthorizeCartMovement(CParty* party, uint16_t entity_unique_id);
void OnCartMovementCommitted(CParty* party,
                             uint16_t entity_unique_id,
                             uint8_t grid_x,
                             uint8_t grid_y,
                             uint8_t action);

// The SDC owner calls this only after it has independently validated and
// recomputed the damage from server combat state.  request_fingerprint is a
// stable nonzero digest of the exact validated request and prevents replay.
bool ObserveValidatedCartDamage(CParty* party,
                                uint16_t entity_unique_id,
                                uint32_t authoritative_damage,
                                uint64_t request_fingerprint);

// Returns true for all three official Golden-Commerce dungeons so the shared
// result owner cannot fall back to an ordinary reward path.  CMD49 is an
// ordering-independent clear latch/fallback: it must not complete a run before
// the official one-byte CMD548 cart-health report has been captured.
bool OnPartyDungeonResult(CParty* party,
                          int dungeon_index,
                          bool cleared);

// Official GoldenCommerce.info [exp rate] applies only to current members of
// a started, unsettled Golden-Commerce session.
unsigned int ScaleMonsterExperience(CParty* party,
                                    CUser* user,
                                    unsigned int experience);

// Presentation-only total of the already committed monster awards.  Never
// grants experience; the result screen must not pay these amounts twice.
void RecordMonsterExperienceAwarded(CParty* party, CUser* user,
                                    unsigned int experience);

// Lifecycle/DB retry boundaries.  `now_epoch` is seconds.  PartySession owns
// only live run/UI state; the reward outbox is detached and survives user
// disconnect, ordinary reset, CParty::init/pool reuse and session expiry until
// the DB response acknowledges delivery.
void OnPeriodicTick(uint32_t now_epoch);
void OnUserDisconnect(CUser* user);
// Reset/release may discard the live PartySession only.  Neither operation is
// an entitlement cancellation boundary and neither may delete pending outbox
// claims.
void ResetParty(CParty* party);
void ReleaseParty(CParty* party);

bool CopySnapshot(CParty* party, PartySnapshot& snapshot);

// Exact 140828 command bodies.
int HandleLoadingCart(CUser* user, PacketBuf& packet);  // CMD546: u32 item id
int HandleStart(CUser* user, PacketBuf& packet);        // CMD547: empty
int HandleReward(CUser* user, PacketBuf& packet);       // CMD548: u8 percent

// Typed DB -> game response boundary for Taiwan inner 27.
void OnRewardClaimResult(CUser* user,
                         const RewardClaimMessage& response);

}  // namespace golden_commerce_0828

class InterGoldenCommerceReward
    : public TaiwanInterfaceInterDispatcher {
public:
    InterGoldenCommerceReward();
    virtual ~InterGoldenCommerceReward();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(InterGoldenCommerceReward) == 0x08u,
              "Golden Commerce Taiwan Inter owner must preserve base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_ESCORT_GOLDEN_COMMERCE_0828_H_
