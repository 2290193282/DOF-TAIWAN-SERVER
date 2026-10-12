#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_MESSAGES_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_MESSAGES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x822D9D8 in the companion source.


#include <stdint.h>

#include <string>
#include <vector>

#include "old_game/player/setting/character_option.h"
#include "infra/network/packet/message_base.h"


#pragma pack(push, 1)

// IDA unit: 04_character_select_create/MSG_CHARAC_SELECT.
struct MSG_CHARAC_SELECT : public MSG_BASE {
    MSG_CHARAC_SELECT();

    uint8_t character_slot;          // IDA +0x0d
    uint32_t tag_character_no;       // IDA +0x0e
    uint8_t striker_skill_index;     // IDA +0x12
};

// IDA unit: 04_character_select_create/DisPatcher_CreateCharac read payload.
struct MSG_CREATE_CHARACTER : public MSG_BASE {
    uint8_t character_job;              // IDA +0x0d
    uint32_t character_name_length;      // IDA +0x0e
    char character_name[0x1d];           // IDA +0x12, read cap 29
    // The native client writes the selected event-character UI ordinal first,
    // then its canonical grow type. Ordinary creation writes zero for both.
    uint8_t event_character_selection_ordinal;  // IDA +0x2f
    uint8_t event_character_grow_type;           // native wire +0x30
};

// IDA unit: 04_character_select_create/DisPatcher_DeleteCharac read payload.
struct MSG_DELETE_CHARACTER : public MSG_BASE {
    uint8_t character_slot;              // IDA +0x0d
    uint32_t character_name_length;      // IDA +0x0e
    char character_name[0x1e];           // IDA +0x12, read cap 30
};

// IDA unit: 04_character_select_create/MSG_DIE_CHARACTER.  CMD43 reads two
// u16 values directly after the 13-byte message base.
struct MSG_DIE_CHARACTER : public MSG_BASE {
    uint16_t death_reason;  // IDA +0x0d
    uint16_t death_value;   // IDA +0x0f
};

// IDA unit: 04_character_select_create/MSG_REQUEST_CHARAC_SKILL_INFO.
struct MSG_REQUEST_CHARAC_SKILL_INFO : public MSG_BASE {
    MSG_REQUEST_CHARAC_SKILL_INFO();

    uint8_t character_slot;  // IDA +0x0d
};

// IDA unit: 04_character_select_create/Dispatcher_Equipslot_Switch.
// The dispatcher-template local is 0x10 bytes: the 13-byte C2S base plus
// these three request bytes.  Keep the old wire order rather than folding
// the channel and expanded-slot switches into an unnamed payload.
struct MSG_EQUIPSLOT_SWITCH : public MSG_BASE {
    uint8_t channel_equipslot_switch;  // IDA +0x0d
    uint8_t expand_equipslot_switch;   // IDA +0x0e
    uint8_t requested_switch;          // IDA +0x0f
};

// IDA unit: 04_character_select_create/
// Dispatcher_Expand_Equipslot_Flag_Update.
struct MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE : public MSG_BASE {
    uint8_t requested_flag;  // IDA +0x0d
};

// IDA unit: 04_character_select_create/MSG_SAVE_CHARACTER_OPTION.
struct MSG_SAVE_CHARACTER_OPTION : public MSG_BASE {
    MSG_SAVE_CHARACTER_OPTION();

    // The packet reader stores the received CharacterOption byte count here.
    // C2 deliberately leaves it uninitialized until read(), matching 0627.
    uint32_t option_size;       // IDA +0x0d
    CharacterOption option_11;  // IDA +0x11
};

// IDA unit:
// 04_character_select_create/MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET.
struct MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET : public MSG_BASE {
    MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET();

    uint8_t byte_0d;          // IDA +0x0d
};

#pragma pack(pop)

static_assert(sizeof(MSG_EQUIPSLOT_SWITCH) == 0x10u,
              "CMD397 request must preserve its 16-byte old ABI");
static_assert(sizeof(MSG_DIE_CHARACTER) == 0x11u,
              "CMD43 request must preserve its 17-byte old ABI");
static_assert(sizeof(MSG_REQUEST_CHARAC_SKILL_INFO) == 0x0eu &&
                  offsetof(MSG_REQUEST_CHARAC_SKILL_INFO, character_slot) ==
                      0x0du,
              "request-character-skill-info wire fields moved");
static_assert(sizeof(MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE) == 0x0eu,
              "CMD398 request must preserve its 14-byte old ABI");
static_assert(sizeof(MSG_CHARAC_SELECT) == 0x13u,
              "CMD select-character request must stay 19 bytes");
static_assert(offsetof(MSG_CHARAC_SELECT, character_slot) == 0x0du &&
                  offsetof(MSG_CHARAC_SELECT, tag_character_no) == 0x0eu &&
                  offsetof(MSG_CHARAC_SELECT, striker_skill_index) == 0x12u,
              "select-character wire fields moved");
static_assert(sizeof(MSG_CREATE_CHARACTER) == 0x31u,
              "native CMD5 create-character request must be 49 bytes");
static_assert(offsetof(MSG_CREATE_CHARACTER, character_job) == 0x0du &&
                  offsetof(MSG_CREATE_CHARACTER, character_name_length) ==
                      0x0eu &&
                  offsetof(MSG_CREATE_CHARACTER, character_name) == 0x12u &&
                  offsetof(MSG_CREATE_CHARACTER,
                           event_character_selection_ordinal) == 0x2fu &&
                  offsetof(MSG_CREATE_CHARACTER, event_character_grow_type) ==
                      0x30u,
              "native create-character wire fields moved");
static_assert(sizeof(MSG_DELETE_CHARACTER) == 0x30u,
              "CMD delete-character request must stay 48 bytes");
static_assert(offsetof(MSG_DELETE_CHARACTER, character_slot) == 0x0du &&
                  offsetof(MSG_DELETE_CHARACTER, character_name_length) ==
                      0x0eu &&
                  offsetof(MSG_DELETE_CHARACTER, character_name) == 0x12u,
              "delete-character wire fields moved");
static_assert(offsetof(MSG_SAVE_CHARACTER_OPTION, option_size) == 0x0du &&
                  offsetof(MSG_SAVE_CHARACTER_OPTION, option_11) == 0x11u,
              "save-character-option wire fields moved");

// The nested slot object has a separate existing canonical provider in
// character_messages.cpp.  The enclosing request was observed at an odd
// packed vector offset (+0x0f) and must not be replaced by a byte buffer.
#pragma pack(push, 1)
struct MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON : public MSG_BASE {
    struct _slot_String {
        _slot_String();
        ~_slot_String();
        _slot_String(const _slot_String& rhs);
        _slot_String& operator=(const _slot_String& rhs);

        uint16_t word_00;  // IDA +0x00
        char text_02[sizeof(std::string)];  // IDA +0x02 std::string storage
    };

    MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON();
    ~MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON();

    uint16_t slot_count;  // IDA +0x0d
    std::vector<_slot_String> slots;  // IDA +0x0f
};
#pragma pack(pop)

#if defined(__i386__)
static_assert(sizeof(MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON) == 0x1bu,
              "CMD473 request must retain its packed vector offset and extent");
#endif


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_MESSAGES_H_
