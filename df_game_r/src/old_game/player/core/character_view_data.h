#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_VIEW_DATA_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_VIEW_DATA_H_

#include <stddef.h>
#include <stdint.h>

#include "common/config.h"

// IDA unit: 04_character_select_create/CCharacterView.
struct CCharacterView {
    CCharacterView();
    ~CCharacterView();

    CCharacterView* enableSaveCharacView();
    int isSaveCharacView();
    int reset();
    CCharacterView* disableSaveCharacView();

    uint8_t save_charac_view;
};

// IDA unit: 04_character_select_create/STGuildMemberCharacData.
struct STGuildMemberCharacData {
    STGuildMemberCharacData();

    uint8_t raw_00[0x21];
};

// IDA unit: 04_character_select_create/STPowerWarCharacData.
struct STPowerWarCharacData {
    STPowerWarCharacData();

    uint16_t word_00;
    uint8_t byte_02;
    uint8_t byte_03;
    int dword_04;
    int dword_08;
};

#pragma pack(push, 1)

struct CHARAC_VIEW_AVATAR_DATA {
    uint32_t item_id;                 // +0x00
    uint16_t avatar_expansion_0;      // +0x04
    uint16_t avatar_expansion_1;      // +0x06
};

// Character-view persistence stores 36 consecutive native records after
// SIG_UPDATE_CHARAC_VIEW::account_id.
struct CHARAC_LOGIN_DATA {
    uint32_t character_no;                         // +0x00
    char character_name[20];                       // +0x04
    uint8_t job;                                   // +0x18
    uint8_t grow_type;                             // +0x19
    uint16_t level;                                // +0x1a
    uint8_t state_flags;                           // +0x1c
    uint8_t striker_skill_index;                   // +0x1d
    uint32_t tag_character_no;                     // +0x1e
    uint8_t request_rename_state;                  // +0x22
    uint8_t state_23_27[5];                        // +0x23
    CHARAC_VIEW_AVATAR_DATA visible_avatar[
        oldsrv::config::kCharacterViewEquipmentTypeCount]; // +0x28
    uint32_t create_time;                          // +0x90
    uint32_t chaos_die_time;                       // +0x94
    uint32_t chaos_response_time_late;             // +0x98
};

struct SIG_UPDATE_CHARAC_VIEW {
    int32_t account_id;                            // +0x0000
    CHARAC_LOGIN_DATA characters[36];              // +0x0004
    uint8_t server_group;                          // +0x15f4
    uint8_t tail_padding[3];                       // +0x15f5
};

#pragma pack(pop)

static_assert(sizeof(CHARAC_VIEW_AVATAR_DATA) == 0x08u,
              "character-view avatar record must stay eight bytes");
static_assert(sizeof(CHARAC_LOGIN_DATA) == 0x9Cu,
              "CHARAC_LOGIN_DATA native ABI must stay 0x9C");
static_assert(offsetof(CHARAC_LOGIN_DATA, visible_avatar) == 0x28u &&
                  offsetof(CHARAC_LOGIN_DATA, create_time) == 0x90u &&
                  offsetof(CHARAC_LOGIN_DATA, chaos_response_time_late) ==
                      0x98u,
              "CHARAC_LOGIN_DATA named field offsets drift");
static_assert(sizeof(SIG_UPDATE_CHARAC_VIEW) == 0x15f8u,
              "SIG_UPDATE_CHARAC_VIEW native ABI must stay 0x15F8");
static_assert(offsetof(SIG_UPDATE_CHARAC_VIEW, account_id) == 0x0000u &&
                  offsetof(SIG_UPDATE_CHARAC_VIEW, characters) == 0x0004u &&
                  offsetof(SIG_UPDATE_CHARAC_VIEW, server_group) == 0x15f4u &&
                  offsetof(SIG_UPDATE_CHARAC_VIEW, tail_padding) == 0x15f5u,
              "SIG_UPDATE_CHARAC_VIEW server group offset drift");


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_VIEW_DATA_H_
