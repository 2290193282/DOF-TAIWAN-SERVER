#ifndef OLDSRV_OLD_GAME_INVENTORY_AVATAR_ITEM_ACCESS_H_
#define OLDSRV_OLD_GAME_INVENTORY_AVATAR_ITEM_ACCESS_H_

#include <stdint.h>

struct Inven_Item;

namespace avatar_inventory {

// These are semantic values, not a second physical declaration of
// Inven_Item.  In particular, AvatarUiId and StackCount deliberately remain
// distinct although the original item record stores both meanings at +0x07
// for their respective item variants.
struct AvatarItemId {
    uint32_t value;
};

struct AvatarUiId {
    uint32_t value;
};

struct HiddenOption {
    uint16_t value;
};

struct EmblemEndurance {
    uint16_t value;
};

struct StackCount {
    int32_t value;
};

struct AvatarItemRecord {
    AvatarItemId item_id;
    AvatarUiId ui_id;
    HiddenOption hidden_option;
    EmblemEndurance emblem_endurance;
};

// Reads only the avatar-specific database semantics proved by
// DB_LoadInventory::_GetUserAvatar.  It does not expose or assert a complete
// physical Inven_Item member layout.
AvatarItemRecord ReadAvatarItemRecord(const Inven_Item& item);

// Each writer changes exactly one proven database field.  There is
// intentionally no whole-record assignment API.
void SetAvatarItemId(Inven_Item& item, AvatarItemId item_id);
void SetAvatarUiId(Inven_Item& item, AvatarUiId ui_id);
void SetAvatarHiddenOption(Inven_Item& item, HiddenOption hidden_option);
void SetAvatarEmblemEndurance(Inven_Item& item,
                              EmblemEndurance emblem_endurance);

// Stackable counts use the same old storage word as AvatarUiId but retain the
// original Inven_Item get/set_add_info member calls and a separate semantic
// type, so avatar keys cannot be silently used as item counts.
StackCount ReadStackCount(const Inven_Item& item);
void SetStackCount(Inven_Item& item, StackCount count);

}  // namespace avatar_inventory

#endif  // OLDSRV_OLD_GAME_INVENTORY_AVATAR_ITEM_ACCESS_H_
