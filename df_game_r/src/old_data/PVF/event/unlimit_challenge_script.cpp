#include "old_data/PVF/event/unlimit_challenge_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <utility>


// IDA_EA: 0x891B454
// IDA_NAME: _ZNK21stUnlimitChallengeKeyltERKS_
// DEMANGLED: stUnlimitChallengeKey::operator<(stUnlimitChallengeKey const&)const
bool stUnlimitChallengeKey::operator<(
    const stUnlimitChallengeKey& rhs) const {
    if (level < rhs.level) {
        return true;
    }
    if (level == rhs.level) {
        return job < rhs.job;
    }
    return false;
}

// IDA_EA: 0x891C84A
// IDA_NAME: _ZN29stUnlimitChallengeMessageInfoC2Ev
// DEMANGLED: stUnlimitChallengeMessageInfo::stUnlimitChallengeMessageInfo(void)
stUnlimitChallengeMessageInfo::stUnlimitChallengeMessageInfo() {}

// IDA_EA: 0x892BFA6
// IDA_NAME: _ZN29stUnlimitChallengeMessageInfoC2ERKS_
// DEMANGLED: stUnlimitChallengeMessageInfo::stUnlimitChallengeMessageInfo(stUnlimitChallengeMessageInfo const&)
stUnlimitChallengeMessageInfo::stUnlimitChallengeMessageInfo(
    const stUnlimitChallengeMessageInfo& rhs)
    : level(rhs.level),
      server_message(rhs.server_message),
      client_message(rhs.client_message) {}

// IDA_EA: 0x83E6154
// IDA_NAME: _ZN29stUnlimitChallengeMessageInfoD2Ev
// DEMANGLED: stUnlimitChallengeMessageInfo::~stUnlimitChallengeMessageInfo()
stUnlimitChallengeMessageInfo::~stUnlimitChallengeMessageInfo() {}

// IDA_EA: 0x892C070
// IDA_NAME: _ZN29stUnlimitChallengeMessageInfoaSERKS_
// DEMANGLED: stUnlimitChallengeMessageInfo::operator=(stUnlimitChallengeMessageInfo const&)
stUnlimitChallengeMessageInfo&
stUnlimitChallengeMessageInfo::operator=(
    const stUnlimitChallengeMessageInfo& rhs) {
    level = rhs.level;
    server_message = rhs.server_message;
    client_message = rhs.client_message;
    return *this;
}

// IDA_EA: 0x837C53C
// IDA_NAME: _ZN22stUnlimitChallengeInfoC2Ev
// DEMANGLED: stUnlimitChallengeInfo::stUnlimitChallengeInfo(void)
stUnlimitChallengeInfo::stUnlimitChallengeInfo() {}

// IDA_EA: 0x837C5FA
// IDA_NAME: _ZN22stUnlimitChallengeInfoD2Ev
// DEMANGLED: stUnlimitChallengeInfo::~stUnlimitChallengeInfo()
stUnlimitChallengeInfo::~stUnlimitChallengeInfo() {}

// IDA_EA: 0x890BC63
// IDA_NAME: _Z28importUnlimitChallengeScriptPKcP22stUnlimitChallengeInfo
// DEMANGLED: importUnlimitChallengeScript(char const*,stUnlimitChallengeInfo *)
int importUnlimitChallengeScript(const char* filename,
                                 stUnlimitChallengeInfo* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string section_tag;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[onoff]") {
            const int enabled = ScanInt(&success);
            if (!success) {
                break;
            }
            info->enabled = enabled != 0;
        } else if (tag == "[server_id]") {
            while (1) {
                const int server_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->server_ids.push_back(server_id);
            }
        } else if (tag == "[level]") {
            while (1) {
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->levels.push_back(level);
            }
        } else if (tag == "[reward_server]") {
            while (ScanType(section_tag, true) == 1) {
                int job = -1;
                if (section_tag == "[swordman]") {
                    job = 0;
                } else if (section_tag == "[fighter]") {
                    job = 1;
                } else if (section_tag == "[gunner]") {
                    job = 2;
                } else if (section_tag == "[mage]") {
                    job = 3;
                } else if (section_tag == "[priest]") {
                    job = 4;
                } else if (section_tag == "[atgunner]") {
                    job = 5;
                } else if (section_tag == "[thief]") {
                    job = 6;
                } else {
                    break;
                }

                while (1) {
                    const int level = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    const int item_id = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    const int item_count = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }

                    stUnlimitChallengeRewardInfo reward;
                    reward.level = static_cast<uint16_t>(level);
                    reward.job = static_cast<uint8_t>(job);
                    reward.item_id = item_id;
                    reward.item_count = item_count;
                    info->server_rewards.push_back(reward);
                }
            }
        } else if (tag == "[reward_client]") {
            while (ScanType(section_tag, true) == 1) {
                int job = -1;
                if (section_tag == "[swordman]") {
                    job = 0;
                } else if (section_tag == "[fighter]") {
                    job = 1;
                } else if (section_tag == "[gunner]") {
                    job = 2;
                } else if (section_tag == "[mage]") {
                    job = 3;
                } else if (section_tag == "[priest]") {
                    job = 4;
                } else if (section_tag == "[atgunner]") {
                    job = 5;
                } else if (section_tag == "[thief]") {
                    job = 6;
                } else {
                    break;
                }

                while (1) {
                    const int level = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    const int item_id = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    const int item_count = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }

                    stUnlimitChallengeKey key;
                    key.level = static_cast<uint16_t>(level);
                    key.job = static_cast<uint8_t>(job);
                    stUnlimitChallengeItem item;
                    item.item_id = item_id;
                    item.item_count = item_count;
                    info->client_rewards.insert(std::make_pair(key, item));
                }
            }
        } else if (tag == "[message]") {
            while (1) {
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }

                std::string server_message;
                std::string client_message;
                ScanStr(&server_message);
                ScanStr(&client_message);

                stUnlimitChallengeMessageInfo message;
                message.level = static_cast<uint16_t>(level);
                message.server_message = server_message;
                message.client_message = client_message;
                info->messages.push_back(message);
            }
        }
    }

    return 1;
}
