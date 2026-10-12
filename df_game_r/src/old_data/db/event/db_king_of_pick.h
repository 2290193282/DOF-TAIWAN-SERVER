#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_KING_OF_PICK_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_KING_OF_PICK_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>

namespace king_of_pick_db {

enum {
    // RECONSTRUCTED_0828: root reserved empty slot 831 in both the DB_Q and
    // DISPATCH_Q tables for the complete King-of-Pick operation family.
    kDbSignal = 831,

    kOperationLoad = 1,
    kOperationDailyRefill = 2,
    kOperationProperDungeonClear = 3,
    kOperationDraw = 4,

    kResultOk = 0,
    kResultInvalidEligibility = 19,
    kResultNoPickChance = 20,
    kResultDailyLimit = 21,
    kResultRewardUnavailable = 22,
    kResultDeliveryPending = 254,
    kResultOperationFailed = 255,

    kMaximumHistoryCount = 100,
    kPoolItemCount = 4,
    kMailTitleBytes = 32,
    kMailContentBytes = 124,
};

#pragma pack(push, 1)
struct MailReward {
    uint32_t item_id;
    uint32_t add_info;
    uint32_t extend_info;
    uint32_t chronicle_value;
    uint16_t endurance;
    uint16_t amplify_value;
    uint8_t seal_flag;
    uint8_t item_attr;
    uint8_t amplify_option;
    uint8_t separate_upgrade;
    uint8_t avatar_flag;
    uint8_t creature_flag;
    uint8_t item_guid[10];
    uint8_t random_option[14];
};

struct HistoryEntry {
    uint16_t selected_position;
    uint32_t reward_item_id;
};

struct PoolReward {
    uint32_t initial_count;
    MailReward reward;
};

// A deliberately fixed payload is copied unchanged through DB_Q and
// DISPATCH_Q.  Identity/session fields let the game thread discard a stale
// pooled-user reply; the complete 100-cell history lets CMD589 be rebuilt
// without a second asynchronous query.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_id;
    int32_t server_group;
    uint32_t revision;
    // Request direction: a loaded session sends its authoritative DB day/week
    // keys.  A not-yet-loaded session sends day_key=0 and a server-generated
    // Unix source epoch in week_key.  Response direction: DB overwrites both
    // fields with the authoritative activity day/week keys.  Reusing the
    // fixed response slots preserves the 1024-byte DB_Q ABI while preventing
    // a request queued before 06:00 from being applied to the next day.
    uint32_t day_key;
    uint32_t week_key;
    uint16_t requested_position;
    uint16_t remaining_pick_chance;
    uint16_t history_count;
    uint16_t daily_proper_clear_count;
    uint16_t proper_clear_serial;
    uint16_t non_reward_clear_serial;
    uint8_t operation;
    uint8_t result;
    uint8_t replayed;
    uint8_t new_mail;
    uint8_t character_level;
    uint8_t has_non_reward_clear_boundary;
    uint8_t reserved_50[2];
    HistoryEntry history[kMaximumHistoryCount];
    PoolReward pool[kPoolItemCount];
    char mail_title[kMailTitleBytes];
    char mail_content[kMailContentBytes];
};
#pragma pack(pop)

static_assert(sizeof(MailReward) == 50u,
              "King-of-Pick mail record must remain 50 bytes");
static_assert(sizeof(HistoryEntry) == 6u,
              "King-of-Pick history entry must remain 6 bytes");
static_assert(sizeof(PoolReward) == 54u,
              "King-of-Pick pool entry must remain 54 bytes");
static_assert(sizeof(Message) == 1024u,
              "King-of-Pick DB message must remain 1024 bytes");
static_assert(offsetof(Message, history) == 52u,
              "King-of-Pick history ABI offset must remain 0x34");

void ClearMessage(Message& message);
bool Queue(int user_idx, const Message& message);

class DB_KingOfPick : public DBDispatcher {
public:
    DB_KingOfPick();
    virtual ~DB_KingOfPick();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

}  // namespace king_of_pick_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_KING_OF_PICK_0828_H_
