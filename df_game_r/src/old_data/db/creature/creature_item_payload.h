#ifndef OLDSRV_OLD_DATA_DB_CREATURE_CREATURE_ITEM_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CREATURE_CREATURE_ITEM_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/creature/creature_item_detail_payload.h"
#include "old_game/economy/equipment/item_option.h"


// IDA_EA: Inven_Item_Compatible::setCopy @0x833F38E and
// Inven_Item_Compatible::makeInvenItem @0x833F418.  SIG_CREATURE_ITEM embeds
// the old 0x26-byte wire-compatible projection, not the 0x3D-byte runtime
// Inven_Item object.
#pragma pack(push, 1)
struct SIG_CREATURE_ITEM_COMPATIBLE_DATA {
    uint8_t item_flag;                         // +0x00
    uint8_t item_type;                         // +0x01
    uint16_t item_type_alignment;              // +0x02
    uint32_t item_id;                          // +0x04
    uint8_t item_attr;                         // +0x08
    uint8_t item_attr_alignment_09;            // +0x09
    uint8_t item_attr_alignment_0a;            // +0x0A
    uint8_t item_attr_alignment_0b;            // +0x0B
    int32_t add_info;                          // +0x0C
    uint16_t endurance;                        // +0x10
    uint16_t endurance_alignment;              // +0x12
    int32_t expire_date;                       // +0x14
    stAmplifyOption_t amplify_option;          // +0x18
    itemGloballyUniqueIdentifier_t identifier; // +0x1C
};

// IDA_EA: Stream::GetInBuffer<SIG_CREATURE_ITEM> @0x8345066,
// Stream::GetOutBuffer<SIG_CREATURE_ITEM> @0x8451A52,
// CStreamGuard::GetInBuffer<SIG_CREATURE_ITEM> @0x8343ADE,
// DB_InsertCreatureItem::insertCreatureItem @0x841FA28, and
// DB_UpdateCreatureItem::dispatch @0x84203E8.
struct SIG_CREATURE_ITEM {
    uint32_t update_mask;                              // +0x00
    SIG_CREATURE_ITEM_COMPATIBLE_DATA inven_item_copy; // +0x04
    uint16_t inven_item_alignment;                     // +0x2A
    SIG_CREATURE_ITEM_DETAIL_INFO detail;              // +0x2C
    int32_t update_kind;                               // +0x64
    uint32_t current_creature_flag;                    // +0x68
    uint32_t previous_charac_no;                       // +0x6C
    uint32_t previous_creature_flag;                   // +0x70
};
#pragma pack(pop)

static_assert(sizeof(SIG_CREATURE_ITEM_COMPATIBLE_DATA) == 0x26u,
              "SIG_CREATURE_ITEM compatible data size must stay 0x26");
static_assert(offsetof(SIG_CREATURE_ITEM_COMPATIBLE_DATA, item_id) == 0x04u,
              "SIG_CREATURE_ITEM compatible item_id offset must stay 0x04");
static_assert(offsetof(SIG_CREATURE_ITEM_COMPATIBLE_DATA, endurance) == 0x10u,
              "SIG_CREATURE_ITEM compatible endurance offset must stay 0x10");
static_assert(offsetof(SIG_CREATURE_ITEM_COMPATIBLE_DATA, amplify_option) ==
                  0x18u,
              "SIG_CREATURE_ITEM compatible amplify offset must stay 0x18");
static_assert(offsetof(SIG_CREATURE_ITEM_COMPATIBLE_DATA, identifier) == 0x1Cu,
              "SIG_CREATURE_ITEM compatible identifier offset must stay 0x1C");
static_assert(sizeof(SIG_CREATURE_ITEM) == 0x74u,
              "SIG_CREATURE_ITEM IDA ABI size must stay 0x74");
static_assert(offsetof(SIG_CREATURE_ITEM, inven_item_copy) == 0x04u,
              "SIG_CREATURE_ITEM compatible data offset must stay 0x04");
static_assert(offsetof(SIG_CREATURE_ITEM, detail) == 0x2Cu,
              "SIG_CREATURE_ITEM detail offset must stay 0x2C");
static_assert(offsetof(SIG_CREATURE_ITEM, update_kind) == 0x64u,
              "SIG_CREATURE_ITEM update kind offset must stay 0x64");
static_assert(offsetof(SIG_CREATURE_ITEM, current_creature_flag) == 0x68u,
              "SIG_CREATURE_ITEM current flag offset must stay 0x68");
static_assert(offsetof(SIG_CREATURE_ITEM, previous_charac_no) == 0x6Cu,
              "SIG_CREATURE_ITEM previous charac offset must stay 0x6C");
static_assert(offsetof(SIG_CREATURE_ITEM, previous_creature_flag) == 0x70u,
              "SIG_CREATURE_ITEM previous flag offset must stay 0x70");

#endif  // OLDSRV_OLD_DATA_DB_CREATURE_CREATURE_ITEM_PAYLOAD_H_
