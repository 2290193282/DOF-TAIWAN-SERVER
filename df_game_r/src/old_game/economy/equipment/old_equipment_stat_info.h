#ifndef OLDSRV_GLOBAL_OLD_EQUIPMENT_STAT_INFO_H_
#define OLDSRV_GLOBAL_OLD_EQUIPMENT_STAT_INFO_H_

#include <stdint.h>

#include <map>

// IDA unit: 06_userinfo_stat_inventory/STEquipmentStatInfo.
// IDA_EA: STEquipmentStatInfo ctor/clear @0x898D25E/0x898D272;
// importOldEquipmentStatInfo @0x8A58174.
// Public value record consumed by the 99-owned OldEquipmentStatInfo importer.
struct STEquipmentStatInfo {
    STEquipmentStatInfo();

    bool clear();

    int32_t hp_max;                       // +0x00
    int32_t mp_max;                       // +0x04
    int32_t physical_attack;              // +0x08
    int32_t physical_defense;             // +0x0C
    int32_t magical_attack;               // +0x10
    int32_t magical_defense;              // +0x14
    int32_t equipment_physical_attack;    // +0x18
    int32_t equipment_physical_defense;   // +0x1C
    int32_t equipment_magical_attack;     // +0x20
    int32_t equipment_magical_defense;    // +0x24
    int32_t element_tolerance[4];         // +0x28
    int32_t all_element_tolerance;        // +0x38
    int32_t active_property[17];          // +0x3C
    int32_t all_active_status_tolerance;  // +0x80
    int32_t reserved_84;
    int32_t reserved_88;
    int32_t reserved_8C;
    int32_t reserved_90;
    int32_t reserved_94;
};

static_assert(sizeof(STEquipmentStatInfo) == 0x98u,
              "STEquipmentStatInfo IDA ABI size must stay 0x98");

extern std::map<unsigned int, STEquipmentStatInfo> g_equipmentStatInfoMap;

int importOldEquipmentStatInfo(const char* dir);

#endif  // OLDSRV_GLOBAL_OLD_EQUIPMENT_STAT_INFO_H_
