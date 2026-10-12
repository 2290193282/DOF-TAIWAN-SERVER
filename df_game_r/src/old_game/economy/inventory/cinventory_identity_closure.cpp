#include "old_game/economy/inventory/inventory.h"
#include "common/config.h"

#include "common/manager/data_manager.h"
#include "common/manager/os_api.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

extern "C" {
int MD5_Init(void* context);
int MD5_Update(void* context, const void* data, unsigned long length);
int MD5_Final(unsigned char* output, void* context);
}

// IDA_EA: 0x84FD6D2
// IDA_NAME: _ZNK10CInventory24MakeSecuRewarditemPacketE10INVEN_TYPEiR11PacketGuard
// DEMANGLED: CInventory::MakeSecuRewarditemPacket(INVEN_TYPE,int,PacketGuard &)const
int CInventory::MakeSecuRewarditemPacket(INVEN_TYPE inventory_type,
                                         int slot,
                                         PacketGuard& packet) const {
    if (static_cast<int>(inventory_type) != 1) {
        return 0;
    }

    const Inven_Item& item = stackable_items_[slot];
    if (item.item_id_ == 0) {
        return 0;
    }

    packet.put_short(slot);
    if (item.item_id_ != 0 && item.add_info_ != 0) {
        packet.put_item_idx(static_cast<unsigned long>(item.item_id_));
    } else {
        packet.put_item_idx(static_cast<unsigned long>(-1));
    }
    packet.put_int(item.add_info_);
    return 1;
}

// IDA_EA: 0x850185E
// IDA_NAME: _ZNK10CInventory14getStartEndPosEN10Inven_Item9ITEM_TYPEERiS2_
// DEMANGLED: CInventory::getStartEndPos(Inven_Item::ITEM_TYPE,int &,int &)const
int CInventory::getStartEndPos(Inven_Item::ITEM_TYPE item_type,
                               int& start,
                               int& end) const {
    return StartEndPos(item_type, start, end);
}

// IDA_EA: 0x850937E
// IDA_NAME: _ZNK10CInventory22isAnyExpiredAvatarItemEv
// DEMANGLED: CInventory::isAnyExpiredAvatarItem(void)const
int CInventory::isAnyExpiredAvatarItem() const {
    const int datetime_tick = OS_API::GetDateTimeTick();
    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kAvatarEquipmentTypeLast;
         ++slot) {
        const Inven_Item& item = equipment_items_[slot];
        if (item.item_id_ != 0 && item.add_info_ != 0 &&
            item.add_info_ < datetime_tick) {
            return 1;
        }
    }

    for (int slot = 0; slot <= 104; ++slot) {
        const Inven_Item& item = avatar_items_[slot];
        if (item.item_id_ != 0 && item.add_info_ != 0 &&
            item.add_info_ < datetime_tick) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8509466
// IDA_NAME: _ZN10CInventory17ExpiredAvatarItemEP11PacketGuard
// DEMANGLED: CInventory::ExpiredAvatarItem(PacketGuard *)
bool CInventory::ExpiredAvatarItem(PacketGuard* packet) {
    // The original body receives but never reads this packet parameter.
    (void)packet;

    const int datetime_tick = OS_API::GetDateTimeTick();
    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kAvatarEquipmentTypeLast;
         ++slot) {
        Inven_Item& item = equipment_items_[slot];
        if (item.item_id_ != 0 && item.add_info_ != 0 &&
            item.add_info_ < datetime_tick) {
            SendAvatarEvent(51, parent_->get_charac_no(-1), slot, 0, 0);
            item.reset();
        }
    }

    for (int slot = 0; slot <= 104; ++slot) {
        Inven_Item& item = avatar_items_[slot];
        if (item.item_id_ != 0 && item.add_info_ != 0 &&
            item.add_info_ < datetime_tick) {
            SendAvatarEvent(
                51,
                parent_->get_charac_no(-1),
                slot + oldsrv::config::kAvatarEquipmentTypeCount,
                0,
                0);
            item.reset();
        }
    }

    // The old function's loop-exit flag is false regardless of whether it
    // removed expired entries.
    return false;
}

// IDA_EA: 0x850A016
// IDA_NAME: _ZNK10CInventory16IsExistStackableEiN10Inven_Item9ITEM_TYPEE
// DEMANGLED: CInventory::IsExistStackable(int,Inven_Item::ITEM_TYPE)const
int CInventory::IsExistStackable(int item_id,
                                 Inven_Item::ITEM_TYPE item_type) const {
    int start = 0;
    int end = 0;
    StartEndPos(item_type, start, end);
    for (int slot = start; slot <= end; ++slot) {
        if (stackable_items_[slot].item_id_ == item_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x850CDD8
// IDA_NAME: _ZN10CInventory13getValidMoneyEi
// DEMANGLED: CInventory::getValidMoney(int)
int CInventory::getValidMoney(int money) {
    const int money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
        parent_->get_charac_level() - 1, parent_->get_acc_name());
    return static_cast<int>(money_) + money > money_limit;
}

// IDA_EA: 0x84FA91C
// IDA_NAME: _ZN10CInventory11getHashCodeEPh
// DEMANGLED: CInventory::getHashCode(unsigned char *)
int CInventory::getHashCode(unsigned char* output) {
    // The original creates 56 dwords and feeds these four exact regions to
    // the incremental legacy MD5 ABI in this order.
    uint32_t context[56];
    MD5_Init(context);
    MD5_Update(context, equipment_items_, sizeof(equipment_items_));
    MD5_Update(
        context,
        stackable_items_,
        oldsrv::config::kMainInventoryItemCount * sizeof(*stackable_items_));
    MD5_Update(
        context,
        avatar_items_,
        oldsrv::config::kAvatarInventoryItemCount * sizeof(*avatar_items_));
    MD5_Update(
        context,
        creature_items_,
        oldsrv::config::kCreatureInventoryItemCount * sizeof(*creature_items_));
    return MD5_Final(output, context);
}
