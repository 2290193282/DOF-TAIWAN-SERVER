#ifndef OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_CLONE_TITLE_RUNTIME_H_
#define OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_CLONE_TITLE_RUNTIME_H_

#include <stdint.h>


class CInventory;
class CUser;

namespace clone_title_db {
struct Message;
}

namespace clone_title {

enum DatabaseReplyAction {
    kDatabaseReplyIgnored = 0,
    kDatabaseReplyLoadApplied = 1,
    kDatabaseReplyLoadFailed = 2,
    kDatabaseReplySaveSucceeded = 3,
    kDatabaseReplySaveFailed = 4,
};

// Queue an idempotent DB_Q preload for a character in this CUser session.
// Duplicate loading/loaded requests are suppressed under the cache mutex.
bool RequestLoad(CUser* user, uint32_t charac_no);

// Resolve the clone-title state cached for this exact current/tag-character
// inventory.  A cache miss deliberately returns zero rather than issuing SQL
// from a packet/game thread or borrowing another character's appearance.
uint32_t GetForInventory(CUser* user, const CInventory* inventory);

// Validate CMD562 and queue its durable save.  The session cache is changed
// only after the DB_Q response succeeds, so the command has explicit ACK /
// failure semantics instead of acknowledging an uncommitted local mutation.
bool SetForCurrentCharacter(CUser* user, uint32_t item_id);

// Apply one signal-822 response on DispatchThread.  The caller owns client
// ACK/NOTI emission according to the returned action.
DatabaseReplyAction ApplyDatabaseResult(
    CUser* user,
    const clone_title_db::Message& message,
    uint32_t* applied_item_id);

// CUser objects are pooled.  Drop all per-session cache entries before their
// character vector is reset or the object is destroyed.
void ResetUser(CUser* user);

}  // namespace clone_title

#endif  // OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_CLONE_TITLE_RUNTIME_H_
