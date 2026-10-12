#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include "old_game/economy/equipment/item_option.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

#include <string.h>

namespace {

// IDA 0x850CDAC addresses the eleventh equipment record: CInventory +0x1c
// plus 10 * sizeof(Inven_Item), then Inven_Item::upgrade_separate_ (+0x33).
// Keep the relation expressed through the recovered member graph instead of
// rebuilding CInventory byte offsets at each caller.
const unsigned int kWeaponEquipmentSlot =
    oldsrv::config::kEquipmentTypeWeapon;

}  // namespace

// IDA_EA: 0x8110C7A
// IDA_NAME: _ZNK10CInventory12GetEventCoinEv
// DEMANGLED: CInventory::GetEventCoin(void)const
int CInventory::GetEventCoin() const {
    return static_cast<int>(event_coin_);
}

// IDA_EA: 0x8110C86
// IDA_NAME: _ZN10CInventory12SetEventCoinEj
// DEMANGLED: CInventory::SetEventCoin(unsigned int)
CInventory* CInventory::SetEventCoin(unsigned int coin) {
    event_coin_ = coin;
    return this;
}

// IDA_EA: 0x81AC72E
// IDA_NAME: _ZNK10CInventory10GetPayCoinEv
// DEMANGLED: CInventory::GetPayCoin(void)const
int CInventory::GetPayCoin() const {
    return static_cast<int>(pay_coin_);
}

// IDA_EA: 0x81AC73A
// IDA_NAME: _ZN10CInventory10SetPayCoinEj
// DEMANGLED: CInventory::SetPayCoin(unsigned int)
CInventory* CInventory::SetPayCoin(unsigned int coin) {
    pay_coin_ = coin;
    return this;
}

// IDA_EA: 0x822D652
// IDA_NAME: _ZN10CInventory9set_moneyEj
// DEMANGLED: CInventory::set_money(unsigned int)
CInventory* CInventory::set_money(unsigned int money) {
    money_ = money;
    return this;
}

// IDA_EA: 0x822D660
// IDA_NAME: _ZNK10CInventory12getTotalCoinEv
// DEMANGLED: CInventory::getTotalCoin(void)const
int CInventory::getTotalCoin() const {
    return static_cast<int>(coin_ + pay_coin_ + event_coin_);
}

// IDA_EA: 0x822D696
// IDA_NAME: _ZNK10CInventory12IsCoinUsableEv
// DEMANGLED: CInventory::IsCoinUsable(void)const
bool CInventory::IsCoinUsable() const {
    return coin_ != 0u || pay_coin_ != 0u || event_coin_ != 0u;
}

// IDA_EA: 0x84FA9D4
// IDA_NAME: _ZN10CInventory9UseUpCoinEj
// DEMANGLED: CInventory::UseUpCoin(unsigned int)
int CInventory::UseUpCoin(unsigned int coin) {
    uint32_t& normal_coin = coin_;
    uint32_t& event_coin = event_coin_;
    uint32_t& pay_coin = pay_coin_;

    unsigned int remainder = 0u;
    if (normal_coin <= coin) {
        remainder = coin - normal_coin;
        normal_coin = 0u;
    } else {
        normal_coin -= coin;
    }

    unsigned int final_remainder = 0u;
    if (event_coin <= remainder) {
        final_remainder = remainder - event_coin;
        event_coin = 0u;
    } else {
        event_coin -= remainder;
    }

    if (pay_coin <= final_remainder) {
        pay_coin = 0u;
    } else {
        pay_coin -= final_remainder;
    }

    return static_cast<int>(normal_coin + event_coin + pay_coin);
}

// IDA_EA: 0x84FAA98
// IDA_NAME: _ZN10CInventory7UseCoinE14eCoinSubReason
// DEMANGLED: CInventory::UseCoin(eCoinSubReason)
int CInventory::UseCoin(eCoinSubReason reason) {
    if (coin_) {
        --coin_;
        if (parent_) {
            parent_->history_log().CoinSub(static_cast<int>(coin_), 1, reason);
        } else {
            cMyTrace trace("int CInventory::UseCoin(eCoinSubReason)", 654, 5);
            trace("History ERROR, m_pParent NULL, CoinSub");
        }
    } else if (event_coin_) {
        --event_coin_;
        if (parent_) {
            parent_->history_log().EventCoinSub(
                static_cast<int>(event_coin_), 1, reason);
        } else {
            cMyTrace trace("int CInventory::UseCoin(eCoinSubReason)", 669, 5);
            trace("History ERROR, m_pParent NULL, EventCoinSub");
        }
    } else if (pay_coin_) {
        --pay_coin_;
        if (parent_) {
            parent_->history_log().PayCoinSub(
                static_cast<int>(pay_coin_), 1, reason);
        } else {
            cMyTrace trace("int CInventory::UseCoin(eCoinSubReason)", 685, 5);
            trace("History ERROR, m_pParent NULL, PayCoinSub");
        }
    } else {
        coin_ = 0;
        pay_coin_ = 0;
        event_coin_ = 0;
    }
    return static_cast<int>(coin_);
}

// IDA_EA: 0x84E9990
// IDA_NAME: _ZN10CInventory14SetClearAvatarEii
// DEMANGLED: CInventory::SetClearAvatar(int,int)
void CInventory::SetClearAvatar(int slot, int item_id) {
    if (static_cast<unsigned int>(slot) <
        oldsrv::config::kPersistedClearAvatarCount) {
        clear_avatar_[slot] = item_id;
    }
}

// IDA_EA: 0x84E99B4
// IDA_NAME: _ZN10CInventory14SetClearAvatarEPm
// DEMANGLED: CInventory::SetClearAvatar(unsigned long *)
void CInventory::SetClearAvatar(unsigned long* clear_avatar) {
    for (int slot = 0;
         slot < oldsrv::config::kPersistedClearAvatarCount;
         ++slot) {
        clear_avatar_[slot] = static_cast<int>(clear_avatar[slot]);
    }
}

// IDA_EA: 0x84FB638
// IDA_NAME: _ZNK10CInventory13IsEquipAvatarEv
// DEMANGLED: CInventory::IsEquipAvatar(void)const
int CInventory::IsEquipAvatar() const {
    for (int slot = 0;
         slot < oldsrv::config::kAvatarEquipmentTypeCount;
         ++slot) {
        if (equipment_items_[slot].item_id_ != 0) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x850CD1C
// IDA_NAME: _ZNK10CInventory18get_avatar_slot_noEi
// DEMANGLED: CInventory::get_avatar_slot_no(int)const
int CInventory::get_avatar_slot_no(int item_id) const {
    for (int slot = 0;
         slot < oldsrv::config::kAvatarInventoryItemCount;
         ++slot) {
        if (avatar_items_[slot].item_id_ == item_id) {
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x850CD62
// IDA_NAME: _ZNK10CInventory17get_inven_slot_noEi
// DEMANGLED: CInventory::get_inven_slot_no(int)const
int CInventory::get_inven_slot_no(int item_id) const {
    for (int slot = 0;
         slot < oldsrv::config::kMainInventoryItemCount;
         ++slot) {
        if (stackable_items_[slot].item_id_ == item_id) {
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x850CDAC
// IDA_NAME: _ZNK10CInventory29getEquipWeaponUpgradeSeparateEv
// DEMANGLED: CInventory::getEquipWeaponUpgradeSeparate(void)const
int CInventory::getEquipWeaponUpgradeSeparate() const {
    const Inven_Item& weapon = equipment_items_[kWeaponEquipmentSlot];
    if (weapon.item_id_ == 0) {
        return 0;
    }
    return weapon.upgrade_separate_.GetUpgradeSeparate();
}

// IDA_EA: 0x822F020
// IDA_NAME: _ZN13CAccountCargo8GetMoneyEv
// DEMANGLED: CAccountCargo::GetMoney(void)
int CAccountCargo::GetMoney() {
    return static_cast<int>(money_);
}

// IDA_EA: 0x8289816
// IDA_NAME: _ZN13CAccountCargo9SetDBDataEP5CUserP10Inven_Itemjj
// DEMANGLED: CAccountCargo::SetDBData(CUser *,Inven_Item *,unsigned int,unsigned int)
CAccountCargo* CAccountCargo::SetDBData(CUser* user,
                                        Inven_Item* items,
                                        unsigned int money,
                                        unsigned int capacity) {
    user_ = user;
    capacity_ = capacity;
    money_ = money;
    altered_ = 0u;
    if (items) {
        return reinterpret_cast<CAccountCargo*>(
            memcpy(items_, items, sizeof(items_)));
    }
    return this;
}

// IDA_EA: 0x828986C
// IDA_NAME: _ZN13CAccountCargo5ClearEv
// DEMANGLED: CAccountCargo::Clear(void)
void* CAccountCargo::Clear() {
    user_ = 0;
    capacity_ = 0u;
    money_ = 0u;
    altered_ = 0u;
    return memset(items_, 0, sizeof(items_));
}

// IDA_EA: 0x8289A0C
// IDA_NAME: _ZN13CAccountCargo10GetSlotRefEi
// DEMANGLED: CAccountCargo::GetSlotRef(int)
Inven_Item* CAccountCargo::GetSlotRef(int slot) {
    if (!CheckValidSlot(slot)) {
        return 0;
    }
    altered_ = 1u;
    return &items_[slot];
}

// IDA_EA: 0x828A554
// IDA_NAME: _ZNK13CAccountCargo14CheckValidSlotEi
// DEMANGLED: CAccountCargo::CheckValidSlot(int)const
bool CAccountCargo::CheckValidSlot(int slot) const {
    return slot >= 0 && static_cast<unsigned int>(slot) < capacity_ &&
           slot <= 55;
}

// IDA_EA: 0x828A580
// IDA_NAME: _ZN13CAccountCargo12GetEmptySlotEv
// DEMANGLED: CAccountCargo::GetEmptySlot(void)
int CAccountCargo::GetEmptySlot() {
    const unsigned int end = capacity_ > 56u ? 56u : capacity_;
    for (unsigned int slot = 0u; slot < end; ++slot) {
        if (items_[slot].item_id_ == 0) {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

// IDA_EA: 0x828A5D4
// IDA_NAME: _ZN13CAccountCargo14CheckSlotEmptyEi
// DEMANGLED: CAccountCargo::CheckSlotEmpty(int)
bool CAccountCargo::CheckSlotEmpty(int slot) {
    if (!CheckValidSlot(slot)) {
        return false;
    }
    return items_[slot].item_id_ == 0;
}

// IDA_EA: 0x828A61A
// IDA_NAME: _ZN13CAccountCargo19GetSpecificItemSlotEi
// DEMANGLED: CAccountCargo::GetSpecificItemSlot(int)
int CAccountCargo::GetSpecificItemSlot(int item_id) {
    const unsigned int end = capacity_ > 56u ? 56u : capacity_;
    for (unsigned int slot = 0u; slot < end; ++slot) {
        if (static_cast<uint32_t>(items_[slot].item_id_) ==
            static_cast<uint32_t>(item_id)) {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

// IDA_EA: 0x828A742
// IDA_NAME: _ZN13CAccountCargo8AddMoneyEj
// DEMANGLED: CAccountCargo::AddMoney(unsigned int)
void CAccountCargo::AddMoney(unsigned int money) {
    if (money != 0u) {
        money_ += money;
    }
}

// IDA_EA: 0x828A764
// IDA_NAME: _ZN13CAccountCargo8SubMoneyEj
// DEMANGLED: CAccountCargo::SubMoney(unsigned int)
void CAccountCargo::SubMoney(unsigned int money) {
    if (money != 0u && money_ >= money) {
        money_ -= money;
    }
}

// IDA_EA: 0x828A794
// IDA_NAME: _ZN13CAccountCargo12GetItemCountEv
// DEMANGLED: CAccountCargo::GetItemCount(void)
int CAccountCargo::GetItemCount() {
    int count = 0;
    for (unsigned int slot = 0u; slot < capacity_; ++slot) {
        if (items_[slot].item_id_ != 0) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x84EBE46
// IDA_NAME: _ZN13CAccountCargo11SetCapacityEj
// DEMANGLED: CAccountCargo::SetCapacity(unsigned int)
CAccountCargo* CAccountCargo::SetCapacity(unsigned int capacity) {
    capacity_ = capacity;
    return this;
}
#include "common/config.h"
