#include "old_game/economy/equipment/old_equipment_stat_info.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

std::map<unsigned int, STEquipmentStatInfo> g_equipmentStatInfoMap;

typedef char g_equipmentStatInfoMap_Size_Must_Be_0x18[
    (sizeof(g_equipmentStatInfoMap) == 0x18) ? 1 : -1];

// IDA_EA: 0x898D25E
// IDA_NAME: _ZN19STEquipmentStatInfoC2Ev
// DEMANGLED: STEquipmentStatInfo::STEquipmentStatInfo(void)
STEquipmentStatInfo::STEquipmentStatInfo() {
    clear();
}

// IDA_EA: 0x898D272
// IDA_NAME: _ZN19STEquipmentStatInfo5clearEv
// DEMANGLED: STEquipmentStatInfo::clear(void)
bool STEquipmentStatInfo::clear() {
    hp_max = 0;
    mp_max = 0;
    physical_attack = 0;
    physical_defense = 0;
    magical_attack = 0;
    magical_defense = 0;
    equipment_physical_attack = 0;
    equipment_physical_defense = 0;
    equipment_magical_attack = 0;
    equipment_magical_defense = 0;
    for (int index = 0; index <= 3; ++index) {
        element_tolerance[index] = 0;
    }
    all_element_tolerance = 0;
    for (int index = 0; index <= 16; ++index) {
        active_property[index] = 0;
    }
    all_active_status_tolerance = 0;
    reserved_84 = 0;
    reserved_88 = 0;
    reserved_8C = 0;
    reserved_90 = 0;
    reserved_94 = 0;
    return false;
}

// IDA_EA: 0x8A58174
// IDA_NAME: _Z26importOldEquipmentStatInfoPKc
// DEMANGLED: importOldEquipmentStatInfo(char const*)
int importOldEquipmentStatInfo(const char* dir) {
    if (!loadRDARScriptFile(
            dir, "equipment/OldEquipmentStatInfoList.dat")) {
        LogManager::logFormat(
            0,
            "../RDARScriptOldEquip.cpp",
            "importOldEquipmentStatInfo",
            0x12,
            "CRITICAL ERROR : ");
        return 0;
    }

    while (true) {
        bool success = false;
        const unsigned int item_index =
            static_cast<unsigned int>(ScanInt(&success));
        if (!success) {
            return 1;
        }

        STEquipmentStatInfo stat_info;

        stat_info.hp_max = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x29,
                "Invalid Old Equipment Stat Info(missing hpMax)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.mp_max = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x30,
                "Invalid Old Equipment Stat Info(missing mpMax)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.physical_attack = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x37,
                "Invalid Old Equipment Stat Info(missing physical attack)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.physical_defense = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x3D,
                "Invalid Old Equipment Stat Info(missing physical defense)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.magical_attack = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x43,
                "Invalid Old Equipment Stat Info(missing magical attack)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.magical_defense = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x49,
                "Invalid Old Equipment Stat Info(missing magical defense)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.equipment_physical_attack = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x50,
                "Invalid Old Equipment Stat Info(missing equipment physical attack)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.equipment_physical_defense = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x57,
                "Invalid Old Equipment Stat Info(missing equipment physical defense)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.equipment_magical_attack = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x5E,
                "Invalid Old Equipment Stat Info(missing magical attack)! itemIndex = %d",
                item_index);
            return 0;
        }

        stat_info.equipment_magical_defense = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x65,
                "Invalid Old Equipment Stat Info(missing magical defense)! itemIndex = %d",
                item_index);
            return 0;
        }

        for (int index = 0; index <= 3; ++index) {
            stat_info.element_tolerance[index] = ScanInt(&success);
            if (!success) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptOldEquip.cpp",
                    "importOldEquipmentStatInfo",
                    0x6E,
                    "Invalid Old Equipment Stat Info(missing element tolerance %d)! itemIndex = %d",
                    index,
                    item_index);
                return 0;
            }
        }

        stat_info.all_element_tolerance = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x76,
                "Invalid Old Equipment Stat Info(missing all element tolerance)! itemIndex = %d",
                item_index);
            return 0;
        }

        for (int index = 0; index <= 16; ++index) {
            stat_info.active_property[index] = ScanInt(&success);
            if (!success) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptOldEquip.cpp",
                    "importOldEquipmentStatInfo",
                    0x7F,
                    "Invalid Old Equipment Stat Info(missing active property %d)! itemIndex = %d",
                    index,
                    item_index);
                return 0;
            }
        }

        stat_info.all_active_status_tolerance = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x87,
                "Invalid Old Equipment Stat Info(missing all activeStatus tolerance)! itemIndex = %d",
                item_index);
            return 0;
        }

        if (g_equipmentStatInfoMap.find(item_index) ==
            g_equipmentStatInfoMap.end()) {
            g_equipmentStatInfoMap.insert(
                std::make_pair(item_index, stat_info));
        } else {
            LogManager::logFormat(
                0,
                "../RDARScriptOldEquip.cpp",
                "importOldEquipmentStatInfo",
                0x92,
                "Redundant Old Equipment Stat Info! itemIndex = %d",
                item_index);
        }
    }
}
