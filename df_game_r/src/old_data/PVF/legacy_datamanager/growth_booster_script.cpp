#include "old_game/activity/reward/growth_booster_abi.h"

#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"

#include <stdint.h>

#include <string>

// IDA_EA: 0x8911EE3
// IDA_NAME: _Z17importGrowthPowerPcP17STGrowthPowerData
// DEMANGLED: importGrowthPower(char *,STGrowthPowerData *)
int importGrowthPower(char* filename, STGrowthPowerData* data) {
    if (!data) {
        return 0;
    }

    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string job_name;
    while (ScanType(tag, true) == 1) {
        if (tag == "[breakaway section]") {
            while (1) {
                bool success = false;
                STBreakAwaySection section;
                section.index = static_cast<uint8_t>(
                    ScanInt(&success));
                if (!success) {
                    break;
                }

                section.low_level = static_cast<uint8_t>(
                    ScanInt(static_cast<bool*>(0)));
                section.high_level = static_cast<uint8_t>(
                    ScanInt(static_cast<bool*>(0)));
                data->break_away_sections.push_back(section);
            }
        } else if (tag == "[exp reward]") {
            while (1) {
                bool success = false;
                STExpReward reward;
                reward.section_index = static_cast<uint8_t>(
                    ScanInt(&success));
                if (!success) {
                    break;
                }

                reward.gained_exp_bonus_rate = static_cast<uint8_t>(
                    ScanInt(static_cast<bool*>(0)));
                reward.clear_reward_exp_bonus_rate = static_cast<uint8_t>(
                    ScanInt(static_cast<bool*>(0)));
                reward.additional_reward_rate = static_cast<uint8_t>(
                    ScanInt(static_cast<bool*>(0)));
                data->exp_rewards.push_back(reward);
            }
        } else if (tag == "[package reward]") {
            while (ScanType(tag, true) == 1) {
                if (tag == "[group]") {
                    if (ScanStr(&job_name) != 1) {
                        return 0;
                    }

                    const int character_job =
                        getCharacterJob(job_name.c_str());
                    const int rank =
                        ScanInt(static_cast<bool*>(0));
                    const int package_key =
                        makeFairPVPEquipmentPackageKey(character_job,
                                                               rank);

                    FairPvpEquipmentPackageList package_list;
                    ScanType(tag, true);
                    while (tag == "[package selection]") {
                        FairPvpEquipmentPackage package;
                        while (1) {
                            bool success = false;
                            const int item_index =
                                ScanInt(&success);
                            if (!success) {
                                break;
                            }

                            STStackablePackageData package_data;
                            package_data.item_index = item_index;
                            package_data.item_count =
                                ScanInt(static_cast<bool*>(0));
                            package.push_back(package_data);
                        }

                        package_list.push_back(package);
                        ScanType(tag, true);
                    }

                    data->package_rewards[package_key] = package_list;
                } else if (tag == "[/package reward]") {
                    break;
                }
            }
        }
    }

    return 1;
}

// IDA_EA: 0x8913A7E
// IDA_NAME: _Z17importBoosterGagePcP15BoosterGageData
// DEMANGLED: importBoosterGage(char *,BoosterGageData *)
int importBoosterGage(char* filename, BoosterGageData* data) {
    if (!data) {
        return 0;
    }

    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[max gage]") {
            bool success = false;
            data->max_gage =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        } else if (tag == "[sub gage]") {
            bool success = false;
            data->sub_gage =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        } else if (tag == "[reward multiple]") {
            bool success = false;
            data->reward_multiple =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        } else if (tag == "[booster point]") {
            while (1) {
                bool success = false;
                BoosterPoint point;
                point.item_index = static_cast<uint32_t>(
                    ScanInt(&success));
                if (!success) {
                    break;
                }

                point.point = static_cast<uint8_t>(
                    ScanInt(&success));
                if (!success) {
                    break;
                }
                data->booster_points.push_back(point);
            }
        }
    }

    return 1;
}
