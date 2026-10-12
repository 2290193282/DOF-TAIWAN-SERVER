#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include "old_game/economy/equipment/item_option.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/title_book_runtime.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x84FAC70
// IDA_NAME: _ZNK10CInventory12FindItemLockEhR14ENUM_ITEMSPACERi
// DEMANGLED: CInventory::FindItemLock(unsigned char,ENUM_ITEMSPACE &,int &)const
int CInventory::FindItemLock(unsigned char lock,
                             ENUM_ITEMSPACE& item_space,
                             int& slot) const {
    item_space = static_cast<ENUM_ITEMSPACE>(3);
    for (slot = 0;
         slot < oldsrv::config::kEquippedAvatarAndOrdinaryItemCount;
         ++slot) {
        if (static_cast<unsigned char>(
                equipment_items_[slot].amplify_option_.GetLock()) == lock) {
            return 1;
        }
    }

    item_space = static_cast<ENUM_ITEMSPACE>(0);
    for (slot = 0;
         slot < oldsrv::config::kMainInventoryItemCount;
         ++slot) {
        if (static_cast<unsigned char>(
                stackable_items_[slot].amplify_option_.GetLock()) == lock) {
            return 1;
        }
    }

    item_space = static_cast<ENUM_ITEMSPACE>(7);
    for (slot = 0;
         slot < oldsrv::config::kCreatureInventoryItemCount;
         ++slot) {
        if (static_cast<unsigned char>(
                creature_items_[slot].amplify_option_.GetLock()) == lock) {
            return 1;
        }
    }

    item_space = static_cast<ENUM_ITEMSPACE>(1);
    for (slot = 0;
         slot < oldsrv::config::kAvatarInventoryItemCount;
         ++slot) {
        if (static_cast<unsigned char>(
                avatar_items_[slot].amplify_option_.GetLock()) == lock) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x850C836
// IDA_NAME: _ZNK6CCargo12FindItemLockEhR14ENUM_ITEMSPACERi
// DEMANGLED: CCargo::FindItemLock(unsigned char,ENUM_ITEMSPACE &,int &)const
int CCargo::FindItemLock(unsigned char lock,
                         ENUM_ITEMSPACE& item_space,
                         int& slot) const {
    item_space = static_cast<ENUM_ITEMSPACE>(2);
    for (slot = 0; slot <= 151; ++slot) {
        if (static_cast<unsigned char>(
                cargo_[slot].amplify_option_.GetLock()) == lock) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x850BD50
// IDA_NAME: _ZNK6CCargo16SendItemLockListEv
// DEMANGLED: CCargo::SendItemLockList(void)const
void CCargo::SendItemLockList() const {
    if (!owner_) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 251);
    int count_index = packet.get_index();
    int count = 0;
    packet.put_short(0);

    for (int slot = 0; slot <= 151; ++slot) {
        const unsigned char lock = static_cast<unsigned char>(
            cargo_[slot].amplify_option_.GetLock());
        if (!lock) {
            continue;
        }

        packet.put_byte(2);
        packet.put_short(slot);
        item_lock::CItemLock* const lock_data =
            reinterpret_cast<item_lock::CItemLock*>(
                owner_->GetCharacExpandDataR(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
        lock_data->MakeItemLockPacket(packet, lock);
        ++count;
    }

    packet.put_byte(count_index, count);
    packet.finalize(true);
    owner_->Send(packet);
}

// IDA_EA: 0x8641124
// IDA_NAME: _ZNK10CTitleBook12findItemLockEhR14ENUM_ITEMSPACERi
// DEMANGLED: CTitleBook::findItemLock(unsigned char,ENUM_ITEMSPACE &,int &)const
int CTitleBook::findItemLock(unsigned char lock,
                             ENUM_ITEMSPACE& item_space,
                             int& slot) const {
    const int logical_max[5] = {70, 140, 50, 100, 40};
    for (int category = 0; category < 5; ++category) {
        for (int index = 0; index < logical_max[category]; ++index) {
            int physical_category = category;
            int physical_slot = index;
            if (category == 2) {
                physical_category = 1;
                physical_slot = 140 + index;
            } else if (category >= 3) {
                physical_category = category - 1;
            }
            if (static_cast<unsigned char>(
                    category_slots_[physical_category][physical_slot]
                        .amplify_option_.GetLock()) ==
                lock) {
                item_space = static_cast<ENUM_ITEMSPACE>(category + 19);
                slot = index;
                return 1;
            }
        }
    }
    return 0;
}
