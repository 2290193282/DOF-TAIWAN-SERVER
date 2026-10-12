#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include <cstring>
#include <stdint.h>

// IDA_EA: 0x84FB11A
// IDA_NAME: _ZN10CInventory12SetInvenDataEiPvi
// DEMANGLED: CInventory::SetInvenData(int,void *,int)
void* CInventory::SetInvenData(int inventory_type,
                               void* source,
                               int bytes) {
    void* result = reinterpret_cast<void*>(
        static_cast<intptr_t>(inventory_type));
    switch (inventory_type) {
        case 0:
            if (bytes == oldsrv::config::kEquipmentTransferItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(equipment_items_, source, bytes);
            }
            break;
        case 1:
            if (bytes == oldsrv::config::kMainInventoryItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(stackable_items_, source, bytes);
            }
            break;
        case 2:
            if (bytes == oldsrv::config::kAvatarInventoryItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(avatar_items_, source, bytes);
            }
            break;
        case 3:
            if (bytes == oldsrv::config::kCreatureInventoryItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(creature_items_, source, bytes);
            }
            break;
        case 5:
            if (bytes == oldsrv::config::kOrdinaryEquipmentTypeCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(
                    &equipment_items_[
                        oldsrv::config::kOrdinaryEquipmentTypeFirst],
                    source, bytes);
            }
            break;
        default:
            break;
    }
    return result;
}

// IDA_EA: 0x84FB222
// IDA_NAME: _ZNK10CInventory12GetInvenDataEiPvi
// DEMANGLED: CInventory::GetInvenData(int,void *,int)const
void* CInventory::GetInvenData(int inventory_type,
                               void* destination,
                               int bytes) const {
    void* result = reinterpret_cast<void*>(
        static_cast<intptr_t>(inventory_type));
    switch (inventory_type) {
        case 0:
            if (bytes == oldsrv::config::kEquipmentTransferItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(destination, equipment_items_, bytes);
            }
            break;
        case 1:
            if (bytes == oldsrv::config::kMainInventoryItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(destination, stackable_items_, bytes);
            }
            break;
        case 2:
            if (bytes == oldsrv::config::kAvatarInventoryItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(destination, avatar_items_, bytes);
            }
            break;
        case 3:
            if (bytes == oldsrv::config::kCreatureInventoryItemCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(destination, creature_items_, bytes);
            }
            break;
        case 5:
            if (bytes == oldsrv::config::kOrdinaryEquipmentTypeCount *
                             static_cast<int>(sizeof(Inven_Item))) {
                result = std::memcpy(
                    destination,
                    &equipment_items_[
                        oldsrv::config::kOrdinaryEquipmentTypeFirst],
                    bytes);
            }
            break;
        default:
            break;
    }
    return result;
}

// IDA_EA: 0x85002E0
// IDA_NAME: _Z14CheckEquipableRK10Inven_ItemS1_
// DEMANGLED: CheckEquipable(Inven_Item const&,Inven_Item const&)
bool CheckEquipable(const Inven_Item& source, const Inven_Item& target) {
    const unsigned int source_id = static_cast<uint32_t>(source.item_id_);
    if (source_id && source.item_category_ != 1u &&
        source.item_category_ != 8u) {
        return false;
    }
    const unsigned int target_id = static_cast<uint32_t>(target.item_id_);
    if (target_id && target.item_category_ != 1u &&
        target.item_category_ != 8u) {
        return false;
    }
    return source_id || target_id;
}
