#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_TIME_SPIRAL_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_TIME_SPIRAL_RUNTIME_H_

#include <stdint.h>

class CParty;
class CUser;

namespace time_spiral {

struct BreakTrapSelection {
    BreakTrapSelection();

    uint8_t target_x;
    uint8_t target_y;
    uint8_t result;
};

enum MoveRequestResolution {
    kMoveRequestRejected = -1,
    kMoveRequestOrdinary = 0,
    kMoveRequestAuthorized = 1,
};

// A successful SelectDungeon creates a new party-owned generation.  No
// CParty/CBattle_Field layout is extended; every lifecycle boundary below
// operates on the mutex-protected sidecar owned by this TU.
bool BeginRun(CParty* party, int dungeon_index);
void ResetParty(CParty* party);

// Called only after the inherited DestroyPassiveObject authority accepted the
// static passive-object script id in the authoritative current map.
bool ArmAfterTrapDestroyed(CParty* party, int object_id);

// First Armed request draws once.  A Selected replay returns the same tuple.
bool SelectOrReplay(CUser* user, BreakTrapSelection* selection);

// Resolve accepts only the selected target or that source's ordinary exit.
// The latter is rewritten to the selected target inside CParty::move_map.
MoveRequestResolution ResolveMoveRequest(CParty* party,
                                         int request_x,
                                         int request_y,
                                         int* target_x,
                                         int* target_y);

// CBattle_Field keeps the selected coordinate as the real destination while
// deriving invade direction from this adjacent ordinary exit.  Only an exact
// selected target returns true, so the range bypass cannot become global.
bool GetMoveOverride(CParty* party,
                     int target_x,
                     int target_y,
                     int* logical_exit_x,
                     int* logical_exit_y);

// Consume only after the inherited movement pipeline has committed its new
// current grid/map.  An error leaves Selected intact for a retry.
void ConsumeMoveSuccess(CParty* party, int target_x, int target_y);

// The start callback may be observed more than once.  These functions bind
// ticket consumption to the current run generation and party/user identity.
bool ShouldConsumeTicket(CParty* party, CUser* user, int dungeon_index);
bool IsTicketConsumed(CParty* party, CUser* user, int dungeon_index);
void MarkTicketConsumed(CParty* party, CUser* user, int dungeon_index);

}  // namespace time_spiral

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_TIME_SPIRAL_RUNTIME_H_
