#include "old_data/PVF/event/level_reward_scripts.h"

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <utility>

// IDA_EA: 0x89107E9
// IDA_NAME: _Z29ImportLevelUpRewardItemScriptPKcR25stLevelUpRewardItemScript
// DEMANGLED: ImportLevelUpRewardItemScript(char const*,stLevelUpRewardItemScript &)
int ImportLevelUpRewardItemScript(const char* filename,
                                  stLevelUpRewardItemScript& script) {
    // IDA uses the shared one-byte empty literal at 0x8E0F3C0 as the RDAR
    // directory argument.  It is not a per-parser synthetic default path.
    if (!loadRDARScriptFile("", filename)) {
        return 10919;
    }

    std::string tag;
    bool success = false;
    LevelUpRewardItemTableKey key;
    LevelUpRewardItemData data;

    while (ScanType(tag, true) == 1) {
        if (tag == "[swordman]") {
            key.charac_job = oldsrv::config::kCharacterJobSwordman;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 10948;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 10952;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[fighter]") {
            key.charac_job = oldsrv::config::kCharacterJobFighter;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 10973;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 10977;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[at fighter]") {
            key.charac_job = oldsrv::config::kCharacterJobAtFighter;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 10998;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11002;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[gunner]") {
            key.charac_job = oldsrv::config::kCharacterJobGunner;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11023;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11027;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[at gunner]") {
            key.charac_job = oldsrv::config::kCharacterJobAtGunner;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11048;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11052;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[mage]") {
            key.charac_job = oldsrv::config::kCharacterJobMage;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11073;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11077;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[priest]") {
            key.charac_job = oldsrv::config::kCharacterJobPriest;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11098;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11102;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[thief]") {
            key.charac_job = oldsrv::config::kCharacterJobThief;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11123;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11127;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[at mage]") {
            // 0828 client sub_167F060 assigns this exact block to job 8.
            key.charac_job = oldsrv::config::kCharacterJobAtMage;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11148;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11152;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[demonic swordman]") {
            // 0828 client sub_167F060 assigns this exact block to job 9.
            key.charac_job = oldsrv::config::kCharacterJobDemonicSwordman;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11173;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11177;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[creator mage]") {
            key.charac_job = oldsrv::config::kCharacterJobCreatorMage;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11198;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11202;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        } else if (tag == "[at swordman]") {
            key.charac_job = oldsrv::config::kCharacterJobAtSwordman;
            key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            while (true) {
                key.charac_level = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data.item_id = ScanInt(&success);
                if (!success) {
                    return 11223;
                }
                data.count = ScanInt(&success);
                if (!success) {
                    return 11227;
                }
                script.reward_items.insert(std::make_pair(key, data));
            }
        }
    }

    return 0;
}
