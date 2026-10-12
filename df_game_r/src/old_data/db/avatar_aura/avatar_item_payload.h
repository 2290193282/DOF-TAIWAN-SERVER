#ifndef DF_GAME_R_DB_AVATAR_ITEM_PAYLOAD_H_
#define DF_GAME_R_DB_AVATAR_ITEM_PAYLOAD_H_

#include "old_game/economy/equipment/avatar_equipment_value_types.h"

#include <stddef.h>
#include <stdint.h>


// IDA unit: 06_userinfo_stat_inventory/SIG_AVATAR_INSERT.
// IDA_EA: set @0x850D3C8, setJewelSocket @0x850D44C.
#pragma pack(push, 1)
struct SIG_AVATAR_INSERT {
    int32_t charac_no;                       // +0x00
    uint32_t item_id;                        // +0x04
    uint8_t slot;                            // +0x08
    char ability_no;                         // +0x09
    int32_t expire_time;                     // +0x0A
    bool obtain_from;                        // +0x0E
    char ipg_agency_no[24];                  // +0x0F
    uint8_t unknown_39;                      // +0x27
    uint32_t add_info;                       // +0x28
    int8_t stat;                             // +0x2C
    stAvatarEmblemInfo_t jewel_socket;       // +0x2D
    uint32_t ui_id;                          // +0x4B
    stAvatarExpansionInfo_t expansion_info;  // +0x4F

    SIG_AVATAR_INSERT* set(int charac_no_value,
                           unsigned int item_id_value,
                           int slot_value,
                           int expire_time_value,
                           bool obtain_from_value,
                           char ability_no_value,
                           const char* ipg_agency_no_value,
                           int stat_value,
                           unsigned long add_info_value);
    SIG_AVATAR_INSERT* setJewelSocket(
        const stAvatarEmblemInfo_t& source);
};

// IDA unit: 06_userinfo_stat_inventory/DB_AvatarChangeOwner.
// IDA_EA: makeRequest @0x840055C, dispatch @0x84004BC.
struct SIG_AVATAR_CHANGE_OWNER {
    uint32_t ui_id;         // +0x00
    uint32_t old_charac_no; // +0x04
    uint32_t new_charac_no; // +0x08
    uint8_t slot;           // +0x0C
};

// IDA unit: 06_userinfo_stat_inventory/DB_AvatarExtendPeriod.
struct SIG_AVATAR_RENEW {
    uint32_t charac_no; // +0x00
    uint8_t slot;       // +0x04
    uint16_t days;      // +0x05
};

// IDA unit: 06_userinfo_stat_inventory/DB_AvatarColorChange.
struct SIG_AVATAR_COLOR_CHANGE {
    uint32_t charac_no; // +0x00
    uint32_t ui_id;     // +0x04
    uint16_t color_1;   // +0x08
    uint16_t color_2;   // +0x0A
};

// IDA unit: 06_userinfo_stat_inventory/DB_AvatarOptionChange.
struct SIG_AVATAR_OPTION_CHANGE {
    uint32_t ui_id;      // +0x00
    uint8_t ability_no;  // +0x04
    uint8_t unknown_05;  // +0x05
    uint8_t unknown_06;  // +0x06
    uint8_t unknown_07;  // +0x07
    uint32_t charac_no;  // +0x08
};

// IDA unit: 04_character_select_create/DB_UpdateAvatarJewelSlot.
struct SIG_UPDATE_AVATAR_JEWEL_SOCKET {
    uint32_t charac_no;                    // +0x00
    uint32_t avatar_ui_id;                 // +0x04
    stAvatarEmblemInfo_t jewel_socket;     // +0x08
    uint8_t abi_tail_26[2];                // +0x26
};

// IDA unit: 06_userinfo_stat_inventory/DB_AvatarItemMove.
struct SIG_AVATAR_MOVE {
    uint32_t unknown_00;             // +0x00
    int32_t first_ui_id;             // +0x04
    int32_t second_ui_id;            // +0x08
    uint8_t second_slot;             // +0x0C
    uint8_t first_slot;              // +0x0D
    int32_t first_clear_avatar_id;   // +0x0E
    int32_t second_clear_avatar_id;  // +0x12
};

// IDA unit: 06_userinfo_stat_inventory/DB_AvatarChangeStat. The signal is a
// request on input and an avatar-row result when the requested stat is zero.
struct SIG_AVATAR_CHANGE_STAT {
    struct LoadedAvatar {
        int8_t ability_no;                  // +0x09
        uint32_t reg_date;                  // +0x0A
        uint32_t expire_date;               // +0x0E
        char ipg_agency_no[24];             // +0x12
        uint8_t unknown_42;                 // +0x2A
        uint16_t hidden_option;             // +0x2B
        uint16_t unknown_45;                // +0x2D
        stAvatarEmblemInfo_t jewel_socket;  // +0x2F
        // DB_AvatarChangeStat @ 0x840DCFA writes the two 16-bit columns
        // at signal offsets +0x4D/+0x4F.  Inter_AvatarChangeStat @
        // 0x84CF6EC passes the same four bytes to RegistItem as the typed
        // stAvatarExpansionInfo_t object.  Both views are original ABI
        // aliases, not a padding region.
        union {
            stAvatarExpansionInfo_t expansion_info;  // signal +0x4D
            struct {
                uint16_t hidden_option_copy;         // signal +0x4D
                int16_t emblem_endurance;            // signal +0x4F
            } db_row;
        } expansion_or_db_row;
    };

    uint32_t ui_id;                         // +0x00
    union {
        int32_t stat;                       // request +0x04
        uint32_t item_id;                   // result +0x04
    } stat_or_item_id;
    uint8_t slot;                           // +0x08
    union {
        uint32_t charac_no;                 // request +0x09
        LoadedAvatar loaded;                // result +0x09
    } charac_or_loaded;
};
#pragma pack(pop)

static_assert(sizeof(bool) == 1u, "old bool ABI must stay one byte");
static_assert(sizeof(unsigned long) == 4u,
              "old SIG_AVATAR_INSERT add_info ABI must stay 32-bit");
static_assert(offsetof(SIG_AVATAR_INSERT, expire_time) == 0x0Au,
              "SIG_AVATAR_INSERT expire_time offset must stay 0x0A");
static_assert(offsetof(SIG_AVATAR_INSERT, add_info) == 0x28u,
              "SIG_AVATAR_INSERT add_info offset must stay 0x28");
static_assert(offsetof(SIG_AVATAR_INSERT, jewel_socket) == 0x2Du,
              "SIG_AVATAR_INSERT jewel_socket offset must stay 0x2D");
static_assert(offsetof(SIG_AVATAR_INSERT, ui_id) == 0x4Bu,
              "SIG_AVATAR_INSERT ui_id offset must stay 0x4B");
static_assert(offsetof(SIG_AVATAR_INSERT, expansion_info) == 0x4Fu,
              "SIG_AVATAR_INSERT expansion offset must stay 0x4F");
static_assert(sizeof(SIG_AVATAR_INSERT) == 0x53u,
              "SIG_AVATAR_INSERT IDA ABI size must stay 0x53");
static_assert(sizeof(SIG_AVATAR_CHANGE_OWNER) == 0x0Du,
              "SIG_AVATAR_CHANGE_OWNER IDA ABI size must stay 0x0D");
static_assert(sizeof(SIG_AVATAR_RENEW) == 0x07u,
              "SIG_AVATAR_RENEW IDA ABI size must stay 0x07");
static_assert(sizeof(SIG_AVATAR_COLOR_CHANGE) == 0x0Cu,
              "SIG_AVATAR_COLOR_CHANGE IDA ABI size must stay 0x0C");
static_assert(sizeof(SIG_AVATAR_OPTION_CHANGE) == 0x0Cu,
              "SIG_AVATAR_OPTION_CHANGE IDA ABI size must stay 0x0C");
static_assert(offsetof(SIG_UPDATE_AVATAR_JEWEL_SOCKET, jewel_socket) == 0x08u,
              "SIG_UPDATE_AVATAR_JEWEL_SOCKET jewel offset must stay 0x08");
static_assert(sizeof(SIG_UPDATE_AVATAR_JEWEL_SOCKET) == 0x28u,
              "SIG_UPDATE_AVATAR_JEWEL_SOCKET IDA ABI size must stay 0x28");
static_assert(sizeof(SIG_AVATAR_MOVE) == 0x16u,
              "SIG_AVATAR_MOVE IDA ABI size must stay 0x16");
static_assert(sizeof(SIG_AVATAR_CHANGE_STAT::LoadedAvatar) == 0x48u,
              "SIG_AVATAR_CHANGE_STAT loaded member ABI must stay 0x48");
static_assert(offsetof(SIG_AVATAR_CHANGE_STAT::LoadedAvatar,
                       expansion_or_db_row) == 0x44u,
              "SIG_AVATAR_CHANGE_STAT DB/expansion union must stay +0x44");
static_assert(offsetof(SIG_AVATAR_CHANGE_STAT, charac_or_loaded) == 0x09u,
              "SIG_AVATAR_CHANGE_STAT charac/result offset must stay 0x09");
static_assert(sizeof(SIG_AVATAR_CHANGE_STAT) == 0x51u,
              "SIG_AVATAR_CHANGE_STAT IDA ABI size must stay 0x51");

#endif  // DF_GAME_R_DB_AVATAR_ITEM_PAYLOAD_H_
