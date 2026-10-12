#include "old_game/progression/quest/quest_skill_messages.h"

#include <cstring>


typedef char MSG_BUY_SKILL_Size_Must_Be_0xe1[
    (sizeof(MSG_BUY_SKILL) == 0xe1) ? 1 : -1];
typedef char MSG_CHANGE_ANOTHER_SKILL_TREE_Size_Must_Be_0x0e[
    (sizeof(MSG_CHANGE_ANOTHER_SKILL_TREE) == 0x0e) ? 1 : -1];
typedef char MSG_GROWTH_POWER_Size_Must_Be_0x13[
    (sizeof(MSG_GROWTH_POWER) == 0x13) ? 1 : -1];
typedef char MSG_REFUND_SKILL_Size_Must_Be_0x14[
    (sizeof(MSG_REFUND_SKILL) == 0x14) ? 1 : -1];
typedef char MSG_REQUEST_DISJOINT_ITEM_Size_Must_Be_0x12[
    (sizeof(MSG_REQUEST_DISJOINT_ITEM) == 0x12) ? 1 : -1];
typedef char MSG_REQUEST_GUILD_JOIN_Size_Must_Be_0x46[
    (sizeof(MSG_REQUEST_GUILD_JOIN) == 0x46) ? 1 : -1];
typedef char MSG_REQUEST_INGAME_ADVERTISEMENT_Size_Must_Be_0x20c[
    (sizeof(MSG_REQUEST_INGAME_ADVERTISEMENT) == 0x20c) ? 1 : -1];
typedef char MSG_SAVE_QUEST_NOTIFY_Size_Must_Be_0x35[
    (sizeof(MSG_SAVE_QUEST_NOTIFY) == 0x35) ? 1 : -1];
typedef char MSG_SKILL_COMMAND_Size_Must_Be_0x19e[
    (sizeof(MSG_SKILL_COMMAND) == 0x19e) ? 1 : -1];
typedef char MSG_SKILL_INIT_Size_Must_Be_0x0e[
    (sizeof(MSG_SKILL_INIT) == 0x0e) ? 1 : -1];
typedef char MSG_COMBO_SKILL_Size_Must_Be_0x23[
    (sizeof(MSG_COMBO_SKILL) == 0x23) ? 1 : -1];
typedef char MSG_PVP_MISSION_HP_PERCENT_Size_Must_Be_0x26[
    (sizeof(MSG_PVP_MISSION_HP_PERCENT) == 0x26) ? 1 : -1];
typedef char MsgClearQuestIndex_Size_Must_Be_0x20[
    (sizeof(MsgClearQuestIndex) == 0x20) ? 1 : -1];
typedef char MsgRequestAssaultPrice_Size_Must_Be_0x0f[
    (sizeof(MsgRequestAssaultPrice) == 0x0f) ? 1 : -1];

// IDA_EA: 0x822DCAA
// IDA_NAME: _ZN13MSG_BUY_SKILLC2Ev
// DEMANGLED: MSG_BUY_SKILL::MSG_BUY_SKILL(void)
MSG_BUY_SKILL::MSG_BUY_SKILL()
    : MSG_BASE(),
      skill_tree_index(0),
      request_count(0) {
    // The original C2 clears only the first four bytes at +0x0f.
    std::memset(requests, 0, 4u);
}

// IDA_EA: 0x822DFF8
// IDA_NAME: _ZN29MSG_CHANGE_ANOTHER_SKILL_TREEC2Ev
// DEMANGLED: MSG_CHANGE_ANOTHER_SKILL_TREE::MSG_CHANGE_ANOTHER_SKILL_TREE(void)
MSG_CHANGE_ANOTHER_SKILL_TREE::MSG_CHANGE_ANOTHER_SKILL_TREE()
    : MSG_BASE(),
      skill_tree_index(0xffu) {}

// IDA_EA: 0x822DF6C
// IDA_NAME: _ZN16MSG_GROWTH_POWERC2Ev
// DEMANGLED: MSG_GROWTH_POWER::MSG_GROWTH_POWER(void)
MSG_GROWTH_POWER::MSG_GROWTH_POWER()
    : MSG_BASE(),
      selection_count(0),
      reward_selections() {}

// IDA_EA: 0x822DEEA
// IDA_NAME: _ZN16MSG_REFUND_SKILLC2Ev
// DEMANGLED: MSG_REFUND_SKILL::MSG_REFUND_SKILL(void)
MSG_REFUND_SKILL::MSG_REFUND_SKILL()
    : MSG_BASE(),
      inventory_item_id(0),
      skill_tree_index(0),
      skill_index(0),
      refund_count(0) {}

// IDA_EA: 0x822DF16 (the old C2 only constructs MSG_BASE; the three fields
// are subsequently filled by Dispatcher_RequestDisjointItem::read).
MSG_REQUEST_DISJOINT_ITEM::MSG_REQUEST_DISJOINT_ITEM()
    : MSG_BASE(),
      owner_unique_id(0),
      item_slot(0),
      item_space(0) {}

// IDA_EA: 0x822DDB8
// IDA_NAME: _ZN22MSG_REQUEST_GUILD_JOINC2Ev
// DEMANGLED: MSG_REQUEST_GUILD_JOIN::MSG_REQUEST_GUILD_JOIN(void)
MSG_REQUEST_GUILD_JOIN::MSG_REQUEST_GUILD_JOIN()
    : MSG_BASE(),
      guild_name_length(0),
      guild_name(),
      memo_length(0),
      memo() {}

// IDA_EA: 0x822E2F2
// IDA_NAME: _ZN32MSG_REQUEST_INGAME_ADVERTISEMENTC2Ev
// DEMANGLED: MSG_REQUEST_INGAME_ADVERTISEMENT::MSG_REQUEST_INGAME_ADVERTISEMENT(void)
MSG_REQUEST_INGAME_ADVERTISEMENT::MSG_REQUEST_INGAME_ADVERTISEMENT()
    : MSG_BASE(),
      request_count(0),
      request_ids() {}

// IDA_EA: 0x82603A4
// IDA_NAME: _ZN21MSG_SAVE_QUEST_NOTIFYC2Ev
// DEMANGLED: MSG_SAVE_QUEST_NOTIFY::MSG_SAVE_QUEST_NOTIFY(void)
MSG_SAVE_QUEST_NOTIFY::MSG_SAVE_QUEST_NOTIFY()
    : MSG_BASE(),
      quest_ids() {}

// IDA_EA: 0x822DBAA
// IDA_NAME: _ZN17MSG_SKILL_COMMANDC2Ev
// DEMANGLED: MSG_SKILL_COMMAND::MSG_SKILL_COMMAND(void)
MSG_SKILL_COMMAND::MSG_SKILL_COMMAND()
    : MSG_BASE() {
    std::memset(this, 0, 4u);
}

// IDA_EA: 0x822E31A
// IDA_NAME: _ZN14MSG_SKILL_INITC2Ev
// DEMANGLED: MSG_SKILL_INIT::MSG_SKILL_INIT(void)
MSG_SKILL_INIT::MSG_SKILL_INIT()
    : MSG_BASE(),
      skill_tree_index(0) {}

// IDA_EA: 0x82603D6
// IDA_NAME: _ZN15MSG_COMBO_SKILLC2Ev
// DEMANGLED: MSG_COMBO_SKILL::MSG_COMBO_SKILL(void)
MSG_COMBO_SKILL::MSG_COMBO_SKILL()
    : MSG_BASE(),
      skill_tree_index(0),
      combo_skills() {
    combo_skills.clear();
}

// IDA_EA: 0x8261524
// IDA_NAME: _ZN15MSG_COMBO_SKILLD2Ev
// DEMANGLED: MSG_COMBO_SKILL::~MSG_COMBO_SKILL()
MSG_COMBO_SKILL::~MSG_COMBO_SKILL() {}

// IDA_EA: 0x8250C7C
// IDA_NAME: _ZN26MSG_PVP_MISSION_HP_PERCENTC2Ev
// DEMANGLED: MSG_PVP_MISSION_HP_PERCENT::MSG_PVP_MISSION_HP_PERCENT(void)
MSG_PVP_MISSION_HP_PERCENT::MSG_PVP_MISSION_HP_PERCENT()
    : MSG_BASE(),
      percent_0e(),
      mission_indexes_1a() {}

// IDA_EA: 0x8250CD2
// IDA_NAME: _ZN26MSG_PVP_MISSION_HP_PERCENTD2Ev
// DEMANGLED: MSG_PVP_MISSION_HP_PERCENT::~MSG_PVP_MISSION_HP_PERCENT()
MSG_PVP_MISSION_HP_PERCENT::~MSG_PVP_MISSION_HP_PERCENT() {
}

// IDA_EA: 0x8231A3A
// IDA_NAME: _ZN18MsgClearQuestIndexC2Ev
// DEMANGLED: MsgClearQuestIndex::MsgClearQuestIndex(void)
MsgClearQuestIndex::MsgClearQuestIndex()
    : MSG_BASE(),
      quest_indexes_14() {}

// IDA_EA: 0x8231A5C
// IDA_NAME: _ZN18MsgClearQuestIndexD2Ev
// DEMANGLED: MsgClearQuestIndex::~MsgClearQuestIndex()
MsgClearQuestIndex::~MsgClearQuestIndex() {
}

// IDA_EA: 0x8235266
// IDA_NAME: _ZN18MsgClearQuestIndexaSERKS_
// DEMANGLED: MsgClearQuestIndex::operator=(MsgClearQuestIndex const&)
MsgClearQuestIndex& MsgClearQuestIndex::operator=(
    const MsgClearQuestIndex& rhs) {
    static_cast<MSG_BASE&>(*this) = static_cast<const MSG_BASE&>(rhs);
    quest_count = rhs.quest_count;
    quest_indexes_14 = rhs.quest_indexes_14;
    return *this;
}

// IDA_EA: 0x822E08E
// IDA_NAME: _ZN22MsgRequestAssaultPriceC2Ev
// DEMANGLED: MsgRequestAssaultPrice::MsgRequestAssaultPrice(void)
MsgRequestAssaultPrice::MsgRequestAssaultPrice()
    : MSG_BASE(),
      item_unique_id(0xffffu) {}

// IDA_EA: 0x822DBD8
// IDA_NAME: _ZN15MSG_GUILD_BOARDC2Ev
// DEMANGLED: MSG_GUILD_BOARD::MSG_GUILD_BOARD(void)
MSG_GUILD_BOARD::MSG_GUILD_BOARD()
    : MSG_BASE(),
      board(),
      request_kind(0) {}
