#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_LEVELUP_SUPPORT_3RD_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_LEVELUP_SUPPORT_3RD_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace levelup_support_3rd_db {

enum {
    // RECONSTRUCTED_0828: slot 840 is empty in both restored 852-entry DB
    // routing tables.  It is an internal game/DB route, not a client command.
    kDbSignal = 840,
    kOperationLoad = 1,
    kOperationClaim = 2,

    kResultOk = 0,
    kResultInvalidEligibility = 19,
    kResultRewardUnavailable = 22,
    kResultOperationFailed = 255,

    // The 0828 PVF currently has one record for every support level.  The
    // packet's u32 count and loop prove a list, so retain bounded headroom.
    kMaximumRewardCount = 8,
    kMailTitleBytes = 29,
    kMailContentBytes = 255,
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

struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t requested_level;
    uint32_t last_claimed_level;
    uint32_t claim_revision;
    uint32_t session_token;
    int32_t server_group;
    uint8_t operation;
    uint8_t result;
    uint8_t reward_count;
    uint8_t replayed;
    uint8_t character_level;
    uint8_t reserved[3];
    char mail_title[kMailTitleBytes + 1];
    char mail_content[kMailContentBytes + 1];
    MailReward rewards[kMaximumRewardCount];
};
#pragma pack(pop)

static_assert(sizeof(MailReward) == 50u,
              "Level-up Support 3rd mail record must remain 50 bytes");
static_assert(sizeof(Message) == 722u,
              "Level-up Support 3rd DB message must remain 722 bytes");

void ClearMessage(Message& message);
bool Queue(int user_idx, const Message& message);

class DB_LevelupSupport3rd : public DBDispatcher {
public:
    DB_LevelupSupport3rd();
    virtual ~DB_LevelupSupport3rd();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

}  // namespace levelup_support_3rd_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_LEVELUP_SUPPORT_3RD_0828_H_

