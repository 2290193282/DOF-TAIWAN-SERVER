#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_DECIDE_LEVELUP_EVENT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_DECIDE_LEVELUP_EVENT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>

namespace decide_levelup_db {

// RECONSTRUCTED_0828: slot 844 is unused in both restored 852-entry routing
// tables.  It carries only the native decide-level event's internal DB work;
// it is not an 0627 packet or claimed original symbol.
enum {
    kDbSignal = 844,
    kOperationLoad = 1,
    kOperationDecide = 2,
    kOperationBindCharacter = 3,
    kOperationReserveRewards = 4,
    kOperationFinalizeRewards = 5,
    kOperationRollbackRewards = 6,
    kOperationPrepareRewardPlan = 7,
    kOperationMailPersistenceAck = 8,

    kResultOk = 0,
    kResultInvalidTarget = 1,
    kResultAlreadyDecided = 18,
    kResultReservationRequired = 19,
    kResultDurabilityPending = 20,
    kResultOperationFailed = 255,

    kPlanStateNone = 0,
    kPlanStatePrepared = 1,
    kPlanStateFinalized = 2,

    kDeliveryEventCoin = 1,
    kDeliveryDirectInventory = 2,
    kDeliveryTrackedMail = 3,
    // RewardDelivery carries the exact mail post-image so a reconnect never
    // regenerates GUID/expiration/random fields.  Twenty-seven 68-byte
    // records keep the complete signal frame (8-byte envelope + Message)
    // below Stream's 2048-byte hard limit; game-side batching splits larger
    // level gaps without splitting any PVF level.
    kMaximumRewardDeliveries = 27,
};

#pragma pack(push, 1)
struct RewardDelivery {
    uint32_t item_id;
    uint32_t count;
    // Non-zero only for tracked mail.  The 128-bit token is also the DB3
    // postal uniqueness key carried by signal 128's persistence context.
    uint8_t delivery_token[16];
    uint8_t kind;
    // Exact InvenItemMailRecord projection captured once, before the durable
    // plan is inserted.  All fields are zero for EventCoin/direct inventory.
    uint8_t seal_flag;
    uint8_t upgrade;
    uint8_t amplify_option;
    uint16_t endurance;
    uint16_t amplify_value;
    uint8_t separate_upgrade;
    uint8_t avatar_flag;
    uint8_t creature_flag;
    uint8_t reserved;
    uint32_t extend_info;
    uint32_t chronicle_value;
    uint8_t item_guid[10];
    uint8_t random_option[14];
};

struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t target_level;
    uint32_t previous_reward_level;
    uint32_t current_reward_level;
    uint32_t claim_token;
    uint32_t session_token;
    int32_t server_group;
    uint8_t operation;
    uint8_t result;
    uint8_t main_reward_decided;
    uint8_t main_reward_newly_decided;
    uint8_t plan_state;
    uint8_t reward_count;
    uint8_t mail_count;
    uint8_t mail_persisted;
    uint32_t event_coin_before;
    uint32_t event_coin_after;
    uint8_t ack_delivery_token[16];
    uint32_t ack_reward_item_id;
    uint8_t inventory_pre_sha256[32];
    uint8_t inventory_post_sha256[32];
    uint8_t reward_plan_sha256[32];
    RewardDelivery deliveries[kMaximumRewardDeliveries];
};
#pragma pack(pop)

static_assert(sizeof(RewardDelivery) == 68u,
              "decide-level delivery record ABI drift");
static_assert(offsetof(Message, deliveries) == 164u &&
                  sizeof(Message) == 2000u,
              "decide-level DB message must remain pointer-free and fixed");
static_assert(sizeof(Message) + 8u <= 2048u,
              "decide-level DB frame must fit the legacy Stream block");

class DB_DecideLevelupEvent : public DBDispatcher {
public:
    DB_DecideLevelupEvent();
    virtual ~DB_DecideLevelupEvent();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool Queue(int user_idx, const Message& request);
    static bool QueueAfterLegacyInventory(int user_idx,
                                          const Message& request);
};

}  // namespace decide_levelup_db

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_DECIDE_LEVELUP_EVENT_H_
