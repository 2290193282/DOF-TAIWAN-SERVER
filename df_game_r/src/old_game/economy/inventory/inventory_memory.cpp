#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include <string.h>

typedef char InventoryMemory_Size_Must_Be_0xCAC7[
    (sizeof(InventoryMemory) == 0xCAC7) ? 1 : -1];

// IDA_EA: 0x822D6C6
// IDA_NAME: _ZN10CInventory10set_parentEP5CUser
// DEMANGLED: CInventory::set_parent(CUser *)
user_creature::CCreatureMgr* CInventory::set_parent(CUser* user) {
    parent_ = user;
    return creature_manager_.SetUser(user);
}

// IDA_EA: 0x86949FE
// IDA_NAME: _ZN6CCargo10set_parentEP5CUser
// DEMANGLED: CCargo::set_parent(CUser *)
void* CCargo::set_parent(CUser* user) {
    owner_ = user;
    return this;
}

// IDA_EA: 0x822D4F6
// IDA_NAME: _ZN10CInventory18SetInventoryMemoryEP15InventoryMemory
// DEMANGLED: CInventory::SetInventoryMemory(InventoryMemory *)
// The original ABI returns this even though early declarations used void *.
CInventory* CInventory::SetInventoryMemory(InventoryMemory* memory) {
    stackable_items_ = memory->inventory_items;
    avatar_items_ = memory->avatar_items;
    creature_items_ = memory->creature_items;
    return this;
}

// IDA_EA: 0x822D52C
// IDA_NAME: _ZN10CInventoryC2Ev
// DEMANGLED: CInventory::CInventory(void)
CInventory::CInventory()
    : stackable_items_(0),
      avatar_items_(0),
      avatar_coin_(),
      creature_items_(0),
      creature_manager_(),
      avatar_item_manager_() {
    reset();
}

// IDA_EA: 0x8699476
// IDA_NAME: _ZN10CInventoryC2ERKS_
// DEMANGLED: CInventory::CInventory(CInventory const&)
CInventory::CInventory(const CInventory& other)
    : creature_manager_(other.creature_manager_),
      avatar_item_manager_(other.avatar_item_manager_) {
    // Restore the complete physical prefix by member identity.  This is the
    // same prefix copied by the optimized old C2 before the two manager copy
    // constructors, without encoding its end as an object byte offset.
    parent_ = other.parent_;
    money_ = other.money_;
    coin_ = other.coin_;
    pay_coin_ = other.pay_coin_;
    event_coin_ = other.event_coin_;
    reset_state_014_ = other.reset_state_014_;
    inventory_capacity_ = other.inventory_capacity_;
    for (unsigned int index = 0;
         index < oldsrv::config::kEquipmentTypeCount;
         ++index) {
        equipment_items_[index] = other.equipment_items_[index];
    }
    stackable_items_ = other.stackable_items_;
    avatar_items_ = other.avatar_items_;
    avatar_coin_.coin_count = other.avatar_coin_.coin_count;
    for (unsigned int index = 0;
         index < oldsrv::config::kPersistedClearAvatarCount;
         ++index) {
        clear_avatar_[index] = other.clear_avatar_[index];
    }
    for (unsigned int index = 0;
         index < oldsrv::config::kCharacterViewEquipmentTypeCount;
         ++index) {
        avatar_visible_data_[index] = other.avatar_visible_data_[index];
    }
    creature_items_ = other.creature_items_;
}

// IDA_EA: 0x822D600
// IDA_NAME: _ZN10CInventoryD2Ev
// DEMANGLED: CInventory::~CInventory(void)
CInventory::~CInventory() {
}

// IDA_EA: 0x869D73C
// IDA_NAME: _ZN10CInventoryaSERKS_
// DEMANGLED: CInventory::operator=(CInventory const&)
CInventory& CInventory::operator=(const CInventory& other) {
    if (this != &other) {
        // Preserve the old assignment split, but name every prefix member.
        // setCopy() remains intentionally unsuitable because it performs a
        // different gameplay copy of the backing inventory contents.
        parent_ = other.parent_;
        money_ = other.money_;
        coin_ = other.coin_;
        pay_coin_ = other.pay_coin_;
        event_coin_ = other.event_coin_;
        reset_state_014_ = other.reset_state_014_;
        inventory_capacity_ = other.inventory_capacity_;
        for (unsigned int index = 0;
             index < oldsrv::config::kEquipmentTypeCount;
             ++index) {
            equipment_items_[index] = other.equipment_items_[index];
        }
        stackable_items_ = other.stackable_items_;
        avatar_items_ = other.avatar_items_;
        avatar_coin_.coin_count = other.avatar_coin_.coin_count;
        for (unsigned int index = 0;
             index < oldsrv::config::kPersistedClearAvatarCount;
             ++index) {
            clear_avatar_[index] = other.clear_avatar_[index];
        }
        for (unsigned int index = 0;
             index < oldsrv::config::kCharacterViewEquipmentTypeCount;
             ++index) {
            avatar_visible_data_[index] = other.avatar_visible_data_[index];
        }
        creature_items_ = other.creature_items_;
        creature_manager_ = other.creature_manager_;
        avatar_item_manager_ = other.avatar_item_manager_;
    }
    return *this;
}

// IDA_EA: 0x8509A06
// IDA_NAME: _ZN10CInventory5resetEv
// DEMANGLED: CInventory::reset(void)
bool CInventory::reset() {
    parent_ = 0;
    money_ = 0;
    coin_ = 1;
    pay_coin_ = 0;
    event_coin_ = 0;
    reset_state_014_ = 0;
    memset(equipment_items_, 0, sizeof(equipment_items_));

    if (stackable_items_) {
        memset(stackable_items_,
               0,
               oldsrv::config::kMainInventoryItemCount *
                   sizeof(*stackable_items_));
    }
    if (avatar_items_) {
        memset(avatar_items_,
               0,
               oldsrv::config::kAvatarInventoryItemCount *
                   sizeof(*avatar_items_));
    }
    if (creature_items_) {
        memset(creature_items_,
               0,
               oldsrv::config::kCreatureInventoryItemCount *
                   sizeof(*creature_items_));
    }

    creature_manager_.Reset();
    creature_manager_.SetUser(parent_);
    avatar_item_manager_.Reset();

    memset(clear_avatar_, 0, sizeof(clear_avatar_));
    inventory_capacity_ = 0;
    avatar_coin_.Set(0);
    for (unsigned int index = 0;
         index < oldsrv::config::kCharacterViewEquipmentTypeCount;
         ++index) {
        avatar_visible_data_[index].init();
    }
    // The historical bool body returns false after a completed reset.
    return false;
}

// IDA_EA: 0x84FA314
// IDA_NAME: _ZN10CInventory7setCopyERKS_
// DEMANGLED: CInventory::setCopy(CInventory const&)
CInventory* CInventory::setCopy(const CInventory& source) {
    parent_ = source.parent_;
    money_ = source.money_;
    coin_ = source.coin_;
    pay_coin_ = source.pay_coin_;
    event_coin_ = source.event_coin_;
    reset_state_014_ = source.reset_state_014_;
    for (unsigned int index = 0;
         index < oldsrv::config::kEquipmentTypeCount;
         ++index) {
        equipment_items_[index].setCopy(source.equipment_items_[index]);
    }

    Inven_Item* const target_inventory = stackable_items_;
    const Inven_Item* const source_inventory = source.stackable_items_;
    for (unsigned int index = 0;
         index < oldsrv::config::kMainInventoryItemCount;
         ++index) {
        target_inventory[index].setCopy(source_inventory[index]);
    }

    for (unsigned int index = 0;
         index < oldsrv::config::kAvatarInventoryItemCount;
         ++index) {
        avatar_items_[index].setCopy(source.avatar_items_[index]);
    }

    creature_manager_.SetUser(parent_);

    for (unsigned int index = 0;
         index < oldsrv::config::kCreatureInventoryItemCount;
         ++index) {
        creature_items_[index].setCopy(source.creature_items_[index]);
    }

    inventory_capacity_ = source.get_inventory_capacity();
    return this;
}

// IDA_EA: 0x86949C6
// IDA_NAME: _ZN6CCargo18SetInventoryMemoryEP15InventoryMemory
// DEMANGLED: CCargo::SetInventoryMemory(InventoryMemory *)
void* CCargo::SetInventoryMemory(InventoryMemory* memory) {
    cargo_ = memory->cargo_items;
    return this;
}

// IDA_EA: 0x822D3B8
// IDA_NAME: _ZN15InventoryMemoryC2Ev
// DEMANGLED: InventoryMemory::InventoryMemory(void)
InventoryMemory::InventoryMemory() {
    // The member-array construction that precedes this body emits the target
    // order: 352 inventory, 105 avatar, 242 creature, then 152 cargo records.
    // The original constructor next invokes reset().
    reset();
}

// IDA_EA: 0x822D472
// IDA_NAME: _ZN15InventoryMemory5resetEv
// DEMANGLED: InventoryMemory::reset(void)
void* InventoryMemory::reset() {
    memset(inventory_items, 0, sizeof(inventory_items));
    memset(avatar_items, 0, sizeof(avatar_items));
    memset(creature_items, 0, sizeof(creature_items));
    return memset(cargo_items, 0, sizeof(cargo_items));
}
