#include "old_game/economy/inventory/inventory.h"

#include "common/config.h"

#include <algorithm>
#include <iterator>
#include <new>
#include <stdint.h>
#include <cstring>
#include <ctime>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/types/code_page.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/creature/creature_script.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "old_data/PVF/etc/cargo_transport_stone_script.h"
#include "old_data/PVF/etc/compound_creature_script.h"
#include "old_data/PVF/etc/equipment_growth.h"
#include "old_data/PVF/stackable/stackable_script.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/arad_item_info.h"
#include "old_game/economy/inventory/random_item_table.h"
#include "old_game/economy/inventory/recipe_info.h"
#include "common/manager/mt_rand.h"
#include "common/manager/system_time.h"

namespace {

template <typename T>
T& At(void* object, size_t offset) {
    return *reinterpret_cast<T*>(
        static_cast<unsigned char*>(object) + offset);
}

template <typename T>
const T& At(const void* object, size_t offset) {
    return *reinterpret_cast<const T*>(
        static_cast<const unsigned char*>(object) + offset);
}

uint8_t& ByteAt(void* object, size_t offset) {
    return *(static_cast<uint8_t*>(object) + offset);
}

int& IntAt(void* object, size_t offset) {
    return At<int>(object, offset);
}

int CountValidAvatarAbilityOptions(
    const std::vector<STAvatarAbilityVariation>& variations) {
    int count = 0;
    for (std::vector<STAvatarAbilityVariation>::const_iterator it =
             variations.begin();
         it != variations.end(); ++it) {
        if (it->ability_index >= 0) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x89BACC1.  The source format is two scalar values followed by
// (item, weight, count) triples.  The original routine owns the target clear.
bool CreateStackableLotteryInfo(LotteryInfo* info,
                                const std::vector<int>& values) {
    info->clear();
    if (values.size() < 2u || ((values.size() - 2u) % 3u) != 0u ||
        values[0] < 0 || values[1] <= 0) {
        return false;
    }
    info->first_dword = static_cast<uint32_t>(values[0]);
    info->second_dword = static_cast<uint32_t>(values[1]);
    for (size_t index = 2u; index + 2u < values.size(); index += 3u) {
        // Item zero is the official gold branch.  A small set of target PVF
        // rows also carries signed item-number bit patterns, so validity here
        // is the grammar (positive weight/count), not an invented ID range.
        if (values[index + 1u] <= 0 || values[index + 2u] <= 0) {
            info->clear();
            return false;
        }
        info->pair_values.push_back(std::make_pair(values[index],
                                                   values[index + 1u]));
        info->int_values.push_back(values[index + 2u]);
    }
    return true;
}

// IDA_EA: 0x89BAE94.  The PVF vector is an exact sequence of
// (effect-type, effect-value-per-10000, duration-seconds) triples.
int CreateGlobalEffectInfo(std::vector<stGlobalEffect_t>* effects,
                           const std::vector<int>& values) {
    effects->clear();
    if ((values.size() % 3u) != 0u) return 0;
    for (size_t index = 0u; index < values.size(); index += 3u) {
        stGlobalEffect_t effect;
        effect.effect_type = static_cast<uint32_t>(values[index]);
        effect.effect_value = static_cast<float>(values[index + 1u]) / 10000.0f;
        effect.duration_seconds = static_cast<uint32_t>(values[index + 2u]);
        effects->push_back(effect);
    }
    return 1;
}

// IDA_EA: 0x89BAFD1.  This preserves the original nested-vector conversion
// and the non-obvious order of the four default-item fields.
bool CreateStackableMultiBoxLotteryInfo(
    MultiBoxLotteryInfo* info,
    const std::vector<std::vector<int> >& groups,
    const std::vector<int>& extra_values,
    const std::vector<int>& defaults) {
    info->clear();
    if (groups.empty() || extra_values.size() != 2u ||
        !((extra_values[0] > 0 && extra_values[1] > 0) ||
          (extra_values[0] == -1 && extra_values[1] == 0))) {
        return false;
    }
    for (std::vector<std::vector<int> >::const_iterator group = groups.begin();
         group != groups.end();
         ++group) {
        if (group->size() < 4u || ((group->size() - 4u) % 4u) != 0u ||
            (*group)[0] <= 0 || (*group)[1] < 0 || (*group)[2] <= 0) {
            info->clear();
            return false;
        }
        MultiBoxLotteryInfo::LotteryUnit unit;
        unit.clear();
        unit.first_dword = static_cast<uint32_t>((*group)[0]);
        unit.second_dword = static_cast<uint32_t>((*group)[1]);
        unit.third_dword = static_cast<uint32_t>((*group)[2]);
        unit.flag = (*group)[3] != 0;
        for (size_t index = 4u; index + 3u < group->size(); index += 4u) {
            // Zero-rate entries exist in 0828 randomcerabox_1.stk and are
            // deliberately unreachable, not malformed.
            if ((*group)[index] <= 0 || (*group)[index + 1u] < 0 ||
                (*group)[index + 2u] <= 0) {
                info->clear();
                return false;
            }
            unit.first_pairs.push_back(
                std::make_pair((*group)[index], (*group)[index + 1u]));
            unit.second_pairs.push_back(
                std::make_pair((*group)[index + 2u], (*group)[index + 3u]));
        }
        info->lottery_units.push_back(unit);
    }
    info->third_dword = extra_values[0];
    info->fourth_dword = static_cast<uint32_t>(extra_values[1]);
    if (defaults.empty()) return true;
    if (defaults.size() < 4u || defaults[0] <= 0 || defaults[1] < 0 ||
        defaults[2] <= 0) {
        info->clear();
        return false;
    }
    MultiBoxLotteryInfo::stDefaultItemInfo& default_info =
        info->default_item_info;
    default_info.first_dword = static_cast<uint32_t>(defaults[0]);
    default_info.second_dword = static_cast<uint32_t>(defaults[1]);
    default_info.third_dword = static_cast<uint32_t>(defaults[2]);
    default_info.flag = defaults[3] != 0;
    // 140109_rightnow_box has one trailing three-value default candidate.
    // The 0627 iterator stops before that incomplete record; preserve that
    // target-data behavior while still validating every complete record.
    for (size_t index = 4u; index + 3u < defaults.size(); index += 4u) {
        if (defaults[index] <= 0 || defaults[index + 1u] < 0 ||
            defaults[index + 2u] <= 0) {
            info->clear();
            return false;
        }
        MultiBoxLotteryInfo::stDefaultItem item;
        item.first_dword = static_cast<uint32_t>(defaults[index]);
        item.third_dword = static_cast<uint32_t>(defaults[index + 1u]);
        item.second_dword = static_cast<uint32_t>(defaults[index + 2u]);
        item.flag = defaults[index + 3u] != 0;
        default_info.default_items.push_back(item);
    }
    return true;
}

// IDA_EA: 0x8514218 / 0x8514B9A / 0x85146DE
// These concrete slot functions implement each old deleting destructor while
// preserving the manually recovered CItemVTable layout.
extern "C" void CItemDeletingDtor(CItem*);
extern "C" void CEquipItemDeletingDtor(CEquipItem*);
extern "C" void CStackableItemDeletingDtor(CStackableItem*);
extern "C" void __cxa_pure_virtual();

typedef CItemVTable::Entry VTableEntry;

#define OLDSRV_VTABLE_ENTRY(function) reinterpret_cast<VTableEntry>(function)

// IDA_EA: 0x85141EE
// The old vtable stores the complete-object D2 entry.  C++ does not permit
// taking a destructor address, so this typed slot invokes the real member
// destructor without performing the D0 delete operation.
void CItemBaseDtor(const CItem* item) {
    const_cast<CItem*>(item)->~CItem();
}

// IDA_EA: 0x8514B70
void CEquipItemBaseDtor(const CItem* item) {
    const_cast<CEquipItem*>(static_cast<const CEquipItem*>(item))
        ->~CEquipItem();
}

// IDA_EA: 0x85146B4
void CStackableItemBaseDtor(const CItem* item) {
    const_cast<CStackableItem*>(static_cast<const CStackableItem*>(item))
        ->~CStackableItem();
}

void PureVirtualMakeItem(const CItem*, Inven_Item&) { __cxa_pure_virtual(); }
int PureVirtualInt(const CItem*) {
    __cxa_pure_virtual();
    return 0;
}
bool PureVirtualBool(const CItem*) {
    __cxa_pure_virtual();
    return false;
}
CItem* PureVirtualSetParentBoosterItemId(CItem*, unsigned long) {
    __cxa_pure_virtual();
    return 0;
}

int CItemImpossibleUpgradeSlot(const CItem* item) {
    return item->IsImpossibleUpgrade();
}
int CItemImpossibleAmplifySlot(const CItem* item) {
    return item->IsImpossibleAmplify();
}
int CItemImpossibleDisjointSlot(const CItem* item) {
    return item->IsImpossibleDisjoint();
}
int CItemImpossibleGiftSlot(const CItem* item) { return item->IsImpossibleGift(); }
int CItemImpossibleCargoSlot(const CItem* item) {
    return item->IsImpossibleCharacCargo();
}
int CItemKiriProtectionSlot(const CItem* item) {
    return item->IsPossibleKiriProtection();
}
int CItemPeriodAfterUnsealingSlot(const CItem* item) {
    return item->PeriodAfterUnsealing();
}
int CItemRandomOptionSlot(const CItem* item) { return item->IsRandomOption(); }
int CItemRandomOptionGradeSlot(const CItem* item) {
    return item->GetRandomOptionGrade();
}
int CItemRandomOptionUnsealSlot(const CItem* item) {
    return item->IsRandomOptionUnseal();
}
int CItemAmplifyPollutionSlot(const CItem* item) {
    return item->isAmplifyPollutionItem();
}
int CItemCheckItemUpgradePossibleSlot(const CItem* item) {
    return item->checkItemUpgradePossible();
}

void CEquipItemMakeItemSlot(const CItem* item, Inven_Item& inventory_item) {
    static_cast<const CEquipItem*>(item)->make_item(inventory_item);
}
int CEquipItemTypeSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->GetItemType();
}
bool CEquipItemAvatarSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsAvatarItem() != 0;
}
bool CEquipItemCreatureSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsCreatureItem() != 0;
}
int CEquipItemClearAvatarSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsClearAvatar();
}
int CEquipItemEnchantableSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsEnchantalbe();
}
int CEquipItemUsableTypeSlot(const CItem* item, int item_type) {
    return static_cast<const CEquipItem*>(item)->getUsableItemType(item_type);
}
bool CEquipItemAvatarFuncSlot(const CItem* item, ENUM_AVATAR_FUNC_FILTER filter) {
    return static_cast<const CEquipItem*>(item)->isEnableAvatarFunc(filter);
}
int CEquipItemGuildPowerSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->GetGuildPowerWarPoint();
}
int CEquipItemUserPowerSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->GetUserPowerWarPoint();
}
int CEquipItemImpossibleUpgradeSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsImpossibleUpgrade();
}
int CEquipItemImpossibleAmplifySlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsImpossibleAmplify();
}
int CEquipItemImpossibleDisjointSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsImpossibleDisjoint();
}
int CEquipItemImpossibleGiftSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsImpossibleGift();
}
int CEquipItemRandomOptionSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->IsRandomOption();
}
int CEquipItemRandomOptionGradeSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->GetRandomOptionGrade();
}
int CEquipItemAmplifyPollutionSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->isAmplifyPollutionItem();
}
int CEquipItemCheckItemUpgradePossibleSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->checkItemUpgradePossible();
}
CItem* CEquipItemSetParentBoosterItemIdSlot(CItem* item,
                                            unsigned long item_id) {
    return static_cast<CEquipItem*>(item)->set_parent_booster_item_id(item_id);
}
int CEquipItemGetParentBoosterItemIdSlot(const CItem* item) {
    return static_cast<const CEquipItem*>(item)->get_parent_booster_item_id();
}

void CStackableItemMakeItemSlot(const CItem* item, Inven_Item& inventory_item) {
    static_cast<const CStackableItem*>(item)->make_item(inventory_item);
}
int CStackableItemTypeSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->GetItemType();
}
bool CStackableItemAvatarSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->IsAvatarItem() != 0;
}
bool CStackableItemCreatureSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->IsCreatureItem() != 0;
}
int CStackableItemClearAvatarSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->IsClearAvatar();
}
int CStackableItemEnchantableSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->IsEnchantalbe();
}
bool CStackableItemUsableTypeSlot(const CItem* item, int item_type) {
    return static_cast<const CStackableItem*>(item)->getUsableItemType(item_type);
}
bool CStackableItemAvatarFuncSlot(const CItem* item,
                                  ENUM_AVATAR_FUNC_FILTER filter) {
    return static_cast<const CStackableItem*>(item)->isEnableAvatarFunc(filter);
}
int CStackableItemGuildPowerSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->GetGuildPowerWarPoint();
}
int CStackableItemUserPowerSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->GetUserPowerWarPoint();
}
int CStackableItemImpossibleGiftSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->IsImpossibleGift();
}
int CStackableItemRandomOptionUnsealSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)->IsRandomOptionUnseal();
}
CItem* CStackableItemSetParentBoosterItemIdSlot(CItem* item,
                                                unsigned long item_id) {
    return static_cast<CStackableItem*>(item)->set_parent_booster_item_id(
        item_id);
}
int CStackableItemGetParentBoosterItemIdSlot(const CItem* item) {
    return static_cast<const CStackableItem*>(item)
        ->get_parent_booster_item_id();
}

const CItemVTable kCItemVTable = {
    OLDSRV_VTABLE_ENTRY(&CItemBaseDtor),
    OLDSRV_VTABLE_ENTRY(&CItemDeletingDtor),
    &PureVirtualMakeItem,
    &PureVirtualInt,
    &PureVirtualBool,
    &PureVirtualBool,
    {
        OLDSRV_VTABLE_ENTRY(&__cxa_pure_virtual),
        OLDSRV_VTABLE_ENTRY(&__cxa_pure_virtual),
        OLDSRV_VTABLE_ENTRY(&__cxa_pure_virtual),
        OLDSRV_VTABLE_ENTRY(&__cxa_pure_virtual),
    },
    &PureVirtualInt,
    &PureVirtualInt,
    &CItemImpossibleUpgradeSlot,
    &CItemImpossibleAmplifySlot,
    &CItemImpossibleDisjointSlot,
    &CItemImpossibleGiftSlot,
    &CItemImpossibleCargoSlot,
    &CItemKiriProtectionSlot,
    &CItemPeriodAfterUnsealingSlot,
    &CItemRandomOptionSlot,
    &CItemRandomOptionGradeSlot,
    &CItemRandomOptionUnsealSlot,
    &CItemAmplifyPollutionSlot,
    &CItemCheckItemUpgradePossibleSlot,
    &PureVirtualSetParentBoosterItemId,
    &PureVirtualInt,
};

const CItemVTable kCEquipItemVTable = {
    OLDSRV_VTABLE_ENTRY(&CEquipItemBaseDtor),
    OLDSRV_VTABLE_ENTRY(&CEquipItemDeletingDtor),
    &CEquipItemMakeItemSlot,
    &CEquipItemTypeSlot,
    &CEquipItemAvatarSlot,
    &CEquipItemCreatureSlot,
    {
        OLDSRV_VTABLE_ENTRY(&CEquipItemClearAvatarSlot),
        OLDSRV_VTABLE_ENTRY(&CEquipItemEnchantableSlot),
        OLDSRV_VTABLE_ENTRY(&CEquipItemUsableTypeSlot),
        OLDSRV_VTABLE_ENTRY(&CEquipItemAvatarFuncSlot),
    },
    &CEquipItemGuildPowerSlot,
    &CEquipItemUserPowerSlot,
    &CEquipItemImpossibleUpgradeSlot,
    &CEquipItemImpossibleAmplifySlot,
    &CEquipItemImpossibleDisjointSlot,
    &CEquipItemImpossibleGiftSlot,
    &CItemImpossibleCargoSlot,
    &CItemKiriProtectionSlot,
    &CItemPeriodAfterUnsealingSlot,
    &CEquipItemRandomOptionSlot,
    &CEquipItemRandomOptionGradeSlot,
    &CItemRandomOptionUnsealSlot,
    &CEquipItemAmplifyPollutionSlot,
    &CEquipItemCheckItemUpgradePossibleSlot,
    &CEquipItemSetParentBoosterItemIdSlot,
    &CEquipItemGetParentBoosterItemIdSlot,
};

const CItemVTable kCStackableItemVTable = {
    OLDSRV_VTABLE_ENTRY(&CStackableItemBaseDtor),
    OLDSRV_VTABLE_ENTRY(&CStackableItemDeletingDtor),
    &CStackableItemMakeItemSlot,
    &CStackableItemTypeSlot,
    &CStackableItemAvatarSlot,
    &CStackableItemCreatureSlot,
    {
        OLDSRV_VTABLE_ENTRY(&CStackableItemClearAvatarSlot),
        OLDSRV_VTABLE_ENTRY(&CStackableItemEnchantableSlot),
        OLDSRV_VTABLE_ENTRY(&CStackableItemUsableTypeSlot),
        OLDSRV_VTABLE_ENTRY(&CStackableItemAvatarFuncSlot),
    },
    &CStackableItemGuildPowerSlot,
    &CStackableItemUserPowerSlot,
    &CItemImpossibleUpgradeSlot,
    &CItemImpossibleAmplifySlot,
    &CItemImpossibleDisjointSlot,
    &CStackableItemImpossibleGiftSlot,
    &CItemImpossibleCargoSlot,
    &CItemKiriProtectionSlot,
    &CItemPeriodAfterUnsealingSlot,
    &CItemRandomOptionSlot,
    &CItemRandomOptionGradeSlot,
    &CStackableItemRandomOptionUnsealSlot,
    &CItemAmplifyPollutionSlot,
    &CItemCheckItemUpgradePossibleSlot,
    &CStackableItemSetParentBoosterItemIdSlot,
    &CStackableItemGetParentBoosterItemIdSlot,
};

#undef OLDSRV_VTABLE_ENTRY

}  // namespace

// IDA_EA: 0x88947D4 / 0x889246C / 0x88947E2.
CItemGloballyUniqueIdentifierGenerator
    g_ItemGloballyUniqueIdentifierGenerator;

CItemGloballyUniqueIdentifierGenerator::CItemGloballyUniqueIdentifierGenerator()
    : sequence_(0) {}

itemGloballyUniqueIdentifier_t*
CItemGloballyUniqueIdentifierGenerator::generate(
    itemGloballyUniqueIdentifier_t* identifier,
    int server_id) {
    identifier->SetServerId(server_id);
    identifier->SetDatetime(static_cast<int>(::time(0)));
    identifier->SetSequenceNo(static_cast<uint16_t>(getSeq()));
    return identifier;
}

int CItemGloballyUniqueIdentifierGenerator::getSeq() {
    ++sequence_;
    if (sequence_ > 0xEA5Fu) sequence_ = 0;
    return sequence_;
}

// IDA_EA: 0x85152D4
// IDA_NAME: _ZN5CItemC2Ev
// DEMANGLED: CItem::CItem(void)
CItem::CItem() {
    vtable_ = &kCItemVTable;
    rental_service_type_ = 0;
    reserved_146_[0] = 0;
    reserved_146_[1] = 0;
}

// IDA_EA: 0x85141EE
// IDA_NAME: _ZN5CItemD2Ev
// DEMANGLED: CItem::~CItem()
CItem::~CItem() {
    vtable_ = &kCItemVTable;
}

// IDA_EA: 0x822C86E
// IDA_NAME: _ZNK5CItem10GetDyeInfoEv
// DEMANGLED: CItem::GetDyeInfo(void)const
DyeInfo* CItem::GetDyeInfo() const {
    return const_cast<DyeInfo*>(&dye_info_);
}

// CLIENT_EA: 0x9607D0
// CLIENT_NAME: CItem_GetRentalServiceType
// The client reads the common item-script byte at +0x1EC. The upgraded server
// stores the same parsed category in the existing CItem ABI tail.
int CItem::GetRentalServiceType() const {
    return static_cast<int>(rental_service_type_);
}

extern "C" void CItemDeletingDtor(CItem* item) {
    item->~CItem();
    ::operator delete(item);
}

// IDA_EA: 0x851437E .. 0x8514412
int CItem::IsImpossibleUpgrade() const { return impossible_upgrade_; }
int CItem::IsImpossibleAmplify() const { return impossible_amplify_; }
int CItem::IsImpossibleDisjoint() const { return impossible_disjoint_; }
int CItem::IsImpossibleGift() const { return impossible_gift_; }
int CItem::IsImpossibleCharacCargo() const { return impossible_charac_cargo_; }
int CItem::IsPossibleKiriProtection() const { return possible_kiri_protection_; }
int CItem::PeriodAfterUnsealing() const { return period_after_unsealing_; }
// IDA_EA: 0x8514404
// IDA_NAME: _ZNK5CItem21GetQuestItemDropBounsEv
int CItem::GetQuestItemDropBouns() const { return quest_item_drop_bonus_; }
bool CItem::IsEventRewardItem() const {
    return item_script_0828::IsEventRewardPrototype(item_index);
}
int CItem::IsRandomOption() const { return 0; }
// IDA_EA: 0x851441C
// IDA_NAME: _ZNK5CItem20GetRandomOptionGradeEv
int CItem::GetRandomOptionGrade() const { return 0; }
// IDA_EA: 0x8514426
// IDA_NAME: _ZNK5CItem20IsRandomOptionUnsealEv
int CItem::IsRandomOptionUnseal() const { return 0; }
// IDA_EA: 0x85136AC
// IDA_NAME: _ZNK5CItem22isAmplifyPollutionItemEv
int CItem::isAmplifyPollutionItem() const { return 1; }
// IDA_EA: 0x8513884
// IDA_NAME: _ZNK5CItem24checkItemUpgradePossibleEv
int CItem::checkItemUpgradePossible() const { return 0; }

// IDA_EA: 0x851547E
// IDA_NAME: _ZN10CEquipItemC2Ev
// DEMANGLED: CEquipItem::CEquipItem(void)
CEquipItem::CEquipItem() {
    vtable_ = &kCEquipItemVTable;
    full_avatar_flag_ = 0;
    skin_emblem_socket_enabled_ = 0;
    reserved_23e_[0] = 0;
    reserved_23e_[1] = 0;
}

// IDA_EA: 0x8514B70
// IDA_NAME: _ZN10CEquipItemD2Ev
// DEMANGLED: CEquipItem::~CEquipItem()
CEquipItem::~CEquipItem() {
    vtable_ = &kCEquipItemVTable;
}

extern "C" void CEquipItemDeletingDtor(CEquipItem* item) {
    item->~CEquipItem();
    ::operator delete(item);
}

// IDA_EA: 0x850FD84
// IDA_NAME: _ZN10CEquipItem8set_itemER17STEquipmentScript
// DEMANGLED: CEquipItem::set_item(STEquipmentScript &)
CEquipItem* CEquipItem::set_item(STEquipmentScript& script) {
    item_index = At<int>(&script, 0u);
    item_script_0828::RegisterEventRewardPrototype(item_index, &script.item);
    grade = At<int>(&script, 4u);
    price = At<int>(&script, 20u);
    sell_price_ = At<int>(&script, 36u);
    cash_price = At<int>(&script, 24u);
    medal_price = At<int>(&script, 28u);
    rarity_ = At<int>(&script, 16u);
    packagable_ = At<uint8_t>(&script, 252u);
    script_value_pair_.first = At<int>(&script, 40u);
    script_value_pair_.second = At<int>(&script, 44u);
    if (ByteAt(&script, 48u) != 0u) {
        G_CDataManager()->set_AuctionRegFreeCeraItem(
            static_cast<unsigned int>(item_index));
    }

    char converted_name[500];
    ::memset(converted_name, 0, sizeof(converted_name));
    const std::string& script_name = At<std::string>(&script, 172u);
    ::strcpy(converted_name, script_name.c_str());
    CodePage::script2Database(
        const_cast<char*>(script_name.c_str()), converted_name);
    item_name_ = converted_name;

    attach_type_ = At<int>(&script, 8u);
    generation_rate_ = At<int>(&script, 12u);
    finish_point_price_ = At<int>(&script, 32u);
    advance_altar_need_gage_ = At<int>(&script, 2188u);
    for (size_t offset = 0u;
         offset < static_cast<size_t>(oldsrv::config::kCharacterJobCount);
         ++offset) {
        usable_job_flags_[offset] = ByteAt(&script, 56u + offset);
    }
    usable_level_ = At<int>(&script, 92u);
    script_value_4c_ = At<int>(&script, 96u);
    item_type_ = At<int>(&script, 436u);
    maximum_endurance_ = At<int>(&script, 444u);
    stackable_ = 0;
    ::memset(reserved_55_, 0, sizeof(reserved_55_));
    need_skill_ = At<int>(&script, 388u);
    foot_control_rate_limit_ = At<int>(&script, 384u);
    experience_amount_ = At<float>(&script, 1872u);
    creature_species_ = At<int>(&script, 1724u);
    creature_minimum_level_ = At<int>(&script, 1848u);

    if (IntAt(&script, 436u) == oldsrv::config::kEquipmentTypeCreature) {
        switch (IntAt(&script, 440u)) {
            case 1:
                hatch_item_id_ = At<int>(&script, 1892u);
                break;
            case 2:
                random_item_table_.Set(&At<std::vector<int> >(&script, 136u));
                break;
            case 0:
                // IDA 0x8510065: creature equipment links the source
                // creature ID (+0x6BC, copied to item +0x15C) to the item
                // prototype index (+0x04) before Creature.lst is loaded.
                // LoadCreatureScriptFile relies on this exact map rather
                // than permitting map::operator[] to manufacture item 0.
                user_creature::GetInstanceCreatureScriptMgr()
                    ->RegisterCreatureIdAndItemId(creature_species_, item_index);
                break;
            default:
                break;
        }
    }
    sub_type_ = At<int>(&script, 440u);
    usable_period_ = At<int>(&script, 244u);
    expiration_date_ = At<int>(&script, 248u);
    clear_avatar_ = At<uint8_t>(&script, 1964u);
    avatar_type_selects_ =
        At<std::vector<stAvatarTypeSelect_t> >(&script, 1728u);
    avatar_socket_types_ = At<std::vector<unsigned short> >(&script, 1740u);
    avatar_function_mask_ = At<int>(&script, 1752u);
    growth_repair_cost_ = At<int>(&script, 376u);
    item_group_name_ = At<int>(&script, 256u);
    script_flag_78_ = At<uint8_t>(&script, 260u);
    epic_routing_item_ = At<uint8_t>(&script, 1980u);
    item_icon_name_ = At<std::string>(&script, 108u);
    guild_power_war_point_ = At<int>(&script, 200u);
    user_power_war_point_ = At<int>(&script, 204u);
    impossible_upgrade_ = At<uint8_t>(&script, 1780u);
    impossible_amplify_ = At<uint8_t>(&script, 1781u);
    impossible_disjoint_ = At<uint8_t>(&script, 1782u);
    impossible_gift_ = At<uint8_t>(&script, 208u);
    impossible_charac_cargo_ = At<uint8_t>(&script, 218u);
    possible_kiri_protection_ = At<uint8_t>(&script, 217u);
    period_after_unsealing_ = At<int>(&script, 1784u);
    special_monster_drop_ = At<STSpecialMonsterDrop>(&script, 261u);
    At<STSpecialMonsterDrop>(&script, 261u).clear();
    emancipate_ = At<uint8_t>(&script, 270u);
    emancipate_materials_ =
        At<std::vector<std::pair<int, int> > >(&script, 2008u);
    emancipate_results_ =
        At<std::vector<std::pair<int, int> > >(&script, 2020u);
    enchant_system_data_ =
        At<std::vector<STEnchantSystemData> >(&script, 1996u);

    usable_equipment_type_ = 0;
    if (IsAvatarItem()) {
        if (grade == 3 && IntAt(&script, 1944u) == 3) {
            usable_equipment_type_ = 6;
        }
        setAvatarOptionSize(CountValidAvatarAbilityOptions(
            At<std::vector<STAvatarAbilityVariation> >(&script, 1060u)));
    } else if (GetItemType() == oldsrv::config::kEquipmentTypeCreature) {
        if (IntAt(&script, 16u) == 2) usable_equipment_type_ = 10;
    } else {
        switch (IntAt(&script, 16u)) {
            case 0: usable_equipment_type_ = 7; break;
            case 1: usable_equipment_type_ = 8; break;
            case 2:
                usable_equipment_type_ = IntAt(&script, 196u) == 5 ? 2 :
                    (IntAt(&script, 196u) == 30 ? 1 : 9);
                break;
            case 3: usable_equipment_type_ = 3; break;
            case 4: usable_equipment_type_ = 5; break;
            case 5: usable_equipment_type_ = 4; break;
            default: break;
        }
    }
    routing_item_ = At<uint8_t>(&script, 271u);
    priority_jobs_ = At<std::vector<std::pair<int, int> > >(&script, 272u);
    quest_item_drop_bonus_ = At<int>(&script, 2064u);
    usable_max_level_ = At<int>(&script, 380u);
    enabled_worlds_ = At<std::set<int> >(&script, 220u);
    usable_pvp_rank_ = At<int>(&script, 100u);
    default_avatar_period_ = At<int>(&script, 1760u);
    random_option_ = At<uint8_t>(&script, 2184u);
    random_option_grade_ = At<int8_t>(&script, 2185u);
    expert_job_material_variation_ = At<float>(&script, 336u);
    expert_job_compound_rate_variation_ = At<float>(&script, 340u);
    expert_job_usable_ =
        At<std::vector<STItemScript::SEXPERTJOB::SUSABLE> >(&script, 344u);
    expert_job_result_variation_.value_00 = At<float>(&script, 312u);
    expert_job_result_variation_.value_04 = At<float>(&script, 316u);
    expert_job_disjoint_big_win_rate_ = At<float>(&script, 328u);
    expert_job_disjoint_result_variation_ = At<float>(&script, 332u);
    expert_job_additional_exp_.value_00 = At<float>(&script, 320u);
    expert_job_additional_exp_.value_04 = At<int>(&script, 324u);
    categories_ =
        At<std::set<STItemScript::ENUM_ITEM_CATEGORY> >(&script, 284u);
    upgrade_cost_discount_ = At<float>(&script, 1540u);
    upgrade_probability_increase_ = At<int>(&script, 1544u);
    script_value_1d4_ = At<int>(&script, 1548u);
    aura_avatar_option_type_ = At<uint8_t>(&script, 2176u);
    aura_avatar_option_value_ = At<int>(&script, 2180u);
    wp_bonus_point_ = At<WpBonusPointData>(&script, 2076u);
    exp_bonus_point_ = At<ExpBonusPointData>(&script, 2084u);
    avatar_ability_variations_ =
        At<std::vector<STAvatarAbilityVariation> >(&script, 1060u);
    amplify_pollution_item_ = IntAt(&script, 2192u) != 1;
    upgrade_level_limits_ =
        At<std::map<ItemUpgradeType::T, std::pair<int, int> > >(
            &script, 1788u);
    avatar_color_variations_[0] = At<uint8_t>(&script, 2186u);
    avatar_color_variations_[1] = At<uint8_t>(&script, 2187u);
    growth_type_ = At<uint8_t>(&script, 1692u);
    growth_begin_level_ = At<int>(&script, 488u);
    growth_end_level_ = At<int>(&script, 492u);
    growth_begin_grade_ = At<int>(&script, 1440u);
    growth_end_grade_ = At<int>(&script, 1444u);
    growth_begin_repair_cost_ = At<int>(&script, 1480u);
    growth_end_repair_cost_ = At<int>(&script, 1484u);
    ban_redeem_item_ = At<uint8_t>(&script, 216u);
    parent_booster_item_id_ = 0;
    rental_service_type_ = script.rental_service_type;
    full_avatar_flag_ = script.full_avatar_flag != 0 ? 1u : 0u;
    skin_emblem_socket_enabled_ =
        script.skin_emblem_socket_enabled != 0 ? 1u : 0u;
    reserved_23e_[0] = 0;
    reserved_23e_[1] = 0;
    bool has_expand_path = false;
    for (std::map<ENUM_CHARACTERJOB, EquipmentAniInfoScript>::const_iterator it =
             script.animation_infos_640.begin();
         it != script.animation_infos_640.end(); ++it) {
        if (!it->second.unknown_string_4C.empty()) {
            has_expand_path = true;
            break;
        }
    }
    cargo_transport::RegisterEquipmentGrowthPrototype(
        item_index,
        script.equipment_growth_type,
        script.equipment_growth_stage,
        has_expand_path);
    equipment_growth::RegisterPrototype(
        item_index,
        script.equipment_growth_type,
        script.equipment_growth_stage);
    compound_creature::RegisterEquipmentPrototype(
        item_index, script.value_82c != 0);
    return this;
}

// The target PVF uses valueless [full avatar] only on skin avatars; the
// minimum USERINFO reader consumes the equipped Equipment.lst item ID.
bool CEquipItem::IsFullAvatar() const {
    return full_avatar_flag_ != 0;
}

bool CEquipItem::HasSkinEmblemSocket() const {
    return skin_emblem_socket_enabled_ != 0;
}

// IDA_EA: 0x851098A
// IDA_NAME: _ZNK10CEquipItem9make_itemER10Inven_Item
int CEquipItem::make_item(Inven_Item& item) const {
    if (IsAvatarItem()) {
        item.item_category_ = 8;
    } else {
        item.item_category_ = 1;
    }

    const int item_type = GetItemType();
    if (item_type == oldsrv::config::kEquipmentTypeCreature) {
        item.item_category_ = 5;
    } else if (item_type >= oldsrv::config::kEquipmentTypeArtifactRed &&
               item_type <= oldsrv::config::kEquipmentTypeArtifactGreen) {
        item.item_category_ = 6;
    }

    item.set_add_info(static_cast<int>(get_rand_int(0)));
    item.ResetItemAttr();
    item.avatar_select_ = static_cast<uint16_t>(maximum_endurance_);

    if (item.item_category_ == 8 && default_avatar_period_ > 0 &&
        default_avatar_period_ <
            static_cast<int>(avatar_type_selects_.size())) {
        item.avatar_select_ = static_cast<uint16_t>(default_avatar_period_);
    }

    if (attach_type_ == 3) item.seal_or_state_ = 1;
    if (PeriodAfterUnsealing() == 0) {
        int expiration_date = 0;
        if (getUsablePeriod()) {
            expiration_date = GlobalData::s_systemTime_.getCurSec() +
                              86400 * getUsablePeriod();
        } else if (getExpirationDate()) {
            expiration_date = getExpirationDate();
        }
        if (expiration_date != 0 &&
            item_type != oldsrv::config::kEquipmentTypeWeaponAvatar) {
            item.avatar_select_ = static_cast<uint16_t>(
                (expiration_date - 1151683200) / 86400);
        }
    }

    item.extend_info_ = 0;
    item.globally_unique_id_.reset();
    const unsigned int unique_id_item_type =
        static_cast<unsigned int>(GetItemType());
    if (unique_id_item_type <
            static_cast<unsigned int>(oldsrv::config::kEquipmentTypeCount) &&
        ((1u << unique_id_item_type) & 0x03BFFC00u) != 0u) {
        g_ItemGloballyUniqueIdentifierGenerator.generate(
            &item.globally_unique_id_,
            At<int>(G_CEnvironment(), 408u));
    }
    return static_cast<int>(reinterpret_cast<uintptr_t>(
        item.amplify_option_.reset()));
}

// IDA_EA: 0x85141A8
// IDA_NAME: _ZN14StatChangeInfoC2Ev
// DEMANGLED: StatChangeInfo::StatChangeInfo(void)
StatChangeInfo::StatChangeInfo()
    : duration_ms(0), duration_tag(0), timer_enabled(0) {}

// IDA_EA: 0x8515762
// IDA_NAME: _ZN14CStackableItemC2Ev
// DEMANGLED: CStackableItem::CStackableItem(void)
CStackableItem::CStackableItem() {
    vtable_ = &kCStackableItemVTable;
    verify_stack_number_ = 0;
    reserved_459_45b_[0] = 0;
    reserved_459_45b_[1] = 0;
    reserved_459_45b_[2] = 0;
    all_skill_plus_ = 0;
    // Mirrors the 0828 STStackableScript::Clear sentinel at client offset
    // +0x650.  Items without [levelup mode] retain legacy range semantics.
    levelup_mode_ = -1;
}

// IDA_EA: 0x85146B4
// IDA_NAME: _ZN14CStackableItemD2Ev
// DEMANGLED: CStackableItem::~CStackableItem()
CStackableItem::~CStackableItem() {
    vtable_ = &kCStackableItemVTable;
}

extern "C" void CStackableItemDeletingDtor(CStackableItem* item) {
    item->~CStackableItem();
    ::operator delete(item);
}

// IDA_EA: 0x850EBFC
// IDA_NAME: _ZN14CStackableItem14get_lotto_itemEPc
// DEMANGLED: CStackableItem::get_lotto_item(char *)
Inven_Item* CStackableItem::get_lotto_item(char* output) {
    Inven_Item* const result = reinterpret_cast<Inven_Item*>(output);
    LotteryInfo& lottery_info = lottery_info_;
    const unsigned long probability_limit = 100000u;
    const int random_value = static_cast<int>(
        limit_cube_random_->randInt(probability_limit));

    int selected_item = -1;
    int selected_count = 1;
    int cumulative_probability = 0;
    std::vector<int>::const_iterator count = lottery_info.int_values.begin();
    for (std::vector<std::pair<int, int> >::const_iterator item =
             lottery_info.pair_values.begin();
         item != lottery_info.pair_values.end();
         ++item, ++count) {
        cumulative_probability += item->second;
        if (cumulative_probability > random_value) {
            selected_item = item->first;
            selected_count = *count;
            break;
        }
    }

    // The original tests the item id before the count.  This preserves the
    // zero-id/count case as well as its ordinary default-item fallback.
    if (selected_item != 0 || selected_count <= 0) {
        if (selected_item == -1) {
            selected_item = static_cast<int>(lottery_info.first_dword);
            selected_count = static_cast<int>(lottery_info.second_dword);
        }
        if (selected_item != 0) {
            result->item_id_ = selected_item;
            G_CDataManager()->get_item_list()->create_item(
                static_cast<ENUM_ITEM_CREATE_TYPE>(2), *result,
                selected_count);
            return result;
        }
    }

    result->item_id_ = 0;
    result->set_add_info(selected_count);
    return result;
}

// IDA_EA: 0x850EDDA
// IDA_NAME: _ZN14CStackableItem25get_multibox_lottery_itemEPSt6vectorISt4pairI10Inven_ItembESaIS3_EE
// DEMANGLED: CStackableItem::get_multibox_lottery_item(std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> *)
void CStackableItem::get_multibox_lottery_item(
    std::vector<std::pair<Inven_Item, bool> >* output) {
    if (!output) return;

    MultiBoxLotteryInfo& lottery_info = multibox_lottery_info_;
    for (std::vector<MultiBoxLotteryInfo::LotteryUnit>::const_iterator unit =
             lottery_info.lottery_units.begin();
         unit != lottery_info.lottery_units.end(); ++unit) {
        // The old body copies every unit before walking its two parallel
        // tables.  Keep that value lifetime so the source vectors cannot be
        // affected by any allocation performed while rewards are appended.
        const MultiBoxLotteryInfo::LotteryUnit current_unit(*unit);
        for (uint32_t index = 0u; index < current_unit.first_dword; ++index) {
            const int random_value = static_cast<int>(get_rand_int(1000000));
            int cumulative_probability = 0;
            int selected_item = 0;
            int selected_count = 0;
            bool selected_flag = false;

            std::vector<std::pair<int, int> >::const_iterator second =
                current_unit.second_pairs.begin();
            for (std::vector<std::pair<int, int> >::const_iterator first =
                     current_unit.first_pairs.begin();
                 first != current_unit.first_pairs.end(); ++first, ++second) {
                cumulative_probability += first->second;
                if (cumulative_probability > random_value) {
                    selected_item = first->first;
                    selected_count = second->first;
                    selected_flag = second->second != 0;
                    break;
                }
            }

            if (selected_item == 0) {
                selected_item = static_cast<int>(current_unit.second_dword);
                selected_count = static_cast<int>(current_unit.third_dword);
                selected_flag = current_unit.flag != 0;
            }

            Inven_Item reward;
            reward.item_id_ = selected_item;
            G_CDataManager()->get_item_list()->create_item(
                static_cast<ENUM_ITEM_CREATE_TYPE>(2), reward, selected_count);
            output->push_back(std::make_pair(reward, selected_flag));
        }
    }
}

// IDA_EA: 0x850F136
// IDA_NAME: _ZN14CStackableItem25get_multibox_default_itemEPSt6vectorISt4pairI10Inven_ItembESaIS3_EE
// DEMANGLED: CStackableItem::get_multibox_default_item(std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> *)
void CStackableItem::get_multibox_default_item(
    std::vector<std::pair<Inven_Item, bool> >* output) {
    if (!output) return;

    MultiBoxLotteryInfo::stDefaultItemInfo& default_info =
        multibox_lottery_info_.default_item_info;
    for (uint32_t index = 0u; index < default_info.first_dword; ++index) {
        const int random_value = static_cast<int>(get_rand_int(1000000));
        int cumulative_probability = 0;
        int selected_item = 0;
        int selected_count = 0;
        bool selected_flag = false;

        for (std::vector<MultiBoxLotteryInfo::stDefaultItem>::const_iterator
                 item = default_info.default_items.begin();
             item != default_info.default_items.end(); ++item) {
            cumulative_probability += static_cast<int>(item->third_dword);
            if (cumulative_probability > random_value) {
                selected_item = static_cast<int>(item->first_dword);
                selected_count = static_cast<int>(item->second_dword);
                selected_flag = item->flag != 0;
                break;
            }
        }

        if (selected_item == 0) {
            selected_item = static_cast<int>(default_info.second_dword);
            selected_count = static_cast<int>(default_info.third_dword);
            selected_flag = default_info.flag != 0;
        }

        Inven_Item reward;
        reward.item_id_ = selected_item;
        G_CDataManager()->get_item_list()->create_item(
            static_cast<ENUM_ITEM_CREATE_TYPE>(2), reward, selected_count);
        output->push_back(std::make_pair(reward, selected_flag));
    }
}

// IDA_EA: 0x850F38A
// IDA_NAME: _ZN14CStackableItem30get_multibox_default_item_aradEPSt6vectorISt4pairI10Inven_ItembESaIS3_EE
// DEMANGLED: CStackableItem::get_multibox_default_item_arad(std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> *)
void CStackableItem::get_multibox_default_item_arad(
    std::vector<std::pair<Inven_Item, bool> >* output) {
    if (!output) return;

    static std::vector<MultiBoxLotteryInfo::LotteryDefaultArad>
        default_item_list_arad;
    if (default_item_list_arad.empty()) {
        default_item_list_arad.push_back(
            MultiBoxLotteryInfo::LotteryDefaultArad(0x28BE16u, 10u, 0xC350u));
        default_item_list_arad.push_back(
            MultiBoxLotteryInfo::LotteryDefaultArad(0x28BE16u, 50u, 0x61A8u));
        default_item_list_arad.push_back(
            MultiBoxLotteryInfo::LotteryDefaultArad(0x28BE16u, 100u, 0x1388u));
        default_item_list_arad.push_back(
            MultiBoxLotteryInfo::LotteryDefaultArad(0x28D150u, 1u, 0x2710u));
    }

    int selected_item = 0;
    int selected_count = 0;
    if (!default_item_list_arad.empty()) {
        const int random_value = static_cast<int>(get_rand_int(100000));
        int cumulative_probability = 0;
        for (std::vector<MultiBoxLotteryInfo::LotteryDefaultArad>::const_iterator
                 item = default_item_list_arad.begin();
             item != default_item_list_arad.end(); ++item) {
            cumulative_probability += static_cast<int>(item->rate);
            if (cumulative_probability >= random_value) {
                selected_item = static_cast<int>(item->item_id);
                selected_count = static_cast<int>(item->count);
                break;
            }
        }
    }

    Inven_Item reward;
    reward.item_id_ = selected_item;
    G_CDataManager()->get_item_list()->create_item(
        static_cast<ENUM_ITEM_CREATE_TYPE>(2), reward, selected_count);
    output->push_back(std::make_pair(reward, false));
}

// IDA_EA: 0x850D81E
// IDA_NAME: _ZN14CStackableItem8set_itemER17STStackableScript
// DEMANGLED: CStackableItem::set_item(STStackableScript &)
int CStackableItem::set_item(STStackableScript& script) {
    const unsigned long seed = static_cast<unsigned long>(
        script.item.index + static_cast<int>(::time(0)));
    limit_cube_random_ = new CMTRand(seed);

    item_index = script.item.index;
    item_script_0828::RegisterEventRewardPrototype(item_index, &script.item);
    grade = script.item.grade;
    price = script.item.values_08[3];
    cash_price = script.item.values_08[4];
    medal_price = script.item.values_08[5];
    sell_price_ = script.item.value_24;
    rarity_ = script.item.values_08[2];
    packagable_ = script.item.value_fc;
    if (script.item.value_30 != 0u) {
        G_CDataManager()->set_AuctionRegFreeCeraItem(
            static_cast<unsigned int>(item_index));
    }

    char converted_name[500];
    ::memset(converted_name, 0, sizeof(converted_name));
    const std::string& script_name = script.item.strings_ac[0];
    ::strcpy(converted_name, script_name.c_str());
    CodePage::script2Database(
        const_cast<char*>(script_name.c_str()), converted_name);
    item_name_ = converted_name;

    usable_period_ = script.item.values_f4[0];
    expiration_date_ = script.item.values_f4[1];
    finish_point_price_ = script.item.values_08[6];
    advance_altar_need_gage_ = script.values_4ec[0];
    script_value_2cc_ = script.value_18c;
    script_value_pair_.first = script.item.value_28;
    script_value_pair_.second = script.item.value_2c;
    generation_rate_ = script.item.values_08[1];
    attach_type_ = script.item.values_08[0];
    item_type_ = script.value_1e0;
    monster_card_id_ = script.value_3cc;

    switch (item_type_) {
        case 3:
            createRecipeInfo(&recipe_info_, &script.item.int_data, 0);
            recipe_info_.pairs_28 = script.pairs_3c0;
            recipe_info_.additionalInfo(script);
            break;
        case 6:
            createLotteryInfo(&lottery_info_, &script.item.int_data);
            break;
        case 7:
        case 13:
            if (!CreateStackableLotteryInfo(
                    &lottery_info_, script.item.int_data)) {
                return 0;
            }
            break;
        case 26:
        case 27:
            if (!CreateStackableMultiBoxLotteryInfo(
                    &multibox_lottery_info_, script.values_244,
                    script.values_250, script.values_25c)) {
                return 0;
            }
            break;
        case 14:
        case 38:
            package_data_ = script.package_data_268;
            package_selection_data_ = script.package_data_274;
            if (!script.package_data_2a4.empty()) {
                alternate_package_data_ = script.package_data_2a4;
                script.package_data_2a4.clear();
            }
            break;
        case 15:
        case 16:
            booster_data_ = script.booster_280;
            break;
        case 17:
            booster_data_ = script.booster_280;
            item_type_ = 16;
            break;
        case 34:
            booster_data_ = script.booster_280;
            booster_select_info_ = script.booster_select_41c;
            break;
        case 23:
            if (CreateGlobalEffectInfo(
                    &global_effect_data_, script.item.int_data) != 1) {
                return 0;
            }
            break;
        case 29:
            limit_cube_info_ = script.limit_cube_3d4;
            break;
        default:
            break;
    }

    std::copy(script.item.usable_job_flags,
              script.item.usable_job_flags +
                  oldsrv::config::kCharacterJobCount,
              usable_job_flags_);
    usable_level_ = script.item.values_5c[0];
    script_value_4c_ = script.item.values_5c[1];
    stackable_ = 1;
    stackable_limit_ = script.value_364;
    sub_type_ = script.value_1e4;
    usable_equipment_types_.clear();
    if (item_type_ == 2 && sub_type_ == 1) {
        const std::vector<std::string>& type_names = script.item.string_data;
        for (size_t index = 1u; index < type_names.size(); ++index) {
            std::string type_name = type_names[index];
            const int type = GetEquipmentTypeStringToEnum(type_name);
            if (type != oldsrv::config::kEquipmentTypeInvalid) {
                usable_equipment_types_.push_back(type);
            }
        }
    }
    continuous_item_state_.cool_time_ms = script.item.values_5c[3];
    continuous_item_state_.cool_time_group = script.values_1a8[3];
    continuous_item_state_.cool_time_maintenance =
        script.continuous_option_1b9;
    continuous_item_state_.stat_change.duration_ms =
        script.stat_changes_34c.value_0c;
    continuous_item_state_.stat_change.duration_tag =
        script.stat_changes_34c.enabled;
    continuous_item_state_.stat_change.timer_enabled = script.value_1bf;
    continuous_item_state_.effect_maintenance =
        script.continuous_option_1bc;
    continuous_item_state_.exp_affect_enabled = script.exp_bonus_enabled_1c0;
    continuous_item_state_.exp_affect_ratio = script.exp_bonus_rate_1c4;
    continuous_item_state_.shared_effect_item = script.value_1d0;
    jewel_target_socket_ = script.value_380;
    item_group_name_ = script.item.value_100;
    script_flag_78_ = script.item.value_104;
    need_skill_ = -1;
    epic_routing_item_ = 0;
    guild_power_war_point_ = script.item.values_c4[1];
    user_power_war_point_ = script.item.values_c4[2];
    impossible_gift_ = script.item.value_d0;
    item_icon_name_ = script.item.icon;
    special_monster_drop_ = script.item.special_monster_drop;
    script.item.special_monster_drop.clear();
    emancipate_ = script.item.value_10e;
    continuous_item_state_.emancipation_value = script.value_394;
    third_chronicle_enchant_map_ = script.enchant_map_3f4;
    third_chronicle_enchant_group_ = script.value_40c;
    third_chronicle_enchant_rates_[0] =
        static_cast<char>(script.value_40d);
    third_chronicle_enchant_rates_[1] =
        static_cast<char>(script.value_40e);
    reinforce_ticket_ = script.value_396;
    reinforce_value_ = script.value_398;
    reinforce_rate_ = script.value_39a;
    amplify_reinforce_ticket_ = script.value_39b;
    amplify_reinforce_value_ = static_cast<uint16_t>(script.value_39c);
    amplify_reinforce_rate_ = script.values_3a0[0];
    has_usable_item_level_range_ = script.value_438;
    if (has_usable_item_level_range_ != 0u) {
        usable_item_level_min_ = script.pair_43c.first;
        usable_item_level_max_ = script.pair_43c.second;
    }
    routing_item_ = script.item.value_10f;
    priority_jobs_ = script.item.special_drop_items;
    quest_item_drop_bonus_ = 0;
    script_flag_d0_ = script.values_3d0[0];
    deletable_item_ = script.values_3d0[1];
    enabled_worlds_ = script.item.usable_worlds;
    usable_pvp_rank_ = script.item.values_5c[2];
    up_skill_type_ = script.value_444;
    need_level_ = script.values_448[0];
    expert_job_material_variation_ = script.item.expert_job.material_variation;
    expert_job_compound_rate_variation_ = script.item.expert_job.compound_rate;
    expert_job_usable_ = script.item.expert_job.usable;
    private_store_info_.expert_job_value_1 = script.values_448[1];
    private_store_info_.expert_job_value_3 = script.values_448[3];
    private_store_info_.shop_doll_item_index = script.values_448[2];
    private_store_info_.expert_job_value_4 = script.values_448[4];
    private_store_info_.expert_job_value_5 = script.values_448[5];
    private_store_info_.expert_job_value_6 = script.values_448[6];
    private_store_info_.expert_job_value_7 = script.values_448[7];
    categories_.clear();
    for (std::set<int>::const_iterator category =
             script.item.item_categories.begin();
         category != script.item.item_categories.end(); ++category) {
        categories_.insert(
            static_cast<STItemScript::ENUM_ITEM_CATEGORY>(*category));
    }
    if (script.values_448[11] != 0) {
        G_CDataManager()->set_lottery_use_cost(
            static_cast<unsigned int>(item_index),
            static_cast<unsigned int>(script.values_448[11]));
    }
    lottery_gold_broadcast_minimum_ = script.value_478;
    random_amplify_reinforce_ticket_ = script.values_3a0[1];
    amplify_reinforce_count_rates_ = script.map_3a4;
    arad_item_info_.values_00[0] = script.arad.value_00;
    arad_item_info_.values_00[1] = script.arad.value_04;
    arad_item_info_.string_08 = script.arad.value_08;
    arad_item_info_.string_0c = script.arad.value_0C;
    arad_item_info_.string_10 = script.arad.value_10;
    if (!script.rabbit_boxes_19c.empty()) {
        arad_item_info_.rabbit_20.reserve(script.rabbit_boxes_19c.size());
        std::copy(script.rabbit_boxes_19c.begin(),
                  script.rabbit_boxes_19c.end(),
                  std::back_inserter(arad_item_info_.rabbit_20));
    }
    if (!script.values_190.empty()) {
        arad_item_info_.values_14.reserve(script.values_190.size());
        std::copy(script.values_190.begin(), script.values_190.end(),
                  std::back_inserter(arad_item_info_.values_14));
    }
    evolution_creature_id_ = script.evolution_creature_id;
    summon_monster_enabled_ = script.summon_monster_314.enabled;
    avatar_option_change_type_ = script.avatar_option_change_47c;
    usable_avatar_item_ids_ = script.set_480;
    prohibited_avatar_options_ = script.set_498;
    avatar_option_change_item_ = script.value_4b0;
    stackable_action_type_ = script.value_1e8;
    action_values_ = script.values_1ec;
    usable_places_ = script.usable_places_1f8;
    usable_server_types_ = script.usable_server_types_214;
    // `[daily delete item]`: STStackableScript +0x205 -> CStackableItem
    // +0x3c5.  Keep this as a named ABI field for the world-entry cleanup
    // path rather than carrying the old raw offset into its caller.
    daily_delete_item_ = script.daily_delete_item_;
    impossible_charac_cargo_ = script.item.value_da;
    increase_status_.type = script.value_220;
    increase_status_.values = script.values_224;
    dye_info_ = script.item.dye_info;
    random_option_unseal_ = script.value_4b8 != 0;
    ban_redeem_item_ = script.item.value_d8;
    parent_booster_item_id_ = 0;
    script_value_3d0_ = script.values_448[8];
    booster_data_mode_ = script.values_448[9];
    if (booster_data_mode_ > 1) {
        booster_data_ = script.booster_280;
    }
    script_value_3cc_ = script.values_1a8[1];
    trade_limit_count_ = script.value_508;
    bead_target_item_ids_ = script.set_50c;
    limit_dungeon_ids_ = script.limit_dungeon_ids;
    add_socket_limited_avatar_ids_ =
        script.add_socket_limited_avatar_ids;
    resent_day_value_ = script.resent_day_value;
    cera_coupon_ = script.cera_coupon;
    exclusive_dungeon_tooltip_enabled_ =
        script.exclusive_dungeon_tooltip_enabled;
    reserved_426_427_[0] = 0;
    reserved_426_427_[1] = 0;
    exclusive_dungeon_ids_ = script.exclusive_dungeon_ids;
    avatar_booster_preview_item_ids_ =
        script.avatar_booster_preview_item_ids;
    stackable_appendage_scripts_ = script.stackable_appendage_scripts;
    verify_stack_number_ = script.verify_stack_number;
    reserved_459_45b_[0] = 0;
    reserved_459_45b_[1] = 0;
    reserved_459_45b_[2] = 0;
    all_skill_plus_ = script.all_skill_plus;
    levelup_mode_ = script.levelup_mode;
    disguise_appendages_ = script.disguise_appendages;
    emancipate_grow_groups_ = script.emancipate_grow_groups;
    emancipate_material_revisions_ =
        script.emancipate_material_revisions;
    stackable_action_limit_indexes_ =
        script.stackable_action_limit_indexes;
    random_image_entries_ = script.random_image_entries;
    return 1;
}

// IDA_EA: 0x850F8B8
// IDA_NAME: _ZNK14CStackableItem9make_itemER10Inven_Item
int CStackableItem::make_item(Inven_Item& item) const {
    switch (GetItemType()) {
        case 1: item.item_category_ = 3; break;
        case 2: item.item_category_ = 10; break;
        case 8: item.item_category_ = 4; break;
        case 9:
        case 10: item.item_category_ = 7; break;
        case 20: item.item_category_ = 9; break;
        default: item.item_category_ = 2; break;
    }

    At<uint16_t>(&item, 11u) = 0;
    int expiration_date = 0;
    if (getUsablePeriod()) {
        expiration_date = GlobalData::s_systemTime_.getCurSec() +
                          86400 * getUsablePeriod();
    } else if (getExpirationDate()) {
        expiration_date = getExpirationDate();
    }
    if (expiration_date != 0) {
        At<uint16_t>(&item, 11u) = static_cast<uint16_t>(
            (expiration_date - 1151683200) / 86400);
    }

    item.ResetItemAttr();
    IntAt(&item, 13u) = 0;
    At<itemGloballyUniqueIdentifier_t>(&item, 21u).reset();
    At<stAmplifyOption_t>(&item, 17u).reset();
    if (GetAttachType() == 7) {
        item.SetTradeLimitCount(static_cast<uint8_t>(getTradeLimitCount()));
        return 1;
    }
    return 0;
}

// The corresponding df_game_r bodies are constant false.  They remain real
// prototype vtable entries instead of default/fallback callbacks.
int CStackableItem::IsEnchantalbe() const { return 0; }
int CStackableItem::IsAvatarItem() const { return 0; }
int CStackableItem::IsClearAvatar() const { return 0; }
bool CStackableItem::isEnableAvatarFunc(ENUM_AVATAR_FUNC_FILTER) const {
    return false;
}
