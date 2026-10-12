#ifndef OLDSRV_OLD_DATA_PVF_EVENT_UNLIMIT_CHALLENGE_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_UNLIMIT_CHALLENGE_SCRIPT_H_

#include "old_game/activity/schedule/event_value_types.h"

#include <stdint.h>

#include <map>
#include <string>
#include <vector>


// IDA_EA: 0x891B454
// IDA_NAME: _ZNK21stUnlimitChallengeKeyltERKS_
// DEMANGLED: stUnlimitChallengeKey::operator<(stUnlimitChallengeKey const&)const
struct stUnlimitChallengeKey {
    bool operator<(const stUnlimitChallengeKey& rhs) const;

    uint16_t level;
    uint8_t job;
    uint8_t reserved_03;
};

// IDA proves this value as the two dwords paired with the 0x04-byte key.
struct stUnlimitChallengeItem {
    int32_t item_id;
    int32_t item_count;
};

struct stUnlimitChallengeMessageInfo {
    // IDA_EA: 0x891C84A
    stUnlimitChallengeMessageInfo();
    // IDA_EA: 0x892BFA6
    stUnlimitChallengeMessageInfo(
        const stUnlimitChallengeMessageInfo& rhs);
    // IDA_EA: 0x83E6154
    ~stUnlimitChallengeMessageInfo();

    // IDA_EA: 0x892C070
    stUnlimitChallengeMessageInfo& operator=(
        const stUnlimitChallengeMessageInfo& rhs);

    uint16_t level;
    uint16_t reserved_02;
    std::string server_message;
    std::string client_message;
};

struct stUnlimitChallengeInfo {
    // IDA_EA: 0x837C53C
    stUnlimitChallengeInfo();
    // IDA_EA: 0x837C5FA
    ~stUnlimitChallengeInfo();

    uint8_t enabled;
    uint8_t reserved_01;
    uint8_t reserved_02;
    uint8_t reserved_03;
    std::vector<int> server_ids;
    std::vector<int> levels;
    std::vector<stUnlimitChallengeRewardInfo> server_rewards;
    std::vector<stUnlimitChallengeMessageInfo> messages;
    std::multimap<stUnlimitChallengeKey,
                  stUnlimitChallengeItem> client_rewards;
};

// IDA_EA: 0x890BC63
// IDA_NAME: _Z28importUnlimitChallengeScriptPKcP22stUnlimitChallengeInfo
// DEMANGLED: importUnlimitChallengeScript(char const*,stUnlimitChallengeInfo *)
int importUnlimitChallengeScript(const char* filename,
                                 stUnlimitChallengeInfo* info);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stUnlimitChallengeKey) == 0x04u,
              "stUnlimitChallengeKey IDA ABI size must stay 0x04");
static_assert(sizeof(stUnlimitChallengeItem) == 0x08u,
              "stUnlimitChallengeItem IDA ABI size must stay 0x08");
static_assert(sizeof(stUnlimitChallengeMessageInfo) == 0x0Cu,
              "stUnlimitChallengeMessageInfo IDA ABI size must stay 0x0C");
static_assert(sizeof(stUnlimitChallengeInfo) == 0x4Cu,
              "stUnlimitChallengeInfo IDA ABI size must stay 0x4C");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_UNLIMIT_CHALLENGE_SCRIPT_H_
