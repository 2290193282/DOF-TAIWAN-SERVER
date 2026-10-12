#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_CREATE_CHARAC_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_CREATE_CHARAC_PAYLOAD_H_

#include "common/config.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/economy/inventory/inventory.h"

#include <stddef.h>
#include <stdint.h>


#pragma pack(push, 1)

// The 0x52-byte initial status record copied by
// DisPatcher_CreateCharac::process @0x81C0638.  Its SQL consumers are
// ARAD::DATABASE::CREATE_QUERY::arad_sp_create_character @0x818AF9A and
// DB_CreateCharac::MakeInsertCharacInfo @0x80EC2A0.
struct CREATE_CHARACTER_INITIAL_STATS {
    uint32_t max_hp;                 // +0x00
    uint32_t max_mp;                 // +0x04
    uint16_t physical_attack;        // +0x08
    uint16_t physical_defense;       // +0x0A
    uint16_t magical_attack;         // +0x0C
    uint16_t magical_defense;        // +0x0E
    uint16_t element_resist[4];      // +0x10, eight-byte DB blob
    uint16_t spec_property[17];      // +0x18, 34-byte DB blob
    int32_t inventory_weight;        // +0x3A
    int16_t hp_regeneration;         // +0x3E
    int16_t mp_regeneration;         // +0x40
    uint32_t move_speed;             // +0x42
    uint16_t attack_speed;           // +0x46
    uint16_t cast_speed;             // +0x48
    int16_t hit_recovery;            // +0x4A
    int16_t jump;                    // +0x4C
    int32_t character_weight;        // +0x4E
};

// 0828 payload used by the complete create-character DB chain.  The primary
// inventory region owns slots 0..288 and therefore carries the canonical 289
// Inven_Item records.  The remaining item regions retain their established
// 63/12/10-record layouts.
struct SIG_CREATE_CHARAC {
    uint32_t account_id;                         // +0x0000
    char character_name[30];                    // +0x0004
    int8_t job;                                 // +0x0022
    // Semantic value is the event grow-type code; event_charac_level is
    // only the historical database column name.
    int8_t event_character_grow_type;           // +0x0023
    uint8_t user_event_character_flag;          // +0x0024
    int8_t expert_job;                          // +0x0025
    CREATE_CHARACTER_INITIAL_STATS initial_stats; // +0x0026
    uint8_t initial_grow_flag;                  // +0x0078
    _Mastered_skill skill_slot[204];            // +0x0079
    _Mastered_skill skill_slot_second[204];     // +0x0211
    Inven_Item inventory[
        oldsrv::config::kMainInventoryPrimaryItemCount]; // +0x03A9
    Inven_Item cargo[63];                       // +0x4886
    Inven_Item equipment[12];                   // +0x5789
    Inven_Item reserved_item_slots[10];         // +0x5A65
    uint8_t has_inventory;                      // +0x5CC7
    uint8_t has_equipment;                      // +0x5CC8
    uint8_t reserved_5cc9;                      // +0x5CC9
    uint8_t reserved_5cca;                      // +0x5CCA
    uint8_t reserved_5ccb;                      // +0x5CCB
    uint32_t result;                            // +0x5CCC
    uint32_t character_no;                      // +0x5CD0
    uint8_t sex;                                // +0x5CD4
    uint8_t first_character_flag;               // +0x5CD5
    uint8_t initial_village;                    // +0x5CD6
    uint8_t reserved_5cd7;                      // +0x5CD7
    uint32_t remain_sp;                         // +0x5CD8
};

#pragma pack(pop)

static_assert(sizeof(CREATE_CHARACTER_INITIAL_STATS) == 0x52u,
              "create-character initial status size must stay 0x52");
static_assert(sizeof(SIG_CREATE_CHARAC) == 0x5cdcu,
              "SIG_CREATE_CHARAC 0828 ABI size must stay 0x5CDC");
static_assert(offsetof(SIG_CREATE_CHARAC, initial_stats) == 0x26u,
              "SIG_CREATE_CHARAC initial status offset must stay 0x26");
static_assert(offsetof(SIG_CREATE_CHARAC, initial_grow_flag) == 0x78u,
              "SIG_CREATE_CHARAC initial grow flag offset must stay 0x78");
static_assert(offsetof(SIG_CREATE_CHARAC, skill_slot) == 0x79u,
              "SIG_CREATE_CHARAC first skill offset must stay 0x79");
static_assert(offsetof(SIG_CREATE_CHARAC, skill_slot_second) == 0x211u,
              "SIG_CREATE_CHARAC second skill offset must stay 0x211");
static_assert(offsetof(SIG_CREATE_CHARAC, inventory) == 0x3A9u,
              "SIG_CREATE_CHARAC inventory offset must stay 0x3A9");
static_assert(
    sizeof(((SIG_CREATE_CHARAC*)0)->inventory) ==
        oldsrv::config::kMainInventoryPrimaryItemCount *
            oldsrv::config::kInventoryItemRecordSize,
    "SIG_CREATE_CHARAC inventory must carry all 289 primary records");
static_assert(offsetof(SIG_CREATE_CHARAC, cargo) == 0x4886u,
              "SIG_CREATE_CHARAC cargo offset must stay 0x4886");
static_assert(offsetof(SIG_CREATE_CHARAC, equipment) == 0x5789u,
              "SIG_CREATE_CHARAC equipment offset must stay 0x5789");
static_assert(offsetof(SIG_CREATE_CHARAC, reserved_item_slots) == 0x5a65u,
              "SIG_CREATE_CHARAC trailing item offset must stay 0x5A65");
static_assert(offsetof(SIG_CREATE_CHARAC, has_inventory) == 0x5cc7u,
              "SIG_CREATE_CHARAC inventory flag offset must stay 0x5CC7");
static_assert(offsetof(SIG_CREATE_CHARAC, has_equipment) == 0x5cc8u,
              "SIG_CREATE_CHARAC equipment flag offset must stay 0x5CC8");
static_assert(offsetof(SIG_CREATE_CHARAC, reserved_5cc9) == 0x5cc9u &&
                  offsetof(SIG_CREATE_CHARAC, reserved_5cca) == 0x5ccau &&
                  offsetof(SIG_CREATE_CHARAC, reserved_5ccb) == 0x5ccbu,
              "SIG_CREATE_CHARAC pre-result reserved bytes must stay packed");
static_assert(offsetof(SIG_CREATE_CHARAC, result) == 0x5cccu,
              "SIG_CREATE_CHARAC result offset must stay 0x5CCC");
static_assert(offsetof(SIG_CREATE_CHARAC, character_no) == 0x5cd0u,
              "SIG_CREATE_CHARAC character number offset must stay 0x5CD0");
static_assert(offsetof(SIG_CREATE_CHARAC, sex) == 0x5cd4u,
              "SIG_CREATE_CHARAC sex offset must stay 0x5CD4");
static_assert(offsetof(SIG_CREATE_CHARAC, first_character_flag) == 0x5cd5u,
              "SIG_CREATE_CHARAC first-character flag must stay at +0x5CD5");
static_assert(offsetof(SIG_CREATE_CHARAC, initial_village) == 0x5cd6u,
              "SIG_CREATE_CHARAC initial village offset must stay 0x5CD6");
static_assert(offsetof(SIG_CREATE_CHARAC, reserved_5cd7) == 0x5cd7u,
              "SIG_CREATE_CHARAC pre-SP reserved byte must stay at +0x5CD7");
static_assert(offsetof(SIG_CREATE_CHARAC, remain_sp) == 0x5cd8u,
              "SIG_CREATE_CHARAC remain-SP offset must stay 0x5CD8");

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_CREATE_CHARAC_PAYLOAD_H_
