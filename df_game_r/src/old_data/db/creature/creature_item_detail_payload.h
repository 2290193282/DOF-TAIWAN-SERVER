#ifndef OLDSRV_OLD_DATA_DB_CREATURE_CREATURE_ITEM_DETAIL_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CREATURE_CREATURE_ITEM_DETAIL_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>


// IDA_EA: CCreatureMgr::GetCreatureItem @0x833B80C,
// CCreatureMgr::InsertCreatureItemResult @0x833B38C and
// DB_LoadInventory::GetUserCreature @0x840D48F.
//
// This is the one physical 0x38-byte creature detail record shared by the
// inventory-load result and by SIG_CREATURE_ITEM at +0x2c.  The expiration
// word carries a usable-period day count on a DB request and the absolute
// expiration tick on a DB/load result; the storage itself is not overlaid.
#pragma pack(push, 1)
struct SIG_CREATURE_ITEM_DETAIL_INFO {
    int32_t slot;                    // +0x00
    int32_t ui_id;                   // +0x04
    char name[16];                   // +0x08
    int32_t stomach;                 // +0x18
    uint32_t charac_no;              // +0x1C
    int32_t experience;              // +0x20
    uint8_t creature_type;           // +0x24
    uint8_t creature_flag;           // +0x25
    uint8_t no_charge;               // +0x26
    uint8_t state;                   // +0x27
    int32_t expiration_value;        // +0x28
    int32_t delete_tick;             // +0x2C
    uint8_t grow_creature;           // +0x30
    uint8_t max_level;               // +0x31
    uint16_t growth_alignment;       // +0x32
    uint32_t cera_order_no;          // +0x34
};
#pragma pack(pop)

static_assert(sizeof(SIG_CREATURE_ITEM_DETAIL_INFO) == 0x38u,
              "creature detail record must stay 0x38 bytes");
static_assert(offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, slot) == 0x00u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, ui_id) == 0x04u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, name) == 0x08u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, stomach) == 0x18u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, charac_no) == 0x1cu &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, experience) == 0x20u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, creature_type) == 0x24u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, creature_flag) == 0x25u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, no_charge) == 0x26u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, state) == 0x27u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, expiration_value) == 0x28u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, delete_tick) == 0x2cu &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, grow_creature) == 0x30u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, max_level) == 0x31u &&
                  offsetof(SIG_CREATURE_ITEM_DETAIL_INFO, cera_order_no) == 0x34u,
              "creature detail named fields must retain the 0627 DB ABI");

typedef SIG_CREATURE_ITEM_DETAIL_INFO LoadInventoryCreatureDetail;

#endif  // OLDSRV_OLD_DATA_DB_CREATURE_CREATURE_ITEM_DETAIL_PAYLOAD_H_
