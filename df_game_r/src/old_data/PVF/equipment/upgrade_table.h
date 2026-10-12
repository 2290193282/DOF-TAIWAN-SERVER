#ifndef OLDSRV_OLD_DATA_PVF_EQUIPMENT_UPGRADE_TABLE_H_
#define OLDSRV_OLD_DATA_PVF_EQUIPMENT_UPGRADE_TABLE_H_

#include <stddef.h>
#include <stdint.h>

#include "common/config.h"

#include <map>
#include <tr1/unordered_map>
#include <vector>

// These two enum names are part of the exported old-server method ABI.  The
// restored functions only use their verified integer domains: rarity 0..5 and
// equipment type 11..22.
enum ENUM_RARITY {
    ENUM_RARITY_NONE = 0,
};

enum ENUM_EQUIPMENTTYPE {
    ENUM_EQUIPMENTTYPE_NONE = 0,
};

// IDA_EA: 0x854B282, 0x89378CE.
struct upgrade_info_t {
    upgrade_info_t();
    ~upgrade_info_t();

    float stat_values[4];
    float stat_value_04;
    float stat_values_05[3];
    int parsed_values_08[3];
    int event_material_item_index;
    int event_material_item_count;
    int calculated_cost;
    int amplification_const[5];
    float chronicle_stat_values[4];
    float chronicle_stat_value_23;
    float chronicle_stat_values_24[2];
    float chronicle_stat_value_26;
};

// IDA_EA: 0x891A8C0, 0x8937B14.
struct STResultItemCnt {
    STResultItemCnt();
    ~STResultItemCnt();

    uint16_t item_index;
    uint16_t item_count;
    uint16_t item_grade;
};

// IDA_EA: 0x891A8E0, 0x854B3A4.
struct ST3rdChronicleDestructionResult {
    ST3rdChronicleDestructionResult();
    ~ST3rdChronicleDestructionResult();

    float equipment_type_rates[
        oldsrv::config::kOrdinaryEquipmentTypeLast + 1];
    int result_count;
    std::vector<STResultItemCnt> result_items;
};

// IDA_EA: 0x89000CA, 0x854B3F6, 0x89003EE, 0x8900436.
struct upgrade_table_t {
    struct MachineFrameInfo {
        MachineFrameInfo();

        float first_rate;
        float second_rate;
        float third_rate;
        float fourth_rate;
        int frame_type;
    };

    upgrade_table_t();
    ~upgrade_table_t();

    int GetPenaltyType(uint8_t upgrade,
                       ENUM_RARITY rarity,
                       const upgrade_info_t* info) const;
    int get_upgrade_info(upgrade_info_t* output,
                         uint8_t rarity,
                         uint8_t growth_grade,
                         uint8_t upgrade,
                         ENUM_EQUIPMENTTYPE equipment_type) const;
    int get_pc_room_rental_upgrade_info(
        upgrade_info_t* output,
        uint8_t rarity,
        uint8_t growth_grade,
        uint8_t upgrade,
        ENUM_EQUIPMENTTYPE equipment_type) const;
    // This selects the event-specific reinforcement values. Whether gold is
    // charged is controlled separately by the event script's [need gold].
    int get_item_upgrade_cost_free_event_info(
        upgrade_info_t* output,
        uint8_t rarity,
        uint8_t growth_grade,
        uint8_t upgrade,
        ENUM_EQUIPMENTTYPE equipment_type) const;
    int get_charm_upgrade_info(upgrade_info_t* output,
                               uint8_t rarity,
                               uint8_t growth_grade,
                               uint8_t upgrade,
                               ENUM_EQUIPMENTTYPE equipment_type) const;
    int get_dimensions_space_charm_upgrade_info(
        upgrade_info_t* output,
        uint8_t rarity,
        uint8_t growth_grade,
        uint8_t upgrade,
        ENUM_EQUIPMENTTYPE equipment_type) const;

    std::vector<upgrade_info_t> upgrade_infos;
    float level_weights[4];
    float equipment_type_weights[
        oldsrv::config::kOrdinaryEquipmentTypeLast + 1];
    float cost_rarity_weights[6];
    float stat_rarity_weights[6];
    int upgrade_costs[100];
    int notice_upgrade_level;
    int disjoint_bonus_item_index;
    int disjoint_equip_level_const;
    float disjoint_failed_bonus_weights[6];
    int disjoint_bonus_count_const;
    int disjoint_correction_grades[6];
    float repair_cost_rates[100];
    int destroy_levels_by_rarity[6];
    int max_upgrade_levels_by_rarity[6];
    ST3rdChronicleDestructionResult destruction_result;
    std::map<int, float> cost_weights_by_upgrade_level;
    int machine_frame_base_times[4];
    // The native equipment table places this legacy hash table at 0x4CC and
    // the subobject ends at 0x4EC. On the G++ 4.9 / i386 target this is the
    // TR1 hash-table ABI (0x20); the C++11 unordered_map ABI is only 0x1C.
    std::tr1::unordered_map<int, MachineFrameInfo> machine_frame_infos;

    // Target PVF [table type] containers. The original normal/amplify table
    // remains in upgrade_infos; these four vectors preserve the additional
    // item-specific tables instead of projecting them onto the normal table.
    std::vector<upgrade_info_t> pc_room_rental_upgrade_infos;
    std::vector<upgrade_info_t> item_upgrade_cost_free_event_infos;
    std::vector<upgrade_info_t> charm_upgrade_infos;
    std::vector<upgrade_info_t> dimensions_space_charm_upgrade_infos;

private:
    int get_upgrade_info_from_table(
        const std::vector<upgrade_info_t>& source,
        upgrade_info_t* output,
        uint8_t rarity,
        uint8_t growth_grade,
        uint8_t upgrade,
        ENUM_EQUIPMENTTYPE equipment_type) const;
};

namespace WongWork {

// IDA_EA: 0x854B382, 0x854B38A, 0x854B496, 0x854B4AA.
class CItemUpgradeTable : public upgrade_table_t {
public:
    CItemUpgradeTable();
    ~CItemUpgradeTable();

    CItemUpgradeTable* getUpgradeInfoTable() const;
    int loadUpgradeScript(const char* filename);
};

}  // namespace WongWork

// IDA_EA: 0x8900C26.
int importUpgradeTable(const char* filename, upgrade_table_t* table);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(ENUM_RARITY) == 0x04u,
              "ENUM_RARITY old ABI width must stay 4");
static_assert(sizeof(ENUM_EQUIPMENTTYPE) == 0x04u,
              "ENUM_EQUIPMENTTYPE old ABI width must stay 4");
static_assert(sizeof(upgrade_info_t) == 0x6Cu,
              "upgrade_info_t IDA ABI size must stay 0x6C");
static_assert(offsetof(upgrade_info_t, parsed_values_08) == 0x20u,
              "upgrade probability fields offset must stay 0x20");
static_assert(offsetof(upgrade_info_t, event_material_item_index) == 0x2Cu,
              "event material index offset must stay 0x2C");
static_assert(offsetof(upgrade_info_t, event_material_item_count) == 0x30u,
              "event material count offset must stay 0x30");
static_assert(offsetof(upgrade_info_t, calculated_cost) == 0x34u,
              "calculated upgrade cost offset must stay 0x34");
static_assert(sizeof(STResultItemCnt) == 0x06u,
              "STResultItemCnt IDA ABI size must stay 0x06");
static_assert(sizeof(ST3rdChronicleDestructionResult) == 0x6Cu,
              "ST3rdChronicleDestructionResult ABI size must stay 0x6C");
static_assert(
    offsetof(ST3rdChronicleDestructionResult, result_count) == 0x5Cu,
    "3rd Chronicle destruction result index offset must stay 0x5C");
static_assert(
    offsetof(ST3rdChronicleDestructionResult, result_items) == 0x60u,
    "3rd Chronicle destruction rows offset must stay 0x60");
static_assert(sizeof(upgrade_table_t::MachineFrameInfo) == 0x14u,
              "MachineFrameInfo IDA ABI size must stay 0x14");
static_assert(offsetof(upgrade_table_t, disjoint_bonus_item_index) == 0x23Cu,
              "disjoint bonus item offset must stay 0x23C");
static_assert(offsetof(upgrade_table_t, disjoint_equip_level_const) == 0x240u,
              "disjoint equipment-level constant offset must stay 0x240");
static_assert(
    offsetof(upgrade_table_t, disjoint_failed_bonus_weights) == 0x244u,
    "disjoint failed bonus weights offset must stay 0x244");
static_assert(offsetof(upgrade_table_t, disjoint_bonus_count_const) == 0x25Cu,
              "disjoint bonus count constant offset must stay 0x25C");
static_assert(offsetof(upgrade_table_t, disjoint_correction_grades) == 0x260u,
              "disjoint correction grades offset must stay 0x260");
static_assert(offsetof(upgrade_table_t, destruction_result) == 0x438u,
              "destruction result offset must stay 0x438");
static_assert(offsetof(upgrade_table_t, cost_weights_by_upgrade_level) ==
                  0x4A4u,
              "cost weight map offset must stay 0x4A4");
static_assert(offsetof(upgrade_table_t, machine_frame_infos) == 0x4CCu,
              "machine frame map offset must stay 0x4CC");
static_assert(offsetof(upgrade_table_t, pc_room_rental_upgrade_infos) ==
                  0x4ECu,
              "PC-room rental table must follow the restored prefix");
static_assert(
    offsetof(upgrade_table_t, item_upgrade_cost_free_event_infos) == 0x4F8u,
    "cost-free event table offset must stay stable");
static_assert(offsetof(upgrade_table_t, charm_upgrade_infos) == 0x504u,
              "charm upgrade table offset must stay stable");
static_assert(
    offsetof(upgrade_table_t, dimensions_space_charm_upgrade_infos) ==
        0x510u,
    "dimensions-space charm table offset must stay stable");
static_assert(sizeof(upgrade_table_t) == 0x51Cu,
              "typed upgrade_table_t size must stay 0x51C");
static_assert(sizeof(WongWork::CItemUpgradeTable) == 0x51Cu,
              "typed CItemUpgradeTable size must stay 0x51C");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_DATA_PVF_EQUIPMENT_UPGRADE_TABLE_H_
