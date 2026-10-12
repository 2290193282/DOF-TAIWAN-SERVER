#include "old_game/economy/equipment/item_amplifier_abi.h"

#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "infra/log/format/trace.h"

#include <algorithm>
#include <cstring>
#include <new>

extern "C++" unsigned int get_rand_int_global(int limit)
    __asm__("_Z12get_rand_inti");
namespace {

typedef int (*ItemTypeFn)(const CItem*);
typedef int (*ItemAmplifyVisibleFn)(const CItem*);

int ItemVtableType(const CItem* item) {
    void** vtable = *reinterpret_cast<void***>(const_cast<CItem*>(item));
    return reinterpret_cast<ItemTypeFn>(vtable[3])(item);
}

int ItemVtableAmplifyVisible(const CItem* item) {
    void** vtable = *reinterpret_cast<void***>(const_cast<CItem*>(item));
    return reinterpret_cast<ItemAmplifyVisibleFn>(vtable[22])(item);
}

uint16_t InvenItemEndurance(const Inven_Item& item) {
    return item.avatar_select_;
}

stAmplifyOption_t* InvenItemAmplifyOption(Inven_Item& item) {
    return &item.amplify_option_;
}

template <typename T, size_t N>
void SwapArray(T (&left)[N], T (&right)[N]) {
    for (size_t index = 0; index < N; ++index) {
        std::swap(left[index], right[index]);
    }
}

void SwapAmplifyItemScript(stAmplifyItemScript_t& left,
                           stAmplifyItemScript_t& right) {
    left.amplify_types.swap(right.amplify_types);
    left.option_infos.swap(right.option_infos);
    SwapArray(left.invest_rates, right.invest_rates);
    std::swap(left.invest_min_grade, right.invest_min_grade);
    SwapArray(left.reserved_40, right.reserved_40);
    SwapArray(left.purify_material_item_idx, right.purify_material_item_idx);
    SwapArray(left.purify_material_item_count,
              right.purify_material_item_count);
    for (size_t index = 0; index < 3; ++index) {
        left.invest_materials[index].swap(right.invest_materials[index]);
    }
    SwapArray(left.random_option_rates, right.random_option_rates);
}

void SwapUpgradeTable(upgrade_table_t& left, upgrade_table_t& right) {
    left.upgrade_infos.swap(right.upgrade_infos);
    SwapArray(left.level_weights, right.level_weights);
    SwapArray(left.equipment_type_weights, right.equipment_type_weights);
    SwapArray(left.cost_rarity_weights, right.cost_rarity_weights);
    SwapArray(left.stat_rarity_weights, right.stat_rarity_weights);
    SwapArray(left.upgrade_costs, right.upgrade_costs);
    std::swap(left.notice_upgrade_level, right.notice_upgrade_level);
    std::swap(left.disjoint_bonus_item_index,
              right.disjoint_bonus_item_index);
    std::swap(left.disjoint_equip_level_const,
              right.disjoint_equip_level_const);
    SwapArray(left.disjoint_failed_bonus_weights,
              right.disjoint_failed_bonus_weights);
    std::swap(left.disjoint_bonus_count_const,
              right.disjoint_bonus_count_const);
    SwapArray(left.disjoint_correction_grades,
              right.disjoint_correction_grades);
    SwapArray(left.repair_cost_rates, right.repair_cost_rates);
    SwapArray(left.destroy_levels_by_rarity,
              right.destroy_levels_by_rarity);
    SwapArray(left.max_upgrade_levels_by_rarity,
              right.max_upgrade_levels_by_rarity);
    SwapArray(left.destruction_result.equipment_type_rates,
              right.destruction_result.equipment_type_rates);
    std::swap(left.destruction_result.result_count,
              right.destruction_result.result_count);
    left.destruction_result.result_items.swap(
        right.destruction_result.result_items);
    left.cost_weights_by_upgrade_level.swap(
        right.cost_weights_by_upgrade_level);
    SwapArray(left.machine_frame_base_times,
              right.machine_frame_base_times);
    left.machine_frame_infos.swap(right.machine_frame_infos);
    left.pc_room_rental_upgrade_infos.swap(
        right.pc_room_rental_upgrade_infos);
    left.item_upgrade_cost_free_event_infos.swap(
        right.item_upgrade_cost_free_event_infos);
    left.charm_upgrade_infos.swap(right.charm_upgrade_infos);
    left.dimensions_space_charm_upgrade_infos.swap(
        right.dimensions_space_charm_upgrade_infos);
}

int AmplifyMaterialCount(const stInvestAmplifyOption_t& option) {
    int count = 0;
    std::memcpy(&count, &option.value, sizeof(count));
    return count;
}

enum {
    kMagiPurifyMaterialType = 2,
    kCeraPurifyMaterialType = 3,
    kMagiPurifyItemIndexOffset = 4,
    kMagiPurifyItemCountOffset = 8,
    kCeraPurifyItemIndexOffset = 12,
    kCeraPurifyItemCountOffset = 16,
};

int ReadAmplifierReservedInt(const stAmplifyItemScript_t& script,
                             size_t offset) {
    int value = 0;
    std::memcpy(&value, &script.reserved_40[offset], sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x82AA510
// IDA_NAME: _ZN21stAmplifyItemScript_tC2Ev
// DEMANGLED: stAmplifyItemScript_t::stAmplifyItemScript_t(void)
stAmplifyItemScript_t::stAmplifyItemScript_t()
    : amplify_types(), option_infos(), invest_rates(), invest_min_grade(0),
      reserved_40(), purify_material_item_idx(), purify_material_item_count(),
      invest_materials(), random_option_rates() {}

// IDA_EA: 0x82AA5CE
// IDA_NAME: _ZN21stAmplifyItemScript_tD2Ev
// DEMANGLED: stAmplifyItemScript_t::~stAmplifyItemScript_t()
stAmplifyItemScript_t::~stAmplifyItemScript_t() {}

// IDA_EA: 0x82AA65C
// IDA_NAME: _ZN14CItemAmplifierC2Ev
// DEMANGLED: CItemAmplifier::CItemAmplifier(void)
CItemAmplifier::CItemAmplifier()
    : stAmplifyItemScript_t(), upgrade_table_() {}

// IDA_EA: 0x8234980
// IDA_NAME: _ZNK14CItemAmplifier19checkInvestableItemEPK5CItemRK10Inven_Itemi
// DEMANGLED: CItemAmplifier::checkInvestableItem(CItem const*, Inven_Item const&, int)const
bool CItemAmplifier::checkInvestableItem(const CItem* item,
                                         const Inven_Item& inven_item,
                                         int charac_level) const {
    if (!item) return false;
    if (item->is_stackable()) return false;
    if (ItemVtableType(item) <
            oldsrv::config::kOrdinaryEquipmentTypeFirst ||
        ItemVtableType(item) >
            oldsrv::config::kOrdinaryEquipmentTypeLast) {
        return false;
    }
    if (ItemVtableType(item) == oldsrv::config::kEquipmentTypeTitle) {
        return false;
    }
    const int rarity = item->get_rarity();
    if (rarity < 2 || rarity > 5) return false;
    if (InvenItemEndurance(inven_item) <
        static_cast<const CEquipItem*>(item)->get_endurance())
        return false;
    CEquipItem* const equip_item =
        static_cast<CEquipItem*>(const_cast<CItem*>(item));
    if (charac_level && equip_item->isExistUpgradableLevelTag()) {
        if (charac_level <= 54) return false;
    } else if (item->getUsableLevel() <= 54) {
        return false;
    }
    const int grade_delta = item->get_grade() -
        upgrade_table_.disjoint_correction_grades[rarity];
    return std::max(0, grade_delta) >= invest_min_grade;
}

// IDA_EA: 0x8234B30
// IDA_NAME: _ZNK14CItemAmplifier19investAmplifyOptionEPK5CItemR10Inven_ItemRK23stInvestAmplifyOption_t
// DEMANGLED: CItemAmplifier::investAmplifyOption(CItem const*, Inven_Item&, stInvestAmplifyOption_t const&)const
char CItemAmplifier::investAmplifyOption(
    const CItem* item,
    Inven_Item& inven_item,
    const stInvestAmplifyOption_t& option) const {
    if (!item || item->get_rarity() < 2 || item->get_rarity() > 5 ||
        option.ability_type < 1 || option.ability_type > 5) {
        return 0;
    }
    for (std::vector<stAmplifyOptionInfo_t>::const_iterator current =
             option_infos.begin();
         current != option_infos.end();
         ++current) {
        if (current->ability_type == option.ability_type) {
            const uint16_t ability_value = static_cast<uint16_t>(
                invest_rates[item->get_rarity()] *
                static_cast<float>(current->value));
            return inven_item.amplify_option_._setAbility(
                       static_cast<uint8_t>(option.ability_type),
                       ability_value,
                       true)
                       ? 1
                       : 0;
        }
    }
    return 0;
}

// IDA_EA: 0x8234C3C
// IDA_NAME: _ZNK14CItemAmplifier25enableInvestAmplifyOptionERK10Inven_Item29ENUM_ITEM_AMPLIFY_INVEST_TYPE
// DEMANGLED: CItemAmplifier::enableInvestAmplifyOption(Inven_Item const&, ENUM_ITEM_AMPLIFY_INVEST_TYPE)const
int CItemAmplifier::enableInvestAmplifyOption(
    const Inven_Item& inven_item,
    ENUM_ITEM_AMPLIFY_INVEST_TYPE invest_type) const {
    switch (static_cast<int>(invest_type)) {
        case 0:
            if (inven_item.amplify_option_.hasAbility()) {
                return 20;
            }
            return inven_item.GetUpgrade() ? 18 : 0;
        case 1:
            if (!inven_item.amplify_option_.hasAbility()) {
                return 21;
            }
            return inven_item.GetUpgrade() ? 18 : 0;
        case 2:
            return 0;
        default:
            return 1;
    }
}

// IDA_EA: 0x8234D5E
// IDA_NAME: _ZNK14CItemAmplifier21getInvestMaterialInfoE29ENUM_ITEM_AMPLIFY_INVEST_TYPEmR23stInvestAmplifyOption_t
// DEMANGLED: CItemAmplifier::getInvestMaterialInfo(ENUM_ITEM_AMPLIFY_INVEST_TYPE, unsigned long, stInvestAmplifyOption_t&)const
int CItemAmplifier::getInvestMaterialInfo(
    ENUM_ITEM_AMPLIFY_INVEST_TYPE invest_type,
    unsigned long material_item_id,
    stInvestAmplifyOption_t& option) const {
    const int type = static_cast<int>(invest_type);
    if (type < 0 || type > 2) {
        return 0;
    }
    const __gnu_cxx::hash_map<unsigned long, stInvestAmplifyOption_t>::const_iterator
        found = invest_materials[type].find(material_item_id);
    if (found == invest_materials[type].end()) {
        return 0;
    }
    option = found->second;
    return 1;
}

// IDA_EA: 0x8376060
// IDA_NAME: _ZN14CItemAmplifier10loadScriptEPKcS1_
// DEMANGLED: CItemAmplifier::loadScript(char const*, char const*)
int CItemAmplifier::loadScript(const char* item_filename,
                               const char* upgrade_filename) {
    // Both PVF files form one runtime state.  Parse them into a temporary
    // owner so a failed second import cannot publish a half-filled object.
    CItemAmplifier staged;
    const int import_line = importAmplifyItem(item_filename, staged);
    if (import_line != 0) {
        cMyTrace trace(
            "bool CItemAmplifier::loadScript(const char*, const char*)",
            20, 0);
        trace("importAmplifyItem failed. line(%d)", import_line);
        return 0;
    }

    if (importUpgradeTable(upgrade_filename, &staged.upgrade_table_) != 1) {
        cMyTrace trace(
            "bool CItemAmplifier::loadScript(const char*, const char*)",
            20, 0);
        trace("importUpgradeTable failed. file(%s)", upgrade_filename);
        return 0;
    }
    if (!staged.isScriptReady()) {
        cMyTrace trace(
            "bool CItemAmplifier::loadScript(const char*, const char*)",
            20, 0);
        trace("amplify script is incomplete. item(%s) upgrade(%s)",
              item_filename, upgrade_filename);
        return 0;
    }

    swapLoadedState(staged);
    return 1;
}

bool CItemAmplifier::isScriptReady() const {
    if (option_infos.empty() || upgrade_table_.upgrade_infos.empty()) {
        return false;
    }

    bool abilities[5] = {false, false, false, false, false};
    uint16_t previous_rate = 0;
    for (std::vector<stAmplifyOptionInfo_t>::const_iterator current =
             option_infos.begin();
         current != option_infos.end(); ++current) {
        if (current->ability_type < 1 || current->ability_type > 4 ||
            current->value == 0 || current->rate <= previous_rate ||
            current->rate > 10000 || abilities[current->ability_type]) {
            return false;
        }
        abilities[current->ability_type] = true;
        previous_rate = current->rate;
    }
    for (int ability = 1; ability <= 4; ++ability) {
        if (!abilities[ability]) return false;
    }
    if (previous_rate != 10000) return false;

    int purify_item_ids[4] = {0, 0, 0, 0};
    for (int material_type = 0; material_type <= kCeraPurifyMaterialType;
         ++material_type) {
        purify_item_ids[material_type] =
            getPurifyMaterialItemIdx(material_type);
        const int material_count =
            getPurifyMaterialItemCount(material_type);
        // [purify only material] is the legacy fourth mapping.  Official
        // 0828 omits it, while derived 0828 PVFs may carry it together with
        // [purify only cera material].  Absence must be a complete zero pair.
        if (material_type == 1 && purify_item_ids[material_type] == 0 &&
            material_count == 0) {
            continue;
        }
        if (purify_item_ids[material_type] <= 0 ||
            material_count <= 0) {
            return false;
        }
        for (int previous = 0; previous < material_type; ++previous) {
            if (purify_item_ids[previous] > 0 &&
                purify_item_ids[previous] == purify_item_ids[material_type]) {
                return false;
            }
        }
    }

    for (int type = 0; type <= 2; ++type) {
        if (invest_materials[type].empty()) return false;
        bool material_abilities[6] = {false, false, false,
                                      false, false, false};
        for (__gnu_cxx::hash_map<unsigned long,
                                  stInvestAmplifyOption_t>::const_iterator
                 material = invest_materials[type].begin();
             material != invest_materials[type].end(); ++material) {
            if (material->first == 0 || material->second.ability_type < 1 ||
                material->second.ability_type > 5 ||
                AmplifyMaterialCount(material->second) <= 0 ||
                (type == 2 && material->second.ability_type != 5)) {
                return false;
            }
            material_abilities[material->second.ability_type] = true;
        }
        if (type != 2) {
            for (int ability = 1; ability <= 5; ++ability) {
                if (!material_abilities[ability]) return false;
            }
        }
    }
    return true;
}

void CItemAmplifier::swapLoadedState(CItemAmplifier& other) {
    SwapAmplifyItemScript(*this, other);
    SwapUpgradeTable(upgrade_table_, other.upgrade_table_);
}

// IDA_EA: 0x8514EBA
// IDA_NAME: _ZNK14CItemAmplifier19generateAmplifyItemEPK5CItemR10Inven_Item
// DEMANGLED: CItemAmplifier::generateAmplifyItem(CItem const*, Inven_Item&)const
stAmplifyOption_t* CItemAmplifier::generateAmplifyItem(const CItem* item,
                                                        Inven_Item& inven_item) const {
    ENUM_AMPLIFY_TYPE ability_type = ENUM_AMPLIFY_TYPE_NONE;
    uint16_t ability_value = 0;
    _generateOption(item, ability_type, ability_value);
    if (ability_type && ability_value) {
        return InvenItemAmplifyOption(inven_item)->_setAbility(
            static_cast<uint8_t>(ability_type), ability_value,
            ItemVtableAmplifyVisible(item) != 1);
    }
    return reinterpret_cast<stAmplifyOption_t*>(
        static_cast<uintptr_t>(ability_type));
}

// IDA_EA: 0x8514F70
// IDA_NAME: _ZNK14CItemAmplifier15_generateOptionEPK5CItemR17ENUM_AMPLIFY_TYPERt
// DEMANGLED: CItemAmplifier::_generateOption(CItem const*, ENUM_AMPLIFY_TYPE&, unsigned short&)const
char CItemAmplifier::_generateOption(const CItem* item,
                                     ENUM_AMPLIFY_TYPE& ability_type,
                                     uint16_t& ability_value) const {
    if (item->get_rarity() < 0 || item->get_rarity() > 5) {
        ability_type = ENUM_AMPLIFY_TYPE_NONE;
        ability_value = 0;
        return 0;
    }
    const int rarity = item->get_rarity();
    if (random_option_rates[rarity] <= get_rand_int_global(10000)) {
        ability_type = ENUM_AMPLIFY_TYPE_NONE;
        ability_value = 0;
        return 0;
    }
    const int random_option = get_rand_int_global(10000);
    for (std::vector<stAmplifyOptionInfo_t>::const_iterator it =
             option_infos.begin(); it != option_infos.end(); ++it) {
        if (it->rate > random_option) {
            ability_type = static_cast<ENUM_AMPLIFY_TYPE>(it->ability_type);
            ability_value = static_cast<uint16_t>(static_cast<int>(
                invest_rates[rarity] * static_cast<float>(it->value)));
            return 0;
        }
    }
    return 0;
}

// IDA_EA: 0x8234CD4
// IDA_NAME: _ZNK14CItemAmplifier24getPurifyMaterialItemIdxEi
// DEMANGLED: CItemAmplifier::getPurifyMaterialItemIdx(int)const
int CItemAmplifier::getPurifyMaterialItemIdx(int material_type) const {
    if (material_type < 0 || material_type > kCeraPurifyMaterialType) {
        return 0;
    }
    if (material_type == kMagiPurifyMaterialType) {
        return ReadAmplifierReservedInt(*this,
                                        kMagiPurifyItemIndexOffset);
    }
    if (material_type == kCeraPurifyMaterialType) {
        return ReadAmplifierReservedInt(*this,
                                        kCeraPurifyItemIndexOffset);
    }
    return purify_material_item_idx[material_type];
}

// IDA_EA: 0x8234CFA
// IDA_NAME: _ZNK14CItemAmplifier26getPurifyMaterialItemCountEi
// DEMANGLED: CItemAmplifier::getPurifyMaterialItemCount(int)const
int CItemAmplifier::getPurifyMaterialItemCount(int material_type) const {
    if (material_type < 0 || material_type > kCeraPurifyMaterialType) {
        return 0;
    }
    if (material_type == kMagiPurifyMaterialType) {
        return ReadAmplifierReservedInt(*this,
                                        kMagiPurifyItemCountOffset);
    }
    if (material_type == kCeraPurifyMaterialType) {
        return ReadAmplifierReservedInt(*this,
                                        kCeraPurifyItemCountOffset);
    }
    return purify_material_item_count[material_type];
}

// IDA_EA: 0x8234D20
// IDA_NAME: _ZNK14CItemAmplifier21getPurifyMaterialTypeEi
// DEMANGLED: CItemAmplifier::getPurifyMaterialType(int)const
int CItemAmplifier::getPurifyMaterialType(int item_index) const {
    if (item_index <= 0) {
        return -1;
    }
    for (int material_type = 0;
         material_type <= kCeraPurifyMaterialType;
         ++material_type) {
        if (getPurifyMaterialItemIdx(material_type) == item_index) {
            return material_type;
        }
    }
    return -1;
}

// IDA_EA: 0x854B560
// IDA_NAME: _ZNK14CItemAmplifier19getUpgradeInfoTableEv
// DEMANGLED: CItemAmplifier::getUpgradeInfoTable(void)const
upgrade_table_t* CItemAmplifier::getUpgradeInfoTable() const {
    return const_cast<upgrade_table_t*>(&upgrade_table_);
}

// IDA_EA: 0x851378A
// IDA_NAME: _ZN10CEquipItem25isExistUpgradableLevelTagEv
// DEMANGLED: CEquipItem::isExistUpgradableLevelTag(void)
bool CEquipItem::isExistUpgradableLevelTag() {
    return !upgrade_level_limits_.empty();
}
