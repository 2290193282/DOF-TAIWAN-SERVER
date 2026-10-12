#include "old_game/activity/reward/advance_altar_shop_parameter.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_game/activity/reward/advance_altar.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace advancealtar {

// IDA_EA: 0x88A10CC
// IDA_NAME: _ZN12advancealtar25AdvanceAltarShopParameter31importAdvanceAltarShopParameterEPKc
// DEMANGLED: advancealtar::AdvanceAltarShopParameter::importAdvanceAltarShopParameter(char const*)
int AdvanceAltarShopParameter::importAdvanceAltarShopParameter(
    const char* filename) {
    std::string tag;
    bool success = false;

    if (loadRDARScriptFile("", filename) == 1) {
        while (ScanType(tag, true) == 1) {
            if (tag == "[star reset fee gold]") {
                while (1) {
                    _StarRestFeeByGold fee;
                    const int star = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    fee.gold_fee = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    star_reset_fee_by_gold.insert(std::make_pair(star, fee));
                }
            } else if (tag == "[ridable id]") {
                BuyShopData shop_data;
                const int ridable_id = ScanInt(&success);
                int ridable_block_valid = 0;

                if (success && isValidRidableId(
                                   static_cast<RidableId::T>(ridable_id))) {
                    shop_data.ridable_id = ridable_id;

                    while (ScanType(tag, true) == 1) {
                        if (tag == "[unit]") {
                            while (ScanType(tag, true) == 1) {
                                if (tag == "[id]") {
                                    BuyUpgradeData upgrade_data;
                                    const int id = ScanInt(&success);
                                    int id_valid = 0;

                                    if (success) {
                                        upgrade_data.id = id;
                                        while (ScanType(tag, true) == 1) {
                                            if (tag ==
                                                "[unlock stage index]") {
                                                const int unlock_stage_index =
                                                    ScanInt(&success);
                                                if (!success) {
                                                    break;
                                                }
                                                upgrade_data
                                                    .unlock_stage_index =
                                                    unlock_stage_index;
                                            } else if (tag == "[name]") {
                                                if (ScanStr(
                                                        &upgrade_data.name) !=
                                                    1) {
                                                    break;
                                                }
                                            } else if (tag == "[field name]") {
                                                std::string field_name;
                                                while (ScanStr(&field_name) ==
                                                       1) {
                                                    const FieldType::T
                                                        field_type =
                                                            getFieldType(
                                                                field_name);
                                                    if (field_type !=
                                                        FieldType::INVALID) {
                                                        upgrade_data
                                                            .field_types
                                                            .push_back(
                                                                field_type);
                                                    }
                                                }
                                            } else if (tag == "[list]") {
                                                FieldData field_data;
                                                std::map<FieldType::T,
                                                         FieldData>
                                                    row;
                                                char keep_scanning = 1;

                                                while (keep_scanning) {
                                                    row.clear();
                                                    for (int index = 0;
                                                         index <
                                                         static_cast<int>(
                                                             upgrade_data
                                                                 .field_types
                                                                 .size());
                                                         ++index) {
                                                        const FieldType::T
                                                            field_type =
                                                                upgrade_data
                                                                    .field_types
                                                                    [index];
                                                        field_data.field_type =
                                                            field_type;
                                                        if (getdataType(
                                                                field_type)) {
                                                            const int value =
                                                                ScanInt(
                                                                    &success);
                                                            if (!success) {
                                                                keep_scanning =
                                                                    0;
                                                                break;
                                                            }
                                                            field_data
                                                                .int_value =
                                                                value;
                                                        } else {
                                                            std::string text;
                                                            if (ScanStr(
                                                                    &text) ==
                                                                1) {
                                                                field_data
                                                                    .string_value =
                                                                    text;
                                                            } else {
                                                                keep_scanning =
                                                                    0;
                                                                break;
                                                            }
                                                        }

                                                        if (keep_scanning) {
                                                            row.insert(
                                                                std::make_pair(
                                                                    field_type,
                                                                    field_data));
                                                        }
                                                    }

                                                    if (keep_scanning) {
                                                        const int level =
                                                            row[FieldType::LEVEL]
                                                                .int_value;
                                                        upgrade_data
                                                            .level_fields
                                                            .insert(
                                                                std::make_pair(
                                                                    level,
                                                                    row));
                                                    }
                                                }
                                                shop_data.unit_upgrades
                                                    .push_back(upgrade_data);
                                            } else if (tag == "[/id]") {
                                                break;
                                            }
                                        }
                                        id_valid = 1;
                                    }

                                    if (!id_valid) {
                                        break;
                                    }
                                } else if (tag == "[/unit]") {
                                    break;
                                }
                            }
                        } else if (tag == "[skill]") {
                            while (ScanType(tag, true) == 1) {
                                if (tag == "[id]") {
                                    BuyUpgradeData upgrade_data;
                                    const int id = ScanInt(&success);
                                    int id_valid = 0;

                                    if (success) {
                                        upgrade_data.id = id;
                                        while (ScanType(tag, true) == 1) {
                                            if (tag ==
                                                "[unlock stage index]") {
                                                const int unlock_stage_index =
                                                    ScanInt(&success);
                                                if (!success) {
                                                    break;
                                                }
                                                upgrade_data
                                                    .unlock_stage_index =
                                                    unlock_stage_index;
                                            } else if (
                                                tag == "[is settable slot]") {
                                                const int is_settable_slot =
                                                    ScanInt(&success);
                                                if (!success) {
                                                    break;
                                                }
                                                upgrade_data
                                                    .is_settable_slot =
                                                    is_settable_slot != 0;
                                            } else if (tag == "[name]") {
                                                if (ScanStr(
                                                        &upgrade_data.name) !=
                                                    1) {
                                                    break;
                                                }
                                            } else if (tag == "[field name]") {
                                                std::string field_name;
                                                while (ScanStr(&field_name) ==
                                                       1) {
                                                    const FieldType::T
                                                        field_type =
                                                            getFieldType(
                                                                field_name);
                                                    if (field_type !=
                                                        FieldType::INVALID) {
                                                        upgrade_data
                                                            .field_types
                                                            .push_back(
                                                                field_type);
                                                    }
                                                }
                                            } else if (tag == "[list]") {
                                                FieldData field_data;
                                                std::map<FieldType::T,
                                                         FieldData>
                                                    row;
                                                char keep_scanning = 1;

                                                while (keep_scanning) {
                                                    row.clear();
                                                    for (int index = 0;
                                                         index <
                                                         static_cast<int>(
                                                             upgrade_data
                                                                 .field_types
                                                                 .size());
                                                         ++index) {
                                                        const FieldType::T
                                                            field_type =
                                                                upgrade_data
                                                                    .field_types
                                                                    [index];
                                                        field_data.field_type =
                                                            field_type;
                                                        if (getdataType(
                                                                field_type)) {
                                                            const int value =
                                                                ScanInt(
                                                                    &success);
                                                            if (!success) {
                                                                keep_scanning =
                                                                    0;
                                                                break;
                                                            }
                                                            field_data
                                                                .int_value =
                                                                value;
                                                        } else {
                                                            std::string text;
                                                            if (ScanStr(
                                                                    &text) ==
                                                                1) {
                                                                field_data
                                                                    .string_value =
                                                                    text;
                                                            } else {
                                                                keep_scanning =
                                                                    0;
                                                                break;
                                                            }
                                                        }

                                                        row.insert(
                                                            std::make_pair(
                                                                field_type,
                                                                field_data));
                                                    }

                                                    if (keep_scanning) {
                                                        const int level =
                                                            row[FieldType::LEVEL]
                                                                .int_value;
                                                        upgrade_data
                                                            .level_fields
                                                            .insert(
                                                                std::make_pair(
                                                                    level,
                                                                    row));
                                                    }
                                                }
                                                shop_data.skill_upgrades
                                                    .push_back(upgrade_data);
                                            } else if (tag == "[/id]") {
                                                break;
                                            }
                                        }
                                        id_valid = 1;
                                    }

                                    if (!id_valid) {
                                        break;
                                    }
                                } else if (tag == "[/skill]") {
                                    break;
                                }
                            }
                        } else if (tag == "[tower]") {
                            while (ScanType(tag, true) == 1) {
                                if (tag == "[id]") {
                                    BuyUpgradeData upgrade_data;
                                    const int id = ScanInt(&success);
                                    int id_valid = 0;

                                    if (success) {
                                        upgrade_data.id = id;
                                        while (ScanType(tag, true) == 1) {
                                            if (tag ==
                                                "[unlock stage index]") {
                                                const int unlock_stage_index =
                                                    ScanInt(&success);
                                                if (!success) {
                                                    break;
                                                }
                                                upgrade_data
                                                    .unlock_stage_index =
                                                    unlock_stage_index;
                                            } else if (tag == "[name]") {
                                                if (ScanStr(
                                                        &upgrade_data.name) !=
                                                    1) {
                                                    break;
                                                }
                                            } else if (tag == "[field name]") {
                                                std::string field_name;
                                                while (ScanStr(&field_name) ==
                                                       1) {
                                                    const FieldType::T
                                                        field_type =
                                                            getFieldType(
                                                                field_name);
                                                    if (field_type !=
                                                        FieldType::INVALID) {
                                                        upgrade_data
                                                            .field_types
                                                            .push_back(
                                                                field_type);
                                                    }
                                                }
                                            } else if (tag == "[list]") {
                                                FieldData field_data;
                                                std::map<FieldType::T,
                                                         FieldData>
                                                    row;
                                                char keep_scanning = 1;

                                                while (keep_scanning) {
                                                    row.clear();
                                                    for (int index = 0;
                                                         index <
                                                         static_cast<int>(
                                                             upgrade_data
                                                                 .field_types
                                                                 .size());
                                                         ++index) {
                                                        const FieldType::T
                                                            field_type =
                                                                upgrade_data
                                                                    .field_types
                                                                    [index];
                                                        field_data.field_type =
                                                            field_type;
                                                        if (getdataType(
                                                                field_type)) {
                                                            const int value =
                                                                ScanInt(
                                                                    &success);
                                                            if (!success) {
                                                                keep_scanning =
                                                                    0;
                                                                break;
                                                            }
                                                            field_data
                                                                .int_value =
                                                                value;
                                                        } else {
                                                            std::string text;
                                                            if (ScanStr(
                                                                    &text) ==
                                                                1) {
                                                                field_data
                                                                    .string_value =
                                                                    text;
                                                            } else {
                                                                keep_scanning =
                                                                    0;
                                                                break;
                                                            }
                                                        }

                                                        row.insert(
                                                            std::make_pair(
                                                                field_type,
                                                                field_data));
                                                    }

                                                    if (keep_scanning) {
                                                        const int level =
                                                            row[FieldType::LEVEL]
                                                                .int_value;
                                                        upgrade_data
                                                            .level_fields
                                                            .insert(
                                                                std::make_pair(
                                                                    level,
                                                                    row));
                                                    }
                                                }
                                                shop_data.tower_upgrades
                                                    .push_back(upgrade_data);
                                            } else if (tag == "[/id]") {
                                                break;
                                            }
                                        }
                                        id_valid = 1;
                                    }

                                    if (!id_valid) {
                                        break;
                                    }
                                } else if (tag == "[/tower]") {
                                    break;
                                }
                            }
                        } else if (tag == "[/ridable id]") {
                            break;
                        }
                    }

                    buy_shop_data.insert(
                        std::make_pair(shop_data.ridable_id, shop_data));
                    ridable_block_valid = 1;
                }

                if (!ridable_block_valid) {
                    break;
                }
            }
        }
    }

    return 0;
}

}  // namespace advancealtar
