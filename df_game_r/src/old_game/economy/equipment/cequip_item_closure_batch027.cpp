#include "old_game/economy/inventory/inventory.h"

#include "common/config.h"
#include <stddef.h>

static_assert(offsetof(CItemVTable, get_item_type) == 0x0cu,
              "CItem item-type virtual slot must stay at +0x0c");
static_assert(offsetof(CItemVTable, is_impossible_upgrade) == 0x30u,
              "CItem upgrade-policy virtual slot must stay at +0x30");
static_assert(offsetof(CItem, usable_period_) == 0x6cu,
              "CItem usable period must stay at +0x6c");
static_assert(offsetof(CItem, expiration_date_) == 0x70u,
              "CItem expiration date must stay at +0x70");
static_assert(offsetof(CEquipItem, avatar_type_selects_) == 0x174u,
              "CEquipItem avatar selections must stay at +0x174");
static_assert(offsetof(CEquipItem, avatar_ability_variations_) == 0x1f0u,
              "CEquipItem avatar variations must stay at +0x1f0");
static_assert(offsetof(CEquipItem, item_type_) == 0x234u,
              "CEquipItem item type must stay at +0x234");

// IDA_EA: 0x8151120
// IDA_NAME: _ZNK10CEquipItem15getAvatarPeriodEh
// DEMANGLED: CEquipItem::getAvatarPeriod(unsigned char)const
int CEquipItem::getAvatarPeriod(unsigned char avatar_select) const {
    if (avatar_select < avatar_type_selects_.size()) {
        return avatar_type_selects_[avatar_select].usable_period;
    }
    return getUsablePeriod();
}

// IDA_EA: 0x8510818
// IDA_NAME: _ZN10CEquipItem6verifyEv
// DEMANGLED: CEquipItem::verify(void)
bool CEquipItem::verify() {
    if (usable_period_ || expiration_date_) {
        const int item_type = vtable_->get_item_type(this);
        const bool is_avatar =
            item_type >= oldsrv::config::kAvatarEquipmentTypeFirst &&
            item_type <= oldsrv::config::kAvatarEquipmentTypeLast;
        const bool is_limited_accessory =
            item_type >= oldsrv::config::kEquipmentTypeWrist &&
            item_type <= oldsrv::config::kEquipmentTypeMagicStone;
        if (!is_avatar && !is_limited_accessory &&
            item_type != oldsrv::config::kEquipmentTypeWeapon &&
            item_type != oldsrv::config::kEquipmentTypeTitle &&
            item_type != oldsrv::config::kEquipmentTypeCreature) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x851353E
// IDA_NAME: _ZNK10CEquipItem19IsAvatarOptionValidEc
// DEMANGLED: CEquipItem::IsAvatarOptionValid(char)const
bool CEquipItem::IsAvatarOptionValid(char option) const {
    // The 0828 wire carries the explicit option key as one byte.  Preserve
    // the historical char ABI while interpreting all 0..255 keys unsigned.
    const unsigned int key = static_cast<unsigned char>(option);
    return key < avatar_ability_variations_.size() &&
           avatar_ability_variations_[key].ability_index >= 0;
}

// IDA_EA: 0x851388E
// IDA_NAME: _ZNK10CEquipItem24checkItemUpgradePossibleEv
// DEMANGLED: CEquipItem::checkItemUpgradePossible(void)const
bool CEquipItem::checkItemUpgradePossible() const {
    if (vtable_->is_impossible_upgrade(this)) return false;
    const unsigned int item_type = static_cast<unsigned int>(item_type_);
    const unsigned int upgradeable_equipment_mask =
        oldsrv::config::kOrdinaryEquipmentTypeMask &
        ~(1u << oldsrv::config::kEquipmentTypeTitle);
    return item_type < oldsrv::config::kEquipmentTypeCount &&
           ((1u << item_type) & upgradeable_equipment_mask) != 0u;
}
