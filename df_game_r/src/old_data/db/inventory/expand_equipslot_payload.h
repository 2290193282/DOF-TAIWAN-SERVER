#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_EXPAND_EQUIPSLOT_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_EXPAND_EQUIPSLOT_PAYLOAD_H_

#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>

#pragma pack(push, 1)

// IDA unit: 04_character_select_create/SIG_SAVE_EXPAND_EQUIPSLOT.
struct SIG_SAVE_EXPAND_EQUIPSLOT {
    SIG_SAVE_EXPAND_EQUIPSLOT();

    uint32_t charac_no;
    Inven_Item expand_equipslot_items[36];
};

struct SIG_SAVE_EXPAND_CHARAC_STAT {
    int8_t channel_equipslot_switch;
    int8_t expand_equipslot_switch;
    uint8_t reserved[2];
};

struct SIG_SAVE_EXPAND_CHARAC {
    SIG_SAVE_EXPAND_EQUIPSLOT equipslot;
    SIG_SAVE_EXPAND_CHARAC_STAT stat;
};

#pragma pack(pop)

static_assert(sizeof(SIG_SAVE_EXPAND_EQUIPSLOT) == 0x898u,
              "SIG_SAVE_EXPAND_EQUIPSLOT size must stay 0x898");
static_assert(sizeof(SIG_SAVE_EXPAND_CHARAC_STAT) == 0x04u,
              "SIG_SAVE_EXPAND_CHARAC_STAT size must stay 0x04");
static_assert(sizeof(SIG_SAVE_EXPAND_CHARAC) == 0x89Cu,
              "SIG_SAVE_EXPAND_CHARAC size must stay 0x89C");

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_EXPAND_EQUIPSLOT_PAYLOAD_H_
