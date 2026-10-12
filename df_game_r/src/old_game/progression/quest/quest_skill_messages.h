#ifndef OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_MESSAGES_H_
#define OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_MESSAGES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x822DCAA in the companion source.


#include <stddef.h>
#include <stdint.h>

#include "old_game/player/core/character_view_data.h"
#include "old_game/progression/skill/combo_skill.h"
#include "infra/network/packet/message_base.h"


#pragma pack(push, 1)

// IDA unit: 08_quest_skill_growth/MSG_BUY_SKILL.
struct MSG_BUY_SKILL : public MSG_BASE {
    struct SkillRequest {
        uint8_t skill_index;  // IDA +0x00 in each three-byte record
        uint8_t skill_level;  // IDA +0x01 in each three-byte record
        uint8_t skill_kind;   // IDA +0x02 in each three-byte record
    };

    MSG_BUY_SKILL();

    uint8_t skill_tree_index;  // IDA +0x0d
    uint8_t request_count;     // IDA +0x0e, maximum 70
    SkillRequest requests[70]; // IDA +0x0f
};

// IDA unit: 08_quest_skill_growth/MSG_CHANGE_ANOTHER_SKILL_TREE.
struct MSG_CHANGE_ANOTHER_SKILL_TREE : public MSG_BASE {
    MSG_CHANGE_ANOTHER_SKILL_TREE();

    uint8_t skill_tree_index;  // IDA +0x0d
};

// IDA unit: 08_quest_skill_growth/MSG_CHANGE_SKILLSLOT.  There is no
// dedicated MSG C2 identity in the old ELF: its state-3 template uses a
// 16-byte automatic message and reads exactly these three payload bytes.
// The implicit MSG_BASE C2 is itself a genuine no-op.
struct MSG_CHANGE_SKILLSLOT : public MSG_BASE {
    uint8_t skill_tree_index;  // IDA +0x0d
    uint8_t source_slot;       // IDA +0x0e
    uint8_t destination_slot;  // IDA +0x0f
};

// IDA unit: 08_quest_skill_growth/MSG_GROWTH_POWER.
struct MSG_GROWTH_POWER : public MSG_BASE {
    MSG_GROWTH_POWER();

    uint8_t selection_count;    // IDA +0x0d
    uint8_t reward_selections[5]; // IDA +0x0e..+0x12
};

// IDA unit: 08_quest_skill_growth/MSG_REFUND_SKILL.
struct MSG_REFUND_SKILL : public MSG_BASE {
    MSG_REFUND_SKILL();

    uint32_t inventory_item_id;  // IDA +0x0d
    // CMD427 accepts 0 for the normal tree and 1 for the alternate tree.
    // The dispatcher rejects a value that does not match the live tree.
    uint8_t skill_tree_index;    // IDA +0x11
    uint8_t skill_index;         // IDA +0x12
    uint8_t refund_count;        // IDA +0x13
};

// IDA unit: 08_quest_skill_growth/Dispatcher_RequestDisjointItem.
// CMD239 contains the owner machine UID, then the requester's source slot
// and item-space.  The old state-3 template creates this packed 0x12-byte
// message directly on its stack.
struct MSG_REQUEST_DISJOINT_ITEM : public MSG_BASE {
    MSG_REQUEST_DISJOINT_ITEM();

    uint16_t owner_unique_id;  // IDA +0x0d
    uint16_t item_slot;        // IDA +0x0f
    uint8_t item_space;        // IDA +0x11
};

// IDA unit: 08_quest_skill_growth/MSG_REQUEST_GUILD_JOIN.
struct MSG_REQUEST_GUILD_JOIN : public MSG_BASE {
    MSG_REQUEST_GUILD_JOIN();

    // Dispatcher_RequestJoinGuild @0x81D4E28 copies this first string to
    // SIG_REQUEST_JOIN_GUILD::guild_name, then copies the second string to
    // its memo field.  The former provisional names inverted those two
    // protocol meanings without changing the wire offsets.
    uint32_t guild_name_length;  // IDA +0x0d
    char guild_name[23];         // IDA +0x11
    uint32_t memo_length;        // IDA +0x28
    char memo[26];               // IDA +0x2c
};

// IDA unit: 08_quest_skill_growth/MSG_REQUEST_INGAME_ADVERTISEMENT.
struct MSG_REQUEST_INGAME_ADVERTISEMENT : public MSG_BASE {
    enum { kMaximumRequestCount = 255 };

    MSG_REQUEST_INGAME_ADVERTISEMENT();

    // 140828 sender sub_70DE00 writes one count byte followed by that many
    // u16 advertisement IDs.  Keep the complete native request in the
    // dispatcher message instead of overwriting the old single-ID field.
    uint8_t request_count;  // native wire +0x00
    uint16_t request_ids[kMaximumRequestCount];  // native wire +0x01
};

// IDA unit: 08_quest_skill_growth/MSG_SAVE_QUEST_NOTIFY.
struct MSG_SAVE_QUEST_NOTIFY : public MSG_BASE {
    MSG_SAVE_QUEST_NOTIFY();

    uint32_t quest_ids[10];  // IDA +0x0d
};

// IDA unit: 08_quest_skill_growth/MSG_SKILL_COMMAND.
struct MSG_SKILL_COMMAND : public MSG_BASE {
    struct CommandRequest {
        uint16_t skill_index;   // IDA +0x00 in each eight-byte record
        uint8_t command_length; // IDA +0x02, maximum 5
        uint8_t command[5];     // IDA +0x03
    };

    MSG_SKILL_COMMAND();

    uint8_t request_count;       // IDA +0x0d; parser accepts at most 49
    CommandRequest requests[50]; // IDA +0x0e
};

// IDA unit: 08_quest_skill_growth/MSG_SKILL_INIT.
struct MSG_SKILL_INIT : public MSG_BASE {
    MSG_SKILL_INIT();

    uint8_t skill_tree_index;  // IDA +0x0d
};

// IDA unit: 08_quest_skill_growth/MSG_COMBO_SKILL.
struct MSG_COMBO_SKILL : public MSG_BASE {
    MSG_COMBO_SKILL();
    ~MSG_COMBO_SKILL();

    uint8_t skill_tree_index;       // IDA +0x0d
    ComboSkillMap combo_skills;     // IDA +0x0e, old packed map ABI
};

// IDA unit: 08_quest_skill_growth/MSG_PVP_MISSION_HP_PERCENT.
struct MSG_PVP_MISSION_HP_PERCENT : public MSG_BASE {
    MSG_PVP_MISSION_HP_PERCENT();
    ~MSG_PVP_MISSION_HP_PERCENT();

    uint8_t byte_0d;  // IDA +0x0d; raw C2 leaves it untouched.
    std::vector<short> percent_0e;       // IDA +0x0e
    std::vector<int> mission_indexes_1a; // IDA +0x1a
};

// IDA unit: 08_quest_skill_growth/MsgClearQuestIndex.
struct MsgClearQuestIndex : public MSG_BASE {
    MsgClearQuestIndex();
    ~MsgClearQuestIndex();

    MsgClearQuestIndex& operator=(const MsgClearQuestIndex& rhs);

    uint8_t prefix_0d[0x03];  // IDA +0x0d..+0x0f; C2 leaves it untouched.
    // IDA 0x81DF892 sign-extends the received short before writing this
    // field.  It is the requested/returned quest-count scalar, not an
    // unsigned opaque dword.
    int32_t quest_count;      // IDA +0x10; copied by operator=.
    std::vector<int> quest_indexes_14;  // IDA +0x14
};

// IDA unit: 08_quest_skill_growth/MsgRequestAssaultPrice.
struct MsgRequestAssaultPrice : public MSG_BASE {
    MsgRequestAssaultPrice();

    uint16_t item_unique_id;  // IDA +0x0d
};

// IDA unit: 13_database_dispatch/MSG_REGISTER_TO_BLACKLIST and
// MSG_DELETE_TO_BLACKLIST.  The old templates reserve 0x34 stack bytes for
// this 0x32-byte packed message; the extra two bytes are compiler stack
// alignment, not protocol fields.  Both commands read the same wire body.
struct MSG_REGISTER_TO_BLACKLIST : public MSG_BASE {
    uint8_t reserved_0d[3];      // IDA +0x0d..+0x0f
    uint32_t target_name_length; // IDA +0x10
    char target_name[30];        // IDA +0x14
};

struct MSG_DELETE_TO_BLACKLIST : public MSG_BASE {
    uint8_t reserved_0d[3];      // IDA +0x0d..+0x0f
    uint32_t target_name_length; // IDA +0x10
    char target_name[30];        // IDA +0x14
};

// IDA units: 04_character_select_create/STGuildMemberCharacData and
// 99_misc_global/STGuildBoardDBInfo.  The shared character-select header owns
// the 33-byte character record and its C2; this packet only embeds it.
struct STGuildBoardDBInfo {
    STGuildBoardDBInfo();

    char text[0x78];                 // +0x00
    uint32_t created_time;           // +0x78
    uint32_t board_index;            // +0x7c
    uint32_t author_charac_no;       // +0x80
    STGuildMemberCharacData author;  // +0x84
};

// IDA unit: 99_misc_global/MSG_GUILD_BOARD.  The 0xb3-byte automatic extent
// is corroborated by IPacketDispatcher<MSG_GUILD_BOARD,...>@0x8252666.
struct MSG_GUILD_BOARD : public MSG_BASE {
    MSG_GUILD_BOARD();

    STGuildBoardDBInfo board;  // +0x0d, text/board_index are read by CMD342/343
    uint8_t request_kind;      // +0xb2, read by CMD341
};

#pragma pack(pop)

static_assert(sizeof(MSG_CHANGE_SKILLSLOT) == 0x10u,
              "CMD30 request must retain the old 16-byte packet ABI");
static_assert(offsetof(MSG_GROWTH_POWER, selection_count) == 0x0du &&
                  offsetof(MSG_GROWTH_POWER, reward_selections) == 0x0eu &&
                  sizeof(MSG_GROWTH_POWER) == 0x13u,
              "CMD432 growth-power selection packet ABI changed");
static_assert(sizeof(MSG_REQUEST_DISJOINT_ITEM) == 0x12u,
              "CMD239 request must retain the old 18-byte packet ABI");
static_assert(sizeof(MSG_REGISTER_TO_BLACKLIST) == 0x32u &&
                  sizeof(MSG_DELETE_TO_BLACKLIST) == 0x32u,
              "CMD121/122 requests must retain their old packed extent");
static_assert(sizeof(STGuildMemberCharacData) == 0x21u &&
                  sizeof(STGuildBoardDBInfo) == 0xa5u &&
                  sizeof(MSG_GUILD_BOARD) == 0xb3u,
              "CMD341--343 guild-board ABI extent changed");
#if defined(__i386__)
static_assert(offsetof(MSG_COMBO_SKILL, skill_tree_index) == 0x0du &&
                  offsetof(MSG_COMBO_SKILL, combo_skills) == 0x0eu &&
                  sizeof(MSG_COMBO_SKILL) == 0x23u,
              "CMD509 MSG_COMBO_SKILL must retain the old 0x23 ABI");
#endif


#endif  // OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_MESSAGES_H_
