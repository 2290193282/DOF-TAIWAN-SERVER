#ifndef OLDSRV_DB_LOAD_INVENTORY_PAYLOAD_H_
#define OLDSRV_DB_LOAD_INVENTORY_PAYLOAD_H_

#include "common/config.h"
#include "old_data/db/inventory/expand_equipslot_payload.h"
#include "old_data/db/creature/creature_item_detail_payload.h"
#include "old_game/player/core/character_tag_data.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_data/db/character/tag_charac_info_payload.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/economy/inventory/inventory.h"

#include <stddef.h>
#include <stdint.h>


// IDA_EA: 0x844D406. DB_LoadInventory::GetItemLock @0x840D766
// validates this record with an eight-byte modulus.
struct stItemLockInfo {
    stItemLockInfo();

    uint32_t lock_time;             // +0x00
    uint8_t lock_type;              // +0x04
    uint8_t lock_state;             // +0x05
    uint8_t lock_reason;            // +0x06
    uint8_t lock_owner;             // +0x07
};

// IDA_EA: 0x848F976. CCreatureMgr::LoadCreatureTempPeriod consumes the
// creature id followed by the absolute expiration time.
struct SIG_POSTAL_CREATURE_TEMP_PERIOD {
    SIG_POSTAL_CREATURE_TEMP_PERIOD();

    int32_t creature_id;            // +0x00
    int32_t expiration_time;        // +0x04
};

// IDA_EA: 0x848F9D0/0x848F9E4/0x8491CF2/0x8694CD2.
// CUser::GetDimensionRentInfo @0x8676BEC writes these exact eight-byte rows.
struct SIG_SAVE_DIMENSION_RENT_INFO {
    SIG_SAVE_DIMENSION_RENT_INFO();
    SIG_SAVE_DIMENSION_RENT_INFO* clear();

    uint32_t item_id;               // +0x00
    uint8_t rental_type;            // +0x04
    uint8_t inventory_slot;         // +0x05
    uint8_t equipped;               // +0x06
    uint8_t rental_state;           // +0x07
};

struct SIG_SAVE_DIMENSION_RENT_ITEM {
    SIG_SAVE_DIMENSION_RENT_ITEM();
    bool clear();

    SIG_SAVE_DIMENSION_RENT_INFO items[11];
};

#pragma pack(push, 1)

// Semantic database view of the canonical 0x3D Inven_Item. Every offset is
// read or written by DB_LoadInventory::_GetUserAvatar @0x840BE8A or
// DB_LoadInventory::GetUserCreature @0x840CA74. The remaining subrecords use
// their canonical public types instead of anonymous byte storage.
struct InventoryItemDbFields {
    uint8_t item_state;                         // +0x00
    uint8_t item_type;                          // +0x01
    uint32_t item_id;                           // +0x02
    uint8_t item_attribute;                     // +0x06
    uint32_t ui_id;                             // +0x07
    uint16_t endurance_or_ability;              // +0x0B
    uint16_t hidden_option;                     // +0x0D
    uint16_t emblem_endurance;                  // +0x0F
    stAmplifyOption_t amplify_option;            // +0x11
    itemGloballyUniqueIdentifier_t unique_id;   // +0x15
    ChronicleRandomOptionInfo chronicle_option; // +0x1F
    uint8_t upgrade_separate_flags;             // +0x33
    ReservedCapacity reserved_capacity;         // +0x34
};

#pragma pack(pop)

// IDA_EA: 0x8491D26. Stream::GetOutBuffer<SIG_LOAD_INVENTORY>
// @0x84507F2 uses the complete target inventory ABI extent.
// Natural outer alignment is preserved without guessed padding arrays.
struct SIG_LOAD_INVENTORY {
    SIG_LOAD_INVENTORY();

    uint32_t account_id;                         // +0x00000
    uint32_t charac_no;                          // +0x00004
    uint32_t tag_charac_no;                      // +0x00008
    TAG_CHARAC_INVEN_DATA tag_inventory;         // +0x0000C
    uint32_t result;                             // +0x040bc
    uint32_t money;                              // +0x040c0
    uint32_t coin;                               // +0x040c4
    uint32_t pay_coin;                           // +0x040c8
    uint32_t event_coin;                         // +0x040cc
    Inven_Item avatar_equipment[oldsrv::config::kAvatarInventoryItemCount];
                                                   // +0x040d0
    Inven_Item inventory[oldsrv::config::kMainInventoryItemCount];
                                                   // +0x059d5
    Inven_Item avatar_inventory[oldsrv::config::kAvatarEquipmentTypeCount];
                                                   // +0x0adb5
    Inven_Item equipment[oldsrv::config::kOrdinaryEquipmentTypeCount];
                                                   // +0x0b054
    int32_t cargo_capacity;                      // +0x0b330
    Inven_Item cargo[oldsrv::config::kCargoInventoryItemCount];
                                                   // +0x0b334
    int32_t avatar_count;                        // +0x0d76c
    AvatarDetailInfo avatar_detail[
        oldsrv::config::kAvatarDetailItemCount]; // +0x0d770
    Inven_Item creature_inventory[
        oldsrv::config::kCreatureInventoryItemCount]; // +0x0f96c
    int32_t creature_count;                      // +0x13318
    SIG_CREATURE_ITEM_DETAIL_INFO creature_detail[141];
                                                   // +0x1331c
    uint32_t clear_avatar[oldsrv::config::kPersistedClearAvatarCount];
                                                   // +0x151f4
    uint32_t avatar_coin;                        // +0x15220
    stItemLockInfo item_locks[100];              // +0x15224
    SIG_SAVE_EXPAND_EQUIPSLOT expanded_equipment;// +0x15544
    uint32_t inventory_capacity;                 // +0x15ddc
    int32_t creature_period_count;               // +0x15de0
    SIG_POSTAL_CREATURE_TEMP_PERIOD creature_period[20];
                                                   // +0x15de4
    SIG_SAVE_PC_ROOM_RENT_ITEM pc_room_rent;     // +0x15e84
    SIG_SAVE_DIMENSION_RENT_ITEM dimension_rent; // +0x15e90
    int32_t max_equip_level;                     // +0x15ee8
};

static_assert(sizeof(stItemLockInfo) == 8u,
              "stItemLockInfo IDA ABI size must stay 8");
static_assert(sizeof(SIG_POSTAL_CREATURE_TEMP_PERIOD) == 8u,
              "creature temp-period IDA ABI size must stay 8");
static_assert(sizeof(SIG_SAVE_DIMENSION_RENT_INFO) == 8u,
              "dimension rent row IDA ABI size must stay 8");
static_assert(sizeof(SIG_SAVE_DIMENSION_RENT_ITEM) == 88u,
              "dimension rent list IDA ABI size must stay 88");
static_assert(sizeof(InventoryItemDbFields) == 0x3Du,
              "typed inventory item view must stay 0x3D");
static_assert(sizeof(LoadInventoryCreatureDetail) == 56u,
              "creature detail IDA ABI size must stay 56");
static_assert(sizeof(SIG_LOAD_INVENTORY) == 89836u,
              "SIG_LOAD_INVENTORY target ABI size must stay 89836");
static_assert(offsetof(SIG_LOAD_INVENTORY, tag_inventory) == 12u,
              "tag inventory offset must stay 12");
static_assert(offsetof(SIG_LOAD_INVENTORY, result) == 16572u,
              "load result offset must stay 16572");
static_assert(offsetof(SIG_LOAD_INVENTORY, avatar_equipment) == 16592u,
              "avatar equipment offset must stay 16592");
static_assert(offsetof(SIG_LOAD_INVENTORY, inventory) == 22997u,
              "inventory offset must stay 22997");
static_assert(offsetof(SIG_LOAD_INVENTORY, avatar_inventory) == 44469u,
              "avatar inventory offset must stay 44469");
static_assert(offsetof(SIG_LOAD_INVENTORY, equipment) == 45140u,
              "equipment offset must stay 45140");
static_assert(offsetof(SIG_LOAD_INVENTORY, cargo_capacity) == 45872u,
              "cargo capacity offset must stay 45872");
static_assert(offsetof(SIG_LOAD_INVENTORY, cargo) == 45876u,
              "cargo offset must stay 45876");
static_assert(offsetof(SIG_LOAD_INVENTORY, avatar_count) == 55148u,
              "avatar count offset must stay 55148");
static_assert(offsetof(SIG_LOAD_INVENTORY, avatar_detail) == 55152u,
              "avatar detail offset must stay 55152");
static_assert(offsetof(SIG_LOAD_INVENTORY, creature_inventory) == 63852u,
              "creature inventory offset must stay 63852");
static_assert(offsetof(SIG_LOAD_INVENTORY, creature_count) == 78616u,
              "creature count offset must stay 78616");
static_assert(offsetof(SIG_LOAD_INVENTORY, creature_detail) == 78620u,
              "creature detail offset must stay 78620");
static_assert(offsetof(SIG_LOAD_INVENTORY, clear_avatar) == 86516u,
              "clear-avatar offset must stay 86516");
static_assert(offsetof(SIG_LOAD_INVENTORY, avatar_coin) == 86560u,
              "avatar coin offset must stay 86560");
static_assert(offsetof(SIG_LOAD_INVENTORY, item_locks) == 86564u,
              "item-lock offset must stay 86564");
static_assert(offsetof(SIG_LOAD_INVENTORY, expanded_equipment) == 87364u,
              "expanded equipment offset must stay 87364");
static_assert(offsetof(SIG_LOAD_INVENTORY, inventory_capacity) == 89564u,
              "inventory capacity offset must stay 89564");
static_assert(offsetof(SIG_LOAD_INVENTORY, creature_period) == 89572u,
              "creature period offset must stay 89572");
static_assert(offsetof(SIG_LOAD_INVENTORY, pc_room_rent) == 89732u,
              "PC-room rent offset must stay 89732");
static_assert(offsetof(SIG_LOAD_INVENTORY, dimension_rent) == 89744u,
              "dimension rent offset must stay 89744");
static_assert(offsetof(SIG_LOAD_INVENTORY, max_equip_level) == 89832u,
              "max equipment level offset must stay 89832");

#endif  // OLDSRV_DB_LOAD_INVENTORY_PAYLOAD_H_
