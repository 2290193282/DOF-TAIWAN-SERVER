#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/equipment/item_option.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <stdint.h>

// IDA_EA: 0x84FA4AC
// IDA_NAME: _ZN10CInventory19getPackedCodeArray1EPmi
// DEMANGLED: CInventory::getPackedCodeArray1(unsigned long *,int)
int CInventory::getPackedCodeArray1(unsigned long* codes, int capacity) {
    int count = 0;
    const Inven_Item* arrays[] = {
        equipment_items_, stackable_items_, avatar_items_, creature_items_};
    const int sizes[] = {
        oldsrv::config::kEquipmentTypeCount,
        oldsrv::config::kMainInventoryItemCount,
        oldsrv::config::kAvatarInventoryItemCount,
        oldsrv::config::kCreatureInventoryItemCount};
    for (int group = 0; group < 4; ++group) {
        for (int slot = 0; slot < sizes[group]; ++slot) {
            const Inven_Item& item = arrays[group][slot];
            if (!item.item_id_)
                continue;
            codes[count++] = item.getPackedCode1();
            if (count >= capacity) return count;
        }
    }
    return count;
}

// IDA_EA: 0x84FA68A
// IDA_NAME: _ZN10CInventory17getSortedKeyArrayEPmi
// DEMANGLED: CInventory::getSortedKeyArray(unsigned long *,int)
int CInventory::getSortedKeyArray(unsigned long* keys, int capacity) {
    std::fill(keys, keys + capacity, 0ul);
    int count = 0;
    const Inven_Item* arrays[] = {
        equipment_items_, stackable_items_, avatar_items_, creature_items_};
    const int sizes[] = {
        oldsrv::config::kEquipmentTypeCount,
        oldsrv::config::kMainInventoryItemCount,
        oldsrv::config::kAvatarInventoryItemCount,
        oldsrv::config::kCreatureInventoryItemCount};
    for (int group = 0; group < 4 && count < capacity; ++group) {
        for (int slot = 0; slot < sizes[group] && count < capacity; ++slot) {
            const Inven_Item& item = arrays[group][slot];
            if (item.item_id_)
                keys[count++] = item.getKey();
        }
    }
    std::sort(keys, keys + count);
    return count;
}

// IDA_EA: 0x84FAE0A
// IDA_NAME: _ZNK10CInventory16SendItemLockListEPK10Inven_Itemi14ENUM_ITEMSPACE
// DEMANGLED: CInventory::SendItemLockList(Inven_Item const*,int,ENUM_ITEMSPACE)const
void CInventory::SendItemLockList(const Inven_Item* items,
                                  int item_count,
                                  ENUM_ITEMSPACE item_space) const {
    CUser* const user = parent_;
    if (!user) return;
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 251);
    int count_index = packet.get_index();
    int locked_count = 0;
    packet.put_short(0);
    for (int slot = 0; slot < item_count; ++slot) {
        const unsigned char lock =
            static_cast<unsigned char>(items[slot].amplify_option_.GetLock());
        if (!lock) continue;
        packet.put_byte(static_cast<int>(item_space));
        packet.put_short(slot);
        const item_lock::CItemLock* const item_lock_data =
            reinterpret_cast<const item_lock::CItemLock*>(
                user->GetCharacExpandDataR(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
        if (item_lock_data) {
            item_lock_data->MakeItemLockPacket(packet, lock);
            ++locked_count;
        }
    }
    packet.put_byte(count_index, locked_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x84FAF8E
// IDA_NAME: _ZNK10CInventory21SendItemLockListInvenEv
// DEMANGLED: CInventory::SendItemLockListInven(void)const
void CInventory::SendItemLockListInven() const {
    SendItemLockList(stackable_items_,
                     oldsrv::config::kMainInventoryItemCount,
                     static_cast<ENUM_ITEMSPACE>(0));
}

// IDA_EA: 0x84FAFBE
// IDA_NAME: _ZNK10CInventory22SendItemLockListAvatarEv
// DEMANGLED: CInventory::SendItemLockListAvatar(void)const
void CInventory::SendItemLockListAvatar() const {
    SendItemLockList(avatar_items_,
                     oldsrv::config::kAvatarInventoryItemCount,
                     static_cast<ENUM_ITEMSPACE>(1));
}

// IDA_EA: 0x84FAFEE
// IDA_NAME: _ZNK10CInventory24SendItemLockListCreatureEv
// DEMANGLED: CInventory::SendItemLockListCreature(void)const
void CInventory::SendItemLockListCreature() const {
    SendItemLockList(creature_items_,
                     oldsrv::config::kCreatureInventoryItemCount,
                     static_cast<ENUM_ITEMSPACE>(7));
}

// IDA_EA: 0x84FB32A
// IDA_NAME: _ZN10CInventory9ResetSlotEii
// DEMANGLED: CInventory::ResetSlot(int,int)
Inven_Item* CInventory::ResetSlot(int inventory_type, int slot) {
    if (CheckValidSlot(inventory_type, slot) != 1) return 0;
    Inven_Item* items = 0;
    switch (inventory_type) {
    case 0:
        items = equipment_items_;
        break;
    case 1:
        items = stackable_items_;
        break;
    case 2:
        items = avatar_items_;
        break;
    case 3:
        items = creature_items_;
        break;
    default:
        break;
    }
    return items ? items[slot].reset() : 0;
}

// IDA_EA: 0x84FB3E4
// IDA_NAME: _ZNK10CInventory14get_item_countEi
// DEMANGLED: CInventory::get_item_count(int)const
int CInventory::get_item_count(int inventory_type) const {
    const Inven_Item* items = 0;
    switch (inventory_type) {
    case 0:
        items = equipment_items_;
        break;
    case 1:
        items = stackable_items_;
        break;
    case 2:
        items = avatar_items_;
        break;
    case 3:
        items = creature_items_;
        break;
    default:
        break;
    }
    if (!items) return 0;
    const int first = inventory_type == 1 ? 3 : 0;
    const int last = inventory_type == 0 ?
                     oldsrv::config::kOrdinaryEquipmentTypeLast :
                     inventory_type == 1 ?
                         oldsrv::config::kMainInventoryLastSlot :
                     inventory_type == 2 ?
                         oldsrv::config::kAvatarInventoryItemCount - 1 :
                         oldsrv::config::kCreatureInventoryItemCount - 1;
    int count = 0;
    for (int slot = first; slot <= last; ++slot)
        if (items[slot].item_id_)
            ++count;
    return count;
}

// IDA_EA: 0x84FB500
// IDA_NAME: _ZNK10CInventory14get_item_countEim
// DEMANGLED: CInventory::get_item_count(int,unsigned long)const
int CInventory::get_item_count(int inventory_type,
                               unsigned long item_id) const {
    const Inven_Item* items = 0;
    switch (inventory_type) {
    case 0:
        items = equipment_items_;
        break;
    case 1:
        items = stackable_items_;
        break;
    case 2:
        items = avatar_items_;
        break;
    case 3:
        items = creature_items_;
        break;
    default:
        break;
    }
    if (!items) return 0;
    const int first = inventory_type == 1 ? 3 : 0;
    const int last = inventory_type == 0 ?
                     oldsrv::config::kOrdinaryEquipmentTypeLast :
                     inventory_type == 1 ?
                         oldsrv::config::kMainInventoryLastSlot :
                     inventory_type == 2 ?
                         oldsrv::config::kAvatarInventoryItemCount - 1 :
                         oldsrv::config::kCreatureInventoryItemCount - 1;
    int count = 0;
    for (int slot = first; slot <= last; ++slot) {
        if (static_cast<unsigned int>(items[slot].item_id_) != item_id)
            continue;
        count += inventory_type == 1
                     ? items[slot].add_info_
                     : 1;
    }
    return count;
}

// IDA_EA: 0x84FBC5A
// IDA_NAME: _ZN10CInventory17GetInvenSlotByRefEiiR10Inven_Item
// DEMANGLED: CInventory::GetInvenSlotByRef(int,int,Inven_Item &)
int CInventory::GetInvenSlotByRef(int inventory_type,
                                  int slot,
                                  Inven_Item& item) {
    if (CheckValidSlot(inventory_type, slot) != 1) return 0;
    const Inven_Item* const source = GetInvenRef(inventory_type, slot);
    if (!source) return 0;
    item.setCopy(*source);
    return 1;
}

// IDA_EA: 0x84FF8B4
// IDA_NAME: _ZN10CInventory15UpdateDailyCoinEi
// DEMANGLED: CInventory::UpdateDailyCoin(int)
CUserCharacInfo* CInventory::UpdateDailyCoin(int daily_coin) {
    const int old_coin = coin_;
    if (old_coin < daily_coin) {
        coin_ = static_cast<uint32_t>(daily_coin);
        CUserCharacInfo* const charac_info =
            reinterpret_cast<CUserCharacInfo*>(parent_);
        return charac_info ? charac_info->enableSaveInven() : 0;
    }
    return reinterpret_cast<CUserCharacInfo*>(
        static_cast<uintptr_t>(static_cast<unsigned int>(old_coin)));
}

// IDA_EA: 0x85049C8
// IDA_NAME: _ZN10CInventory30MoveItemToEmptySlotOfInventoryEiii
// DEMANGLED: CInventory::MoveItemToEmptySlotOfInventory(int,int,int)
int CInventory::MoveItemToEmptySlotOfInventory(int source_slot,
                                               int begin_slot,
                                               int end_slot) {
    Inven_Item* const items = stackable_items_;
    int empty_slot = -1;
    for (int slot = begin_slot; slot <= end_slot; ++slot) {
        if (!items[slot].item_id_) {
            empty_slot = slot;
            break;
        }
    }
    if (empty_slot == -1) return 22;
    Inven_Item item;
    item.setCopy(items[source_slot]);
    items[source_slot].reset();
    items[empty_slot].setCopy(item);
    return 0;
}

// IDA_EA: 0x8504A8A
// IDA_NAME: _ZN10CInventory13SortInvenSlotEN10Inven_Item9ITEM_TYPEE
// DEMANGLED: CInventory::SortInvenSlot(Inven_Item::ITEM_TYPE)
int CInventory::SortInvenSlot(Inven_Item::ITEM_TYPE item_type) {
    int begin_slot = 0;
    int end_slot = 0;
    StartEndPos(item_type, begin_slot, end_slot);
    Inven_Item* const items = stackable_items_;
    int last_used = 0;
    for (int slot = end_slot; slot >= begin_slot; --slot) {
        if (items[slot].item_id_) {
            last_used = slot;
            break;
        }
    }
    if (last_used == begin_slot || !last_used) return 18;
    bool already_compact = true;
    for (int slot = begin_slot; slot <= last_used; ++slot) {
        if (!items[slot].item_id_) {
            already_compact = false;
            break;
        }
    }
    if (already_compact) return 18;
    for (int slot = end_slot; slot >= begin_slot; --slot) {
        if (!items[slot].item_id_)
            continue;
        const int move_result =
            MoveItemToEmptySlotOfInventory(slot, begin_slot, end_slot);
        if (move_result) {
            cMyTrace trace(
                "int CInventory::SortInvenSlot(Inven_Item::ITEM_TYPE)",
                6477, 5);
            trace("MoveItemToEmptySlotOfInventory return %d ( index %d,s %d,e %d )",
                  move_result, slot, begin_slot, end_slot);
            return move_result;
        }
        int current_last = 0;
        for (int probe = end_slot; probe >= begin_slot; --probe) {
            if (items[probe].item_id_) {
                current_last = probe;
                break;
            }
        }
        bool compact = true;
        for (int probe = begin_slot; probe <= current_last; ++probe) {
            if (!items[probe].item_id_) {
                compact = false;
                break;
            }
        }
        if (compact) break;
    }
    return 0;
}

// IDA_EA: 0x8504F24
// IDA_NAME: _ZNK10CInventory16check_slot_emptyEi
// DEMANGLED: CInventory::check_slot_empty(int)const
bool CInventory::check_slot_empty(int slot) const {
    return slot <= oldsrv::config::kMainInventoryLastSlot && slot > 0 &&
           !stackable_items_[slot].item_id_;
}

// IDA_EA: 0x85052BE
// IDA_NAME: _ZNK10CInventory36CheckExistItemInInventoryOrEquipmentEiRiRb
// DEMANGLED: CInventory::CheckExistItemInInventoryOrEquipment(int,int &,bool &)const
int CInventory::CheckExistItemInInventoryOrEquipment(int item_id,
                                                      int& slot,
                                                      bool& equipment) const {
    equipment = false;
    const Inven_Item* const inventory = stackable_items_;
    for (int index = 3; index <= 8; ++index) {
        if (static_cast<unsigned int>(inventory[index].item_id_) ==
            static_cast<unsigned int>(item_id)) {
            slot = index;
            return 1;
        }
    }
    const int item_type = GetItemType(item_id);
    if (!item_type) {
        LogManager::logFormat(
            1, "inventory.cpp",
            "bool CInventory::CheckExistItemInInventoryOrEquipment(int, int&, bool&) const",
            0x1ACE, "Unknown Type %d", item_id);
        return 0;
    }
    const Inven_Item* const equips = equipment_items_;
    for (int index = 0;
         index < oldsrv::config::kEquipmentTypeCount;
         ++index) {
        if (static_cast<unsigned int>(equips[index].item_id_) ==
            static_cast<unsigned int>(item_id)) {
            equipment = true;
            slot = index;
            return 1;
        }
    }
    int begin_slot = 0;
    int end_slot = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type), begin_slot,
                end_slot);
    for (int index = begin_slot; index <= end_slot; ++index) {
        if (static_cast<unsigned int>(inventory[index].item_id_) ==
            static_cast<unsigned int>(item_id)) {
            slot = index;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x850542A
// IDA_NAME: _ZNK10CInventory30ShowAllExistItemsForErrorCheckEv
// DEMANGLED: CInventory::ShowAllExistItemsForErrorCheck(void)const
bool CInventory::ShowAllExistItemsForErrorCheck() const {
    const Inven_Item* const inventory = stackable_items_;
    for (int slot = 3;
         slot <= oldsrv::config::kMainInventoryLastSlot;
         ++slot) {
        const unsigned int item_id =
            static_cast<unsigned int>(inventory[slot].item_id_);
        if (item_id) {
            LogManager::logFormat(
                1, "inventory.cpp",
                "void CInventory::ShowAllExistItemsForErrorCheck() const",
                0x1AF0, "Inventory(%d) : %d\n", slot, item_id);
        }
    }
    const Inven_Item* const equips = equipment_items_;
    for (int slot = 0;
         slot < oldsrv::config::kEquipmentTypeCount;
         ++slot) {
        const unsigned int item_id =
            static_cast<unsigned int>(equips[slot].item_id_);
        if (item_id) {
            LogManager::logFormat(
                1, "inventory.cpp",
                "void CInventory::ShowAllExistItemsForErrorCheck() const",
                0x1AF6, "Equip(%d) : %d\n", slot, item_id);
        }
    }
    return false;
}

// IDA_EA: 0x850562A
// IDA_NAME: _ZNK10CInventory18possible_to_insertEi
// DEMANGLED: CInventory::possible_to_insert(int)const
bool CInventory::possible_to_insert(int item_id) const {
    const int item_type = GetItemType(item_id);
    if (!item_type) return false;
    if (check_empty_count(static_cast<Inven_Item::ITEM_TYPE>(item_type), 1))
        return true;
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item || !item->is_stackable()) return false;
    return check_item_exist(item_id) >= 0;
}

// IDA_EA: 0x8509880
// IDA_NAME: _ZN10CInventory20ChecknFixWineAndMoruEv
// DEMANGLED: CInventory::ChecknFixWineAndMoru(void)
bool CInventory::ChecknFixWineAndMoru() {
    Inven_Item* const inventory = stackable_items_;
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuickSlotLast;
         ++slot) {
        Inven_Item& item = inventory[slot];
        const unsigned int item_id = static_cast<unsigned int>(item.item_id_);
        if (item_id && item.item_category_ != 1 &&
            IsOverStackableItem(static_cast<int>(item_id), item.item_category_) != 1 &&
            item.add_info_ > 799) {
            item.add_info_ = 1;
        }
    }
    for (int slot = oldsrv::config::kMainInventoryConsumableSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuestSlotLast;
         ++slot) {
        Inven_Item& item = inventory[slot];
        const unsigned int item_id = static_cast<unsigned int>(item.item_id_);
        if (item_id &&
            IsOverStackableItem(static_cast<int>(item_id), item.item_category_) != 1 &&
            item.add_info_ > 799) {
            item.add_info_ = 1;
        }
    }
    return false;
}
#include "common/config.h"
