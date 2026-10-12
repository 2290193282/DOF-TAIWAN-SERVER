#ifndef OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_EQUIPMENT_VALUE_TYPES_H_
#define OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_EQUIPMENT_VALUE_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>

#include "old_data/PVF/event/action_send_mail.h"
#include "old_game/economy/inventory/avatar_type_select_runtime.h"
#include "old_game/activity/schedule/in_game_event_script.h"


// IDA unit: 06_userinfo_stat_inventory/stAuraAvatarOption.
struct stAuraAvatarOption {
    void* reset();

    // IDA 0x840838E accepts option_type 0..2 and writes one signed dword per
    // type; 0x868DDC2 copies the same three consecutive dwords.
    int32_t values[3];
};

// IDA unit: 06_userinfo_stat_inventory/stAvatarEmblemInfo_t.  The original
// init @ 0x81506E8 writes a 16-bit socket type and 32-bit emblem item ID at
// each of five consecutive six-byte records.
#pragma pack(push, 1)
struct stAvatarEmblemSocket_t {
    uint16_t socket_type;       // +0x00
    uint32_t emblem_item_index; // +0x02
};
#pragma pack(pop)

struct stAvatarEmblemInfo_t {
    bool init();

    stAvatarEmblemSocket_t sockets[5]; // +0x00
};

static_assert(sizeof(stAvatarEmblemSocket_t) == 0x06u,
              "avatar emblem socket record must stay six bytes");
static_assert(offsetof(stAvatarEmblemSocket_t, emblem_item_index) == 0x02u,
              "avatar emblem item offset must stay +0x02");
static_assert(sizeof(stAvatarEmblemInfo_t) == 0x1eu,
              "avatar emblem data must retain the IDA 30-byte ABI");

// IDA unit: 06_userinfo_stat_inventory/stAvatarExpansionInfo_t.
struct stAvatarExpansionInfo_t {
    bool init();

    uint16_t avatar_expansion[2];
};

// IDA unit: 06_userinfo_stat_inventory/stEquipCondition.
struct stEquipCondition {
    stEquipCondition();

    uint8_t has_equipment_material;  // +0x00
    uint8_t equipment_upgrade;       // +0x01
    uint8_t has_amplify_option;      // +0x02
    uint8_t amplify_ability_type;    // +0x03
    uint16_t amplify_ability_value;  // +0x04
    uint8_t sealed;                  // +0x06
    uint8_t reseal_count;            // +0x07
};

static_assert(sizeof(stEquipCondition) == 0x08u,
              "recipe equipment condition must retain its 0x08-byte ABI");

// IDA_EA: CInventory::CheckEquipmentItemCondition @0x85083D0.
// CMD274 copies this exact 12-byte record from the target equipment before it
// deletes and remakes the output item.  It is distinct from the 8-byte recipe
// stEquipCondition above: +0x08 is the preserved Inven_Item::extend_info_.
struct stEquipConditionForChronicle {
    uint8_t upgrade;       // +0x00
    uint8_t has_ability;   // +0x01
    uint8_t ability_type;  // +0x02
    uint8_t padding_03;    // +0x03
    uint16_t ability_value; // +0x04
    uint8_t seal_or_state; // +0x06
    uint8_t reseal_count;  // +0x07
    int32_t extend_info;   // +0x08
};

static_assert(sizeof(stEquipConditionForChronicle) == 0x0cu,
              "CMD274 equipment condition must retain its 0x0c-byte ABI");

// IDA unit: 06_userinfo_stat_inventory/AvatarDisjointRandomRewardScript.
struct AvatarDisjointRandomRewardScript : public InGameEventScript {
    // Old IDA address: 0x8985BE2.
    AvatarDisjointRandomRewardScript();

    std::map<char, unsigned int> random_reward_items;
    Action_SendMail send_mail;
};


// Old IDA address: 0x89824B4.
int importAvatarDisjointRandomRewardScript(
    AvatarDisjointRandomRewardScript* script,
    const char* filename)
    __asm__("_Z38importAvatarDisjointRandomRewardScriptP32AvatarDisjointRandomRewardScriptPKc");

#endif  // OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_EQUIPMENT_VALUE_TYPES_H_
