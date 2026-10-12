#ifndef OLDSRV_DB_DB_SAVE_INVEN_H_
#define OLDSRV_DB_DB_SAVE_INVEN_H_

#include "old_data/db/core/db_dispatcher.h"
#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include <stddef.h>
#include <stdint.h>

// IDA unit: old_game/economy/inventory/SIG_SAVE_INVENTORY.
// The 0xb75e-byte DB request is one packed source record.  The four equipment
// records copied after the creature table intentionally overlay its final
// four slots in the old serializer; the union names both original views.
#pragma pack(push, 1)
struct SIG_SAVE_INVENTORY_CREATURE_TAIL_OVERLAY {
    Inven_Item creature_prefix[
        oldsrv::config::kCreatureInventoryEquipmentFirst];
    Inven_Item equipment_tail[
        oldsrv::config::kCreatureArtifactEquipmentTypeCount];
};

union SIG_SAVE_INVENTORY_CREATURE_REGION {
    Inven_Item creature_items[
        oldsrv::config::kCreatureInventoryItemCount];
    SIG_SAVE_INVENTORY_CREATURE_TAIL_OVERLAY creature_and_equipment_tail;
};

struct SIG_SAVE_INVENTORY {
    uint32_t account_id;                         // +0x0000
    uint32_t character_no;                       // +0x0004
    uint8_t save_mode;                           // +0x0008
    uint32_t money;                              // +0x0009
    uint32_t coin;                               // +0x000d
    uint32_t pay_coin;                           // +0x0011
    uint32_t event_coin;                         // +0x0015
    Inven_Item equipment_items[
        oldsrv::config::kEquippedAvatarAndOrdinaryItemCount]; // +0x0019
    Inven_Item stackable_items[oldsrv::config::kMainInventoryItemCount];
                                                         // +0x0594
    SIG_SAVE_INVENTORY_CREATURE_REGION creature_region; // +0x5974
    uint32_t cargo_capacity;                     // +0x931e
    Inven_Item cargo_items[oldsrv::config::kCargoInventoryItemCount];
                                                   // +0x9322
    uint32_t avatar_coin;                        // +0xb75a
};
#pragma pack(pop)

static_assert(offsetof(SIG_SAVE_INVENTORY, equipment_items) == 25u,
              "SIG_SAVE_INVENTORY equipment table offset drift");
static_assert(offsetof(SIG_SAVE_INVENTORY, stackable_items) == 1428u,
              "SIG_SAVE_INVENTORY stackable table offset drift");
static_assert(offsetof(SIG_SAVE_INVENTORY, creature_region) == 22900u,
              "SIG_SAVE_INVENTORY creature table offset drift");
static_assert(offsetof(SIG_SAVE_INVENTORY, creature_region) +
                  offsetof(SIG_SAVE_INVENTORY_CREATURE_TAIL_OVERLAY,
                           equipment_tail) == 37418u,
              "SIG_SAVE_INVENTORY equipment-tail overlay offset drift");
static_assert(offsetof(SIG_SAVE_INVENTORY, cargo_capacity) == 37662u,
              "SIG_SAVE_INVENTORY cargo capacity offset drift");
static_assert(offsetof(SIG_SAVE_INVENTORY, cargo_items) == 37666u,
              "SIG_SAVE_INVENTORY cargo table offset drift");
static_assert(offsetof(SIG_SAVE_INVENTORY, avatar_coin) == 46938u,
              "SIG_SAVE_INVENTORY avatar coin offset drift");
static_assert(sizeof(SIG_SAVE_INVENTORY) == 46942u,
              "SIG_SAVE_INVENTORY IDA ABI size must stay 0xb75e");


// IDA_EA: 0x8416A42/0x8416A5E/0x8416A8E/0x8416AAC/0x8416B38/0x8416CFA/0x8417204.
// IDA unit: 13_database_dispatch/DB_SaveInven.
class DB_SaveInven : public DBDispatcher {
public:
    DB_SaveInven();
    virtual ~DB_SaveInven();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool _saveCharacInvenExpand(SIG_SAVE_INVENTORY* payload) const;
    bool SaveInven(SIG_SAVE_INVENTORY* payload);
    bool SendResult(int signal, int user_idx, SIG_SAVE_INVENTORY* payload);
};

#endif  // OLDSRV_DB_DB_SAVE_INVEN_H_
