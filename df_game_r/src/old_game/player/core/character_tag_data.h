#ifndef OLDSRV_OLD_GAME_USERINFO_CHARACTER_TAG_DATA_H_
#define OLDSRV_OLD_GAME_USERINFO_CHARACTER_TAG_DATA_H_

#include <stddef.h>
#include <stdint.h>

#include "common/config.h"
#include "old_data/db/character/load_charac_payload.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/inventory.h"

#pragma pack(push, 1)

// IDA_EA: DB_LoadInventory::_GetUserAvatar @0x840BE8A advances this record
// by 75 bytes and writes the following eight fields at the asserted offsets.
struct AvatarDetailInfo {
    uint32_t ui_id;                       // +0x00
    uint32_t expire_date;                 // +0x04
    uint32_t registration_date;           // +0x08
    char ipg_agency_no[25];               // +0x0c
    uint32_t clear_avatar_id;             // +0x25
    stAvatarEmblemInfo_t jewel_socket;     // +0x29
    int16_t color1;                        // +0x47
    int16_t color2;                        // +0x49
};

// IDA unit: 04_character_select_create/TAG_CHARAC_INVEN_DATA.
struct TAG_CHARAC_INVEN_DATA {
    TAG_CHARAC_INVEN_DATA();

    void* clear();

    Inven_Item avatar_inventory[
        oldsrv::config::kEquippedAvatarAndOrdinaryItemCount]; // +0x0000
    Inven_Item avatar_equipment[oldsrv::config::kAvatarInventoryItemCount];
                                                               // +0x057b
    int32_t avatar_count;                                      // +0x1e80
    AvatarDetailInfo avatar_detail[oldsrv::config::kAvatarDetailItemCount];
                                                               // +0x1e84
    // Target slot 10 is the weapon-avatar slot and has its own clear-avatar
    // association, so the persisted TAG record carries slots 0..10.
    unsigned long clear_avatar[oldsrv::config::kPersistedClearAvatarCount];
                                                               // +0x4080 (i386)
    uint32_t cargo_capacity;                                   // +0x40ac
};

#pragma pack(pop)

static_assert(sizeof(AvatarDetailInfo) == 75u,
              "AvatarDetailInfo IDA ABI size must stay 75");
static_assert(offsetof(AvatarDetailInfo, clear_avatar_id) == 37u,
              "AvatarDetailInfo clear-avatar offset must stay 37");
static_assert(offsetof(AvatarDetailInfo, jewel_socket) == 41u,
              "AvatarDetailInfo jewel-socket offset must stay 41");
static_assert(offsetof(AvatarDetailInfo, color1) == 71u,
              "AvatarDetailInfo first color offset must stay 71");
static_assert(sizeof(TAG_CHARAC_INVEN_DATA) == 0x40b0u,
              "TAG_CHARAC_INVEN_DATA target ABI size must stay 0x40b0");
static_assert(offsetof(TAG_CHARAC_INVEN_DATA, avatar_equipment) == 0x057bu,
              "TAG inventory avatar equipment offset must stay 0x057b");
static_assert(offsetof(TAG_CHARAC_INVEN_DATA, avatar_count) == 0x1e80u,
              "TAG inventory avatar count offset must stay 0x1e80");
static_assert(offsetof(TAG_CHARAC_INVEN_DATA, avatar_detail) == 0x1e84u,
              "TAG inventory avatar detail offset must stay 0x1e84");
static_assert(offsetof(TAG_CHARAC_INVEN_DATA, clear_avatar) == 0x4080u,
              "TAG inventory clear-avatar offset must stay 0x4080");
static_assert(offsetof(TAG_CHARAC_INVEN_DATA, cargo_capacity) == 0x40acu,
              "TAG inventory cargo capacity offset must stay 0x40ac");

// IDA unit: 04_character_select_create/TAG_CHARAC_ETC_DATA.
struct TAG_CHARAC_ETC_DATA {
    TAG_CHARAC_ETC_DATA();

    void* clear();

    _Mastered_skill skill_trees[2][204];
};

static_assert(sizeof(TAG_CHARAC_ETC_DATA) == 0x330u,
              "TAG_CHARAC_ETC_DATA IDA ABI size must stay 0x330");

// IDA unit: 04_character_select_create/CHARAC_QUEST_SHOP_DATA.
struct __attribute__((packed)) CHARAC_QUEST_SHOP_DATA {
    CHARAC_QUEST_SHOP_DATA();

    void* clear();

    uint32_t charac_no;             // IDA +0x00
    uint32_t quest_point;           // IDA +0x04
    uint32_t init_count;            // IDA +0x08
    uint16_t buying_count[77];      // IDA +0x0c, status ids 0..76
    uint32_t quest_piece;           // IDA +0xa6
};

static_assert(sizeof(CHARAC_QUEST_SHOP_DATA) == 0xaau,
              "quest-shop character payload must stay 0xaa bytes");
static_assert(offsetof(CHARAC_QUEST_SHOP_DATA, buying_count) == 0x0cu &&
                  offsetof(CHARAC_QUEST_SHOP_DATA, quest_piece) == 0xa6u,
              "quest-shop character fields must preserve the 0627 ABI");


#endif  // OLDSRV_OLD_GAME_USERINFO_CHARACTER_TAG_DATA_H_
