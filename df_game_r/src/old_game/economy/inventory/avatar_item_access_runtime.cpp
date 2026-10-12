#include "old_game/economy/inventory/avatar_item_access.h"

#include <cstring>
#include <stddef.h>

#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/economy/inventory/inventory.h"

namespace {

// DB_LoadInventory::_GetUserAvatar @ 0x840BE8A materializes these exact
// members before writing the 0x3d-byte Inven_Item record.  Keep the field
// proof with the inventory owner rather than duplicating a record view in the
// avatar gameplay state machine.
static_assert(offsetof(InventoryItemDbFields, item_id) == 0x02u,
              "old avatar item_id field must remain at +0x02");
static_assert(offsetof(InventoryItemDbFields, ui_id) == 0x07u,
              "old avatar ui_id field must remain at +0x07");
static_assert(offsetof(InventoryItemDbFields, hidden_option) == 0x0du,
              "old avatar hidden option must remain at +0x0d");
static_assert(offsetof(InventoryItemDbFields, emblem_endurance) == 0x0fu,
              "old avatar emblem endurance must remain at +0x0f");

}  // namespace

namespace avatar_inventory {

AvatarItemRecord ReadAvatarItemRecord(const Inven_Item& item) {
    AvatarItemRecord record;
    const uint32_t extend_info = static_cast<uint32_t>(item.extend_info_);
    record.item_id.value = static_cast<uint32_t>(item.item_id_);
    record.ui_id.value = static_cast<uint32_t>(item.add_info_);
    record.hidden_option.value = static_cast<uint16_t>(extend_info);
    record.emblem_endurance.value =
        static_cast<uint16_t>(extend_info >> 16);
    return record;
}

void SetAvatarItemId(Inven_Item& item, AvatarItemId item_id) {
    item.item_id_ = static_cast<int32_t>(item_id.value);
}

void SetAvatarUiId(Inven_Item& item, AvatarUiId ui_id) {
    item.add_info_ = static_cast<int32_t>(ui_id.value);
}

void SetAvatarHiddenOption(Inven_Item& item, HiddenOption hidden_option) {
    const uint32_t extend_info = static_cast<uint32_t>(item.extend_info_);
    item.extend_info_ = static_cast<int32_t>(
        (extend_info & 0xffff0000u) |
        static_cast<uint32_t>(hidden_option.value));
}

void SetAvatarEmblemEndurance(Inven_Item& item,
                              EmblemEndurance emblem_endurance) {
    const uint32_t extend_info = static_cast<uint32_t>(item.extend_info_);
    item.extend_info_ = static_cast<int32_t>(
        (extend_info & 0x0000ffffu) |
        (static_cast<uint32_t>(emblem_endurance.value) << 16));
}

StackCount ReadStackCount(const Inven_Item& item) {
    StackCount count;
    count.value = item.get_add_info();
    return count;
}

void SetStackCount(Inven_Item& item, StackCount count) {
    item.set_add_info(count.value);
}

}  // namespace avatar_inventory
