#include "old_game/player/core/cuser.h"
#include "common/config.h"

#include "common/manager/os_api.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>

namespace {

// This is the exact caller-owned batch written by SetEquipment.  It remains
// source-local because the original producer/consumer type name is not yet
// recovered, while its physical layout is proven by the two indexed writes.
struct ExpiredEquipmentBatch {
    int32_t count;
    int32_t character_no[360];
    uint8_t equipment_slot[360];
};

static_assert(sizeof(ExpiredEquipmentBatch) == 0x70cu,
              "SetEquipment expiry batch must retain IDA size 0x70c");

}  // namespace

// IDA_EA: 0x865DA4E
// IDA_NAME: _ZN5CUser12SetEquipmentEiPviS0_
// DEMANGLED: CUser::SetEquipment(int,void *,int,void *)
void* CUser::SetEquipment(int character_index,
                          void* source,
                          int bytes,
                          void* expired_batch) {
    Inven_Item* const items = static_cast<Inven_Item*>(source);
    ExpiredEquipmentBatch* const batch =
        static_cast<ExpiredEquipmentBatch*>(expired_batch);

    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kAvatarEquipmentTypeLast;
         ++slot) {
        if (items[slot].item_id_ != 0 && items[slot].get_add_info() > 0 &&
            items[slot].get_add_info() < OS_API::GetDateTimeTick()) {
            const int index = batch->count;
            batch->character_no[index] =
                characters_[character_index].fields_.character_no_;
            batch->equipment_slot[index] = static_cast<uint8_t>(slot);
            items[slot].reset();
            ++batch->count;
        }
    }

    return characters_[character_index]
        .fields_.inventory_.SetInvenData(0, source, bytes);
}
