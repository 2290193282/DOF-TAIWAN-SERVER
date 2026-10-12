#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include <stdint.h>
#include <cstring>

#include "old_game/player/core/character_tag_data.h"

namespace {

int GetItemIdByAddInfo(const Inven_Item* items,
                       int item_count,
                       uint32_t add_info) {
    for (int index = 0; index < item_count; ++index) {
        const Inven_Item& item = items[index];
        if (static_cast<uint32_t>(item.add_info_) == add_info && add_info) {
            return item.item_id_;
        }
    }
    return 0;
}

}  // namespace

// IDA_EA: 0x8690226
// IDA_NAME: _ZN5CUser17setTagCharacInvenEP21TAG_CHARAC_INVEN_DATA
// DEMANGLED: CUser::setTagCharacInven(TAG_CHARAC_INVEN_DATA *)
int CUser::setTagCharacInven(TAG_CHARAC_INVEN_DATA* data) {
    static const int kInventoryItemCount = 105;

    if (!data || !getTagCharacR()) {
        return 0;
    }

    CInventory* const inventory = getTagCharacInvenW();
    Inven_Item equipment[oldsrv::config::kEquipmentTransferItemCount] = {};
    std::memcpy(equipment,
                data->avatar_inventory,
                sizeof(data->avatar_inventory));
    inventory->SetInvenData(0, equipment, sizeof(equipment));

    unsigned long clear_avatar[
        oldsrv::config::kPersistedClearAvatarCount] = {};
    std::memcpy(clear_avatar,
                data->clear_avatar,
                sizeof(data->clear_avatar));
    inventory->SetClearAvatar(clear_avatar);

    const uint32_t changed_count = static_cast<uint32_t>(data->avatar_count);
    if (changed_count <=
        static_cast<uint32_t>(oldsrv::config::kAvatarDetailItemCount)) {
        for (uint32_t changed = 0; changed < changed_count; ++changed) {
            const AvatarDetailInfo& detail = data->avatar_detail[changed];
            const uint32_t changed_add_info = detail.clear_avatar_id;
            if (!changed_add_info) {
                continue;
            }

            const int item_id = GetItemIdByAddInfo(
                data->avatar_equipment,
                kInventoryItemCount,
                changed_add_info);
            const uint32_t source_add_info = detail.ui_id;
            for (int slot = 0;
                 slot < oldsrv::config::kPersistedClearAvatarCount;
                 ++slot) {
                if (static_cast<uint32_t>(
                        data->avatar_inventory[slot].add_info_) ==
                    source_add_info) {
                    inventory->SetClearAvatar(slot, item_id);
                }
            }
        }
    }

    const int cargo_capacity = static_cast<int>(data->cargo_capacity);
    getTagCharacCargoW()->SetCapacity(cargo_capacity ? cargo_capacity : 8);
    return 1;
}
