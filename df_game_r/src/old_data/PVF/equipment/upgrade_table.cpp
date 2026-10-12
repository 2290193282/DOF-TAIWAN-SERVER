#include "old_data/PVF/common/rdar_script.h"
#include "common/config.h"
#include "old_data/PVF/equipment/upgrade_table.h"
#include "infra/log/format/trace.h"

#include <stdint.h>

#include <string>
#include <utility>

// IDA_EA: 0x854B282
// IDA_NAME: _ZN14upgrade_info_tC2Ev
// DEMANGLED: upgrade_info_t::upgrade_info_t(void)
upgrade_info_t::upgrade_info_t()
    : stat_values(),
      stat_value_04(0.0f),
      stat_values_05(),
      parsed_values_08(),
      event_material_item_index(0),
      event_material_item_count(0),
      calculated_cost(0),
      amplification_const(),
      chronicle_stat_values(),
      chronicle_stat_value_23(0.0f),
      chronicle_stat_values_24(),
      chronicle_stat_value_26(0.0f) {}

// IDA_EA: 0x89378CE
// IDA_NAME: _ZN14upgrade_info_tD2Ev
// DEMANGLED: upgrade_info_t::~upgrade_info_t()
upgrade_info_t::~upgrade_info_t() {}

// IDA_EA: 0x891A8C0
// IDA_NAME: _ZN15STResultItemCntC2Ev
// DEMANGLED: STResultItemCnt::STResultItemCnt(void)
STResultItemCnt::STResultItemCnt()
    : item_index(0), item_count(0), item_grade(0) {}

// IDA_EA: 0x8937B14
// IDA_NAME: _ZN15STResultItemCntD2Ev
// DEMANGLED: STResultItemCnt::~STResultItemCnt()
STResultItemCnt::~STResultItemCnt() {}

// IDA_EA: 0x891A8E0
// IDA_NAME: _ZN31ST3rdChronicleDestructionResultC2Ev
// DEMANGLED: ST3rdChronicleDestructionResult::ST3rdChronicleDestructionResult(void)
ST3rdChronicleDestructionResult::ST3rdChronicleDestructionResult()
    : equipment_type_rates(), result_count(0), result_items() {
    result_items.clear();
}

// IDA_EA: 0x854B3A4
// IDA_NAME: _ZN31ST3rdChronicleDestructionResultD2Ev
// DEMANGLED: ST3rdChronicleDestructionResult::~ST3rdChronicleDestructionResult()
ST3rdChronicleDestructionResult::~ST3rdChronicleDestructionResult() {}

// IDA_EA: 0x89003B4
// IDA_NAME: _ZN15upgrade_table_t16MachineFrameInfoC2Ev
// DEMANGLED: upgrade_table_t::MachineFrameInfo::MachineFrameInfo(void)
upgrade_table_t::MachineFrameInfo::MachineFrameInfo()
    : first_rate(1.0f),
      second_rate(1.0f),
      third_rate(1.0f),
      fourth_rate(1.0f),
      frame_type(2) {}

// IDA_EA: 0x89000CA
// IDA_NAME: _ZN15upgrade_table_tC2Ev
// DEMANGLED: upgrade_table_t::upgrade_table_t(void)
upgrade_table_t::upgrade_table_t()
    : upgrade_infos(),
      level_weights(),
      equipment_type_weights(),
      cost_rarity_weights(),
      stat_rarity_weights(),
      upgrade_costs(),
      notice_upgrade_level(0),
      disjoint_bonus_item_index(-1),
      disjoint_equip_level_const(0),
      disjoint_failed_bonus_weights(),
      disjoint_bonus_count_const(0),
      disjoint_correction_grades(),
      repair_cost_rates(),
      destroy_levels_by_rarity(),
      max_upgrade_levels_by_rarity(),
      destruction_result(),
      cost_weights_by_upgrade_level(),
      machine_frame_base_times(),
      machine_frame_infos(10u),
      pc_room_rental_upgrade_infos(),
      item_upgrade_cost_free_event_infos(),
      charm_upgrade_infos(),
      dimensions_space_charm_upgrade_infos() {
    for (int level = 0; level <= 3; ++level) level_weights[level] = 1.0f;
    for (int type = 0;
         type <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++type)
        equipment_type_weights[type] = 1.0f;
    for (int rarity = 0; rarity <= 5; ++rarity) {
        cost_rarity_weights[rarity] = 1.0f;
        stat_rarity_weights[rarity] = 1.0f;
        destroy_levels_by_rarity[rarity] = -1;
        max_upgrade_levels_by_rarity[rarity] = -1;
    }
    for (int level = 0; level <= 99; ++level) {
        upgrade_costs[level] = 1;
        repair_cost_rates[level] = 1.0f;
    }
    cost_weights_by_upgrade_level.clear();
    machine_frame_infos.clear();
}

// IDA_EA: 0x854B3F6
// IDA_NAME: _ZN15upgrade_table_tD2Ev
// DEMANGLED: upgrade_table_t::~upgrade_table_t()
upgrade_table_t::~upgrade_table_t() {}

// IDA_EA: 0x89003EE
// IDA_NAME: _ZNK15upgrade_table_t14GetPenaltyTypeEh11ENUM_RARITYPK14upgrade_info_t
// DEMANGLED: upgrade_table_t::GetPenaltyType(unsigned char,ENUM_RARITY,upgrade_info_t const*)const
int upgrade_table_t::GetPenaltyType(uint8_t upgrade,
                                    ENUM_RARITY rarity,
                                    const upgrade_info_t* info) const {
    const int rarity_index = static_cast<int>(rarity);
    const int destroy_level = destroy_levels_by_rarity[rarity_index];
    if (destroy_level == -1 || upgrade < destroy_level) {
        return info->parsed_values_08[1];
    }
    return 3;
}

// IDA_EA: 0x8900436
// IDA_NAME: _ZNK15upgrade_table_t16get_upgrade_infoEP14upgrade_info_thhh18ENUM_EQUIPMENTTYPE
// DEMANGLED: upgrade_table_t::get_upgrade_info(upgrade_info_t *,unsigned char,unsigned char,unsigned char,ENUM_EQUIPMENTTYPE)const
int upgrade_table_t::get_upgrade_info(
    upgrade_info_t* output,
    uint8_t rarity,
    uint8_t growth_grade,
    uint8_t upgrade,
    ENUM_EQUIPMENTTYPE equipment_type) const {
    return get_upgrade_info_from_table(upgrade_infos,
                                       output,
                                       rarity,
                                       growth_grade,
                                       upgrade,
                                       equipment_type);
}

int upgrade_table_t::get_pc_room_rental_upgrade_info(
    upgrade_info_t* output,
    uint8_t rarity,
    uint8_t growth_grade,
    uint8_t upgrade,
    ENUM_EQUIPMENTTYPE equipment_type) const {
    return get_upgrade_info_from_table(pc_room_rental_upgrade_infos,
                                       output,
                                       rarity,
                                       growth_grade,
                                       upgrade,
                                       equipment_type);
}

int upgrade_table_t::get_item_upgrade_cost_free_event_info(
    upgrade_info_t* output,
    uint8_t rarity,
    uint8_t growth_grade,
    uint8_t upgrade,
    ENUM_EQUIPMENTTYPE equipment_type) const {
    return get_upgrade_info_from_table(item_upgrade_cost_free_event_infos,
                                       output,
                                       rarity,
                                       growth_grade,
                                       upgrade,
                                       equipment_type);
}

int upgrade_table_t::get_charm_upgrade_info(
    upgrade_info_t* output,
    uint8_t rarity,
    uint8_t growth_grade,
    uint8_t upgrade,
    ENUM_EQUIPMENTTYPE equipment_type) const {
    return get_upgrade_info_from_table(charm_upgrade_infos,
                                       output,
                                       rarity,
                                       growth_grade,
                                       upgrade,
                                       equipment_type);
}

int upgrade_table_t::get_dimensions_space_charm_upgrade_info(
    upgrade_info_t* output,
    uint8_t rarity,
    uint8_t growth_grade,
    uint8_t upgrade,
    ENUM_EQUIPMENTTYPE equipment_type) const {
    return get_upgrade_info_from_table(dimensions_space_charm_upgrade_infos,
                                       output,
                                       rarity,
                                       growth_grade,
                                       upgrade,
                                       equipment_type);
}

int upgrade_table_t::get_upgrade_info_from_table(
    const std::vector<upgrade_info_t>& source,
    upgrade_info_t* output,
    uint8_t rarity,
    uint8_t growth_grade,
    uint8_t upgrade,
    ENUM_EQUIPMENTTYPE equipment_type) const {
    const int equipment_type_index = static_cast<int>(equipment_type);
    if (output == 0 || upgrade >= source.size() || rarity > 5u ||
        equipment_type_index > oldsrv::config::kOrdinaryEquipmentTypeLast ||
        equipment_type_index < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
        growth_grade > 99u) {
        return 0;
    }

    *output = source.at(upgrade);
    const long double base = 1.0L;
    for (int index = 0; index <= 3; ++index) {
        output->stat_values[index] = static_cast<float>(
            (static_cast<long double>(growth_grade) / level_weights[index] +
             base) * output->stat_values[index] * stat_rarity_weights[rarity]);
        output->chronicle_stat_values[index] = static_cast<float>(
            (static_cast<long double>(growth_grade) / level_weights[index] +
             base) * output->chronicle_stat_values[index] *
            stat_rarity_weights[rarity]);
    }
    output->stat_value_04 *= stat_rarity_weights[rarity];
    for (int index = 0; index <= 2; ++index)
        output->stat_values_05[index] *= stat_rarity_weights[rarity];
    output->chronicle_stat_value_23 *= stat_rarity_weights[rarity];
    for (int index = 0; index <= 1; ++index)
        output->chronicle_stat_values_24[index] *= stat_rarity_weights[rarity];
    output->chronicle_stat_value_26 *= stat_rarity_weights[rarity];

    output->calculated_cost = upgrade_costs[growth_grade];
    output->calculated_cost = static_cast<int>(
        static_cast<long double>(output->calculated_cost) *
            equipment_type_weights[equipment_type_index] +
        0.5L);
    output->calculated_cost = static_cast<int>(
        static_cast<long double>(output->calculated_cost) *
            cost_rarity_weights[rarity] +
        0.5L);

    const int cost_weight_level = upgrade > 18u ? 18 : upgrade;
    std::map<int, float>::const_iterator found =
        cost_weights_by_upgrade_level.find(cost_weight_level);
    if (found != cost_weights_by_upgrade_level.end()) {
        output->calculated_cost = static_cast<int>(
            found->second * static_cast<float>(output->calculated_cost));
    }
    return 1;
}

// IDA_EA: 0x854B382
// IDA_NAME: _ZNK8WongWork17CItemUpgradeTable19getUpgradeInfoTableEv
// DEMANGLED: WongWork::CItemUpgradeTable::getUpgradeInfoTable(void)const
WongWork::CItemUpgradeTable*
WongWork::CItemUpgradeTable::getUpgradeInfoTable() const {
    return const_cast<WongWork::CItemUpgradeTable*>(this);
}

// IDA_EA: 0x854B38A
// IDA_NAME: _ZN8WongWork17CItemUpgradeTable17loadUpgradeScriptEPKc
// DEMANGLED: WongWork::CItemUpgradeTable::loadUpgradeScript(char const*)
int WongWork::CItemUpgradeTable::loadUpgradeScript(const char* filename) {
    return importUpgradeTable(filename, this);
}

// IDA_EA: 0x854B496
// IDA_NAME: _ZN8WongWork17CItemUpgradeTableC2Ev
// DEMANGLED: WongWork::CItemUpgradeTable::CItemUpgradeTable(void)
WongWork::CItemUpgradeTable::CItemUpgradeTable() : upgrade_table_t() {}

// IDA_EA: 0x854B4AA
// IDA_NAME: _ZN8WongWork17CItemUpgradeTableD2Ev
// DEMANGLED: WongWork::CItemUpgradeTable::~CItemUpgradeTable()
WongWork::CItemUpgradeTable::~CItemUpgradeTable() {}

// IDA_EA: 0x8900C26
// IDA_NAME: _Z18importUpgradeTablePKcP15upgrade_table_t
// DEMANGLED: importUpgradeTable(char const*,upgrade_table_t *)
int importUpgradeTable(const char* filename, upgrade_table_t* table) {
    if (table == 0 || !loadRDARScriptFile("", filename)) {
        return 0;
    }

    // Upgrade.etc and Chronicle.etc are loaded successively into this object.
    // Treat each file as a partial overlay: only a [table type] explicitly
    // present in the current script may replace its corresponding vector.
    std::string tag;
    std::string table_type;
    std::string value_text;
    std::string unused_text;
    (void)unused_text;
    bool primary_table_seen = false;
    bool pc_room_rental_table_seen = false;
    bool cost_free_event_table_seen = false;
    bool charm_table_seen = false;
    bool dimensions_space_charm_table_seen = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[table]") {
            if (ScanType(tag, true) != 1 || tag != "[table type]" ||
                ScanStr(&table_type) != 1) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importUpgradeTable",
                    0,
                    "importUpgradeTable - invalid [table type]");
                return 0;
            }

            std::vector<upgrade_info_t>* target = 0;
            bool* table_seen = 0;
            if (table_type == "normal" || table_type == "amplify") {
                target = &table->upgrade_infos;
                table_seen = &primary_table_seen;
            } else if (table_type == "pc_room_rental") {
                target = &table->pc_room_rental_upgrade_infos;
                table_seen = &pc_room_rental_table_seen;
            } else if (table_type == "item_upgrade_cost_free_event") {
                target = &table->item_upgrade_cost_free_event_infos;
                table_seen = &cost_free_event_table_seen;
            } else if (table_type == "charm_upgrade_table") {
                target = &table->charm_upgrade_infos;
                table_seen = &charm_table_seen;
            } else if (table_type ==
                       "dimensions_space_charm_upgrade_table") {
                target = &table->dimensions_space_charm_upgrade_infos;
                table_seen = &dimensions_space_charm_table_seen;
            } else {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importUpgradeTable",
                    0,
                    "importUpgradeTable - unknown [table type] %s",
                    table_type.c_str());
                return 0;
            }

            if (*table_seen) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importUpgradeTable",
                    0,
                    "importUpgradeTable - duplicated [table type] %s",
                    table_type.c_str());
                return 0;
            }
            *table_seen = true;
            target->clear();

            for (;;) {
                upgrade_info_t row;
                bool success = false;
                row.stat_values[0] = ScanFloat(&success);
                if (!success) {
                    break;
                }
                row.stat_value_04 = 0.0f;
                row.stat_values[1] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.stat_values_05[0] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.stat_values[2] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.stat_values_05[1] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.stat_values[3] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.stat_values_05[2] = 0.0f;
                for (int index = 0; index <= 2; ++index) {
                    row.parsed_values_08[index] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                }
                row.event_material_item_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                row.event_material_item_count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                row.calculated_cost = 0;
                row.chronicle_stat_values[0] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.chronicle_stat_value_23 = 0.0f;
                row.chronicle_stat_values[1] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.chronicle_stat_values_24[0] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.chronicle_stat_values[2] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.chronicle_stat_values_24[1] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.chronicle_stat_values[3] = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                row.chronicle_stat_value_26 = 0.0f;
                target->push_back(row);
            }

            if (target->empty()) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importUpgradeTable",
                    0,
                    "importUpgradeTable - empty [table type] %s",
                    table_type.c_str());
                return 0;
            }
        } else if (tag == "[amplification const]") {
            unsigned int row_index = 0;
            for (;;) {
                bool success = false;
                int constants[5] = {0, 0, 0, 0, 0};
                constants[1] = ScanInt(&success);
                if (!success) {
                    break;
                }
                for (int index = 2; index <= 4; ++index) {
                    constants[index] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                }

                if (row_index < table->upgrade_infos.size()) {
                    for (int index = 0; index <= 4; ++index) {
                        table->upgrade_infos[row_index]
                            .amplification_const[index] = constants[index];
                    }
                }
                if (row_index < table->pc_room_rental_upgrade_infos.size()) {
                    for (int index = 0; index <= 4; ++index) {
                        table->pc_room_rental_upgrade_infos[row_index]
                            .amplification_const[index] = constants[index];
                    }
                }
                if (row_index <
                    table->item_upgrade_cost_free_event_infos.size()) {
                    for (int index = 0; index <= 4; ++index) {
                        table->item_upgrade_cost_free_event_infos[row_index]
                            .amplification_const[index] = constants[index];
                    }
                }
                if (row_index < table->charm_upgrade_infos.size()) {
                    for (int index = 0; index <= 4; ++index) {
                        table->charm_upgrade_infos[row_index]
                            .amplification_const[index] = constants[index];
                    }
                }
                if (row_index <
                    table->dimensions_space_charm_upgrade_infos.size()) {
                    for (int index = 0; index <= 4; ++index) {
                        table->dimensions_space_charm_upgrade_infos[row_index]
                            .amplification_const[index] = constants[index];
                    }
                }
                ++row_index;
            }
        } else if (tag == "[cost weights by rarity]") {
            for (int rarity = 0; rarity <= 5; ++rarity) {
                bool success = false;
                table->cost_rarity_weights[rarity] = ScanFloat(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importUpgradeTable",
                        0xB48,
                    "importUpgradeTable - invalid cost rarity table");
                }
            }
        } else if (tag == "[stat weights by rarity]") {
            for (int rarity = 0; rarity <= 5; ++rarity) {
                bool success = false;
                table->stat_rarity_weights[rarity] = ScanFloat(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importUpgradeTable",
                        0xB5A,
                    "importUpgradeTable - invalid stat rarity table");
                }
            }
        } else if (tag == "[type]") {
            for (int type = oldsrv::config::kOrdinaryEquipmentTypeFirst;
                 type <= oldsrv::config::kOrdinaryEquipmentTypeLast;
                 ++type) {
                bool success = false;
                table->equipment_type_weights[type] = ScanFloat(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importUpgradeTable",
                        0xB6C,
                    "importUpgradeTable - invalid type table");
                }
            }
        } else if (tag == "[level]") {
            for (int level = 0; level <= 3; ++level) {
                bool success = false;
                table->level_weights[level] = ScanFloat(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importUpgradeTable",
                        0xB7E,
                    "importUpgradeTable - invalid level table");
                }
            }
        } else if (tag == "[cost]") {
            for (int level = 0; level <= 99; ++level) {
                bool success = false;
                table->upgrade_costs[level] = ScanInt(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importUpgradeTable",
                        0xB90,
                        "importUpgradeTable - invalid gold table");
                }
            }
        } else if (tag == "[notice]") {
            bool success = false;
            table->notice_upgrade_level = ScanInt(&success);
            if (!success) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importUpgradeTable",
                    0xB9E,
                "importUpgradeTable - invalid notice tag");
            }
        } else if (tag == "[repair cost rate by upgrade level]") {
            for (int level = 0; level <= 99; ++level) {
                bool success = false;
                table->repair_cost_rates[level] = ScanFloat(&success);
                if (!success) {
                    if (level != 99) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptEtcParameter.cpp",
                            "importUpgradeTable",
                            0xBAC,
                            "ERROR : [repair cost rate by upgrade level] size %d / %d",
                            level,
                            99);
                    }
                    break;
                }
            }
        } else if (tag == "[destroy level by rarity]") {
            while (ScanStr(&value_text) == 1) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int rarity = getItemRarity(value_text.c_str());
                if (rarity == 6) {
                    return 0;
                }
                table->destroy_levels_by_rarity[rarity] = value;
            }
        } else if (tag == "[max upgrade level by rarity]") {
            while (ScanStr(&value_text) == 1) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int rarity = getItemRarity(value_text.c_str());
                if (rarity == 6) {
                    return 0;
                }
                table->max_upgrade_levels_by_rarity[rarity] = value;
            }
        } else if (tag == "[disjoint]") {
            for (;;) {
                if (ScanType(tag, true) != 1) {
                    throw static_cast<unsigned int>(3036);
                }
                if (tag == "[/disjoint]") {
                    break;
                }
                if (tag == "[disjoint bonus item]") {
                    bool success = false;
                    table->disjoint_bonus_item_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                } else if (tag == "[equip level const]") {
                    bool success = false;
                    table->disjoint_equip_level_const = ScanInt(&success);
                    if (!success) {
                        throw static_cast<unsigned int>(3050);
                    }
                } else if (
                    tag == "[upgrade failed bonus weight by rarity]") {
                    while (ScanStr(&value_text) == 1) {
                        bool success = false;
                        const float value = ScanFloat(&success);
                        if (!success) {
                            throw static_cast<unsigned int>(3060);
                        }
                        const int rarity =
                            getItemRarity(value_text.c_str());
                        if (rarity == 6) {
                            throw static_cast<unsigned int>(3063);
                        }
                        table->disjoint_failed_bonus_weights[rarity] = value;
                    }
                } else if (
                    tag == "[upgrade const for bonus item count]") {
                    bool success = false;
                    table->disjoint_bonus_count_const = ScanInt(&success);
                    if (!success) {
                        throw static_cast<unsigned int>(3071);
                    }
                } else if (tag == "[correction grade by rarity]") {
                    while (ScanStr(&value_text) == 1) {
                        bool success = false;
                        const int value = ScanInt(&success);
                        if (!success) {
                            throw static_cast<unsigned int>(3081);
                        }
                        const int rarity =
                            getItemRarity(value_text.c_str());
                        if (rarity == 6) {
                            throw static_cast<unsigned int>(3084);
                        }
                        table->disjoint_correction_grades[rarity] = value;
                    }
                }
            }
        } else if (tag == "[destruction result]") {
            for (int type = oldsrv::config::kOrdinaryEquipmentTypeFirst;
                 type <= oldsrv::config::kOrdinaryEquipmentTypeLast;
                 ++type) {
                bool success = false;
                table->destruction_result.equipment_type_rates[type] =
                    ScanFloat(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importUpgradeTable",
                        0xC26,
                        "importUpgradeTable - invalid type table");
                }
            }
            bool success = false;
            table->destruction_result.result_count = ScanInt(&success);
            if (!success) {
                break;
            }
            for (;;) {
                STResultItemCnt item;
                item.item_index = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                item.item_count = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                item.item_grade = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                table->destruction_result.result_items.push_back(item);
            }
        } else if (tag == "[cost weight by upgrade level]") {
            for (;;) {
                bool success = false;
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }
                const float weight = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                const std::pair<std::map<int, float>::iterator, bool> inserted =
                    table->cost_weights_by_upgrade_level.insert(
                        std::make_pair(level, weight));
                if (!inserted.second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importUpgradeTable",
                        0xC54,
                        "[cost weight by upgrade level] key is duplicated]");
                    return 0;
                }
            }
        } else if (tag == "[upgrade machine frame base time]") {
            for (int index = 0; index <= 3; ++index) {
                if (!ScanInt(&table->machine_frame_base_times[index])) {
                    return 0;
                }
            }
        } else if (tag == "[upgrade machine frame rate]") {
            upgrade_table_t::MachineFrameInfo info;
            int key = 0;
            while (ScanInt(&key)) {
                if (!ScanFloat(&info.first_rate) ||
                    !ScanFloat(&info.second_rate) ||
                    !ScanFloat(&info.third_rate) ||
                    !ScanFloat(&info.fourth_rate) ||
                    !ScanInt(&info.frame_type)) {
                    return 0;
                }
                table->machine_frame_infos.insert(std::make_pair(key, info));
            }
        }
    }
    return 1;
}
