#include "old_data/PVF/n_quest/quest_skill_script.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

#include "common/config.h"
#include "common/manager/global_data.h"
#include "old_data/PVF/common/rdar_script.h"

extern char g_questScriptBaseDirectory[256];

namespace {

bool ScanRequiredInt(int& value) {
    bool success = false;
    value = ScanInt(&success);
    return success;
}

int QuestConditionType(const std::string& tag) {
    static const struct {
        const char* tag;
        int value;
    } kTypes[] = {
        {"[seeking]", 0},
        {"[condition under clear]", 1},
        {"[hunt monster]", 2},
        {"[clear map]", 3},
        {"[meet npc]", 4},
        {"[pvp rank]", 5},
        {"[hunt enemy]", 6},
        {"[powerwar win]", 7},
        {"[powerwar point]", 8},
        {"[mobile]", 9},
        {"[use item]", 10},
        {"[get score]", 11},
        {"[clear quest]", 12},
        {"[check time]", 13},
        {"[check life]", 14},
        {"[amplify item]", 15},
        {"[disjoint item]", 16},
        {"[equipped item]", 17},
        {"[get item]", 18},
        {"[get item check index]", 18},
        {"[custom quest]", 19},
        {"[send chatting]", 20},
        {"[turn gold card]", 21},
        {"[use fortune coin]", 22},
        {"[meet secret npc]", 23},
        {"[ui click]", 24},
        {"[pvp quest]", 24},
        {"[pvp match]", 24},
        {"[pvp win]", 24},
        {"[pvp lose]", 24},
        {"[combo success]", 24},
        {"[seek n meet npc]", 25},
        {"[assault count]", 26},
    };

    for (size_t index = 0; index != sizeof(kTypes) / sizeof(kTypes[0]);
         ++index) {
        if (tag == kTypes[index].tag) {
            return kTypes[index].value;
        }
    }
    return -1;
}

bool ParseQuestSchedule(QuestScript& script, const char* filename) {
    const int quest_index = g_questScriptFileList.FindIndexByFullScan(filename);
    std::string start_date;
    std::string end_date;
    std::string weekday_text;
    std::string start_time_text;
    std::string end_time_text;

    while (true) {
        _DailySchedule schedule;
        std::memset(&schedule, 0, sizeof(schedule));
        bool success = false;
        schedule.dword_00 = ScanInt(&success);
        if (!success) {
            return true;
        }
        schedule.schedule_type = DAILY_SCHEDULE_QUEST;
        schedule.schedule_id = static_cast<unsigned int>(quest_index);

        // The old function treats the first missing date string as the normal
        // end of this optional row list, while later missing fields fail the
        // complete quest import.
        if (ScanStr(&start_date) != 1) {
            return true;
        }
        if (ScanStr(&end_date) != 1 || ScanStr(&weekday_text) != 1 ||
            ScanStr(&start_time_text) != 1 || ScanStr(&end_time_text) != 1) {
            return false;
        }

        long start_date_value = 0;
        long end_date_value = 0;
        unsigned int weekday = 0;
        unsigned char start_hour = 0;
        unsigned char start_minute = 0;
        unsigned char start_second = 0;
        unsigned char end_hour = 0;
        unsigned char end_minute = 0;
        unsigned char end_second = 0;
        if (ConvertStringToTime(start_date, start_date_value) != 1 ||
            ConvertStringToTime(end_date, end_date_value) != 1 ||
            ConvertWeekdayStringToInt(weekday_text, weekday) != 1 ||
            ConvertHHMMSSStringToInt(start_time_text,
                                     start_hour,
                                     start_minute,
                                     start_second) != 1 ||
            ConvertHHMMSSStringToInt(end_time_text,
                                     end_hour,
                                     end_minute,
                                     end_second) != 1) {
            return false;
        }

        // _DailySchedule's timing subfields are an already-established ABI
        // region used by importDailyScheduleEtcScript.  The original quest
        // importer writes the same fields and keeps state zeroed.
        const int32_t start_value = static_cast<int32_t>(start_date_value);
        const int32_t end_value = static_cast<int32_t>(end_date_value);
        std::memcpy(&schedule.reserved_0c[0], &start_value, sizeof(start_value));
        std::memcpy(&schedule.reserved_0c[4], &end_value, sizeof(end_value));
        std::memcpy(&schedule.reserved_0c[8], &weekday, sizeof(weekday));
        schedule.reserved_0c[12] = start_hour;
        schedule.reserved_0c[13] = start_minute;
        schedule.reserved_0c[14] = start_second;
        schedule.reserved_0c[15] = end_hour;
        schedule.reserved_0c[16] = end_minute;
        schedule.reserved_0c[17] = end_second;
        script.schedules.push_back(schedule);
    }
}

void ParseMonsterRewardItems(std::vector<MonsterKillItem>& output,
                             bool enemy_reward) {
    output.clear();
    while (true) {
        MonsterKillItem item;
        bool success = false;
        item.monster_index = ScanInt(&success);
        if (!success) {
            break;
        }
        if (enemy_reward) {
            item.target_type = ScanInt(&success);
        }
        item.required_count = ScanInt(static_cast<bool*>(0));
        item.current_count = ScanInt(static_cast<bool*>(0));
        item.map_index = ScanInt(static_cast<bool*>(0));
        item.difficulty = ScanInt(static_cast<bool*>(0));
        item.value_18 = ScanInt(static_cast<bool*>(0));
        item.value_1c = ScanInt(static_cast<bool*>(0));
        output.push_back(item);
    }
}

bool ParseQuestException(QuestScript& script) {
    while (true) {
        bool is_type = false;
        std::string token;
        if (ScanTypeOrStr(is_type, token, true) != 1) {
            return false;
        }
        if (is_type && token == "[/exception quest]") {
            return true;
        }
        if (token != "[urgent]") {
            return false;
        }
        script.exception_quest_types.push_back(QUEST_EXCEPTION_URGENT);
    }
}

void ParseQuestDialog(QuestScript& script) {
    dungeonEventHanaseStruct dialog;
    dialog.raw_00 = static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
    dialog.raw_04 = static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
    dialog.raw_08 = static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
    dialog.raw_0c = static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
    dialog.raw_10 = static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)) != 0);
    dialog.raw_14 = static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
    dialog.raw_18 = static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
    if (script.job_message_enabled != 0) {
        ScanJobStr(&dialog.default_dialog, dialog.job_dialogs);
    } else {
        ScanStr(&dialog.default_dialog);
    }
    bool is_type = false;
    std::string ignored;
    ScanTypeOrStr(is_type, ignored, true);
    script.dungeon_event_dialogs.push_back(dialog);
}

}  // namespace

// IDA_EA: 0x83C5A34
// IDA_NAME: _ZN15MonsterKillItemD2Ev
// DEMANGLED: MonsterKillItem::~MonsterKillItem()
MonsterKillItem::~MonsterKillItem() {}

// IDA_EA: 0x869383A
// IDA_NAME: _ZN15MonsterKillItem5clearEv
// DEMANGLED: MonsterKillItem::clear(void)
MonsterKillItem* MonsterKillItem::clear() {
    target_type = 1;
    monster_index = 0;
    required_count = -1;
    current_count = -1;
    map_index = 0;
    difficulty = 0;
    value_18 = 0;
    value_1c = -1;
    return this;
}

// IDA_EA: 0x869388E
// IDA_NAME: _ZN15MonsterKillItemC2Ev
// DEMANGLED: MonsterKillItem::MonsterKillItem(void)
MonsterKillItem::MonsterKillItem() { clear(); }

// IDA_EA: 0x836DEBA
// IDA_NAME: _ZN11QuestScript5clearEv
// DEMANGLED: QuestScript::clear(void)
QuestScript* QuestScript::clear() {
    int i;

    quest_index = -1;
    name.clear();
    request_message.clear();
    condition_message.clear();
    solve_message.clear();
    substitutive_names.clear();
    value_2e8.clear();
    show_npc_on_clear = false;
    show_click_on_complete_npc = false;
    no_dungeon_name = false;

    quest_grade = 4;
    quest_type = 3;
    value_0dc = 0;
    value_0e0 = -1;
    value_0e4 = 0;
    value_0e8 = -1;
    value_0ec = oldsrv::config::kCharacterJobCommon;
    value_0f0 = -1;
    value_100 = 0;
    value_104 = 0;
    value_108 = false;
    conditions.clear();
    condition_values.clear();
    value_124 = 0;
    value_128 = 0;
    value_12c = 0;
    value_130 = false;
    value_134 = -1;
    dungeon_clear_items.clear();
    monster_kill_items.clear();
    appear_map.clear();
    depend_give_items.clear();
    additional_dungeon_info.clear();
    value_178 = 0;
    value_17c = 0;
    limit_dungeon_indices.clear();
    value_18c.clear();
    value_198.clear();
    value_1b0.clear();
    value_1bc = false;
    value_1c0 = -1;
    value_1c4 = 4;
    target_characters.clear();
    value_1c8.clear();
    value_1d4 = 0;
    reward_items.clear();
    value_1e4 = -1;
    value_218 = 0;
    clear_reward_items.clear();
    relation_quest_indices.clear();
    light_reward_items.clear();
    interactive_dialogs.clear();
    dungeon_event_dialogs.clear();

    value_228 = -1;
    value_22c = false;
    exposed_by_npc = true;
    event = false;
    for (i = 0; i != 3; ++i) {
        auto_display[i] = -1;
    }
    has_substitutive_names = false;
    difficulty = 0;
    gold_multiple = 100;
    mail_sender.clear();
    mail_value_248 = -1;
    mail_value_24c = -1;
    mail_value_250 = -1;
    mail_value_254 = -1;
    mail_time = -1;
    mail_title.clear();
    mail_contents.clear();
    mail_enclose.clear();
    cant_giveup = false;
    searching_information = true;
    belong_to_winning_power = 0xFFu;
    grow_number = -1;
    value_278 = false;
    online = 0;
    job_message_enabled = 0;
    for (i = 0; i != oldsrv::config::kCharacterJobCount; ++i) {
        job_names[i] = 0;
        job_request_messages[i] = 0;
        job_condition_messages[i] = 0;
        job_solve_messages[i] = 0;
    }
    quest_point = 0;
    quest_piece = 0;
    for (i = 0; i != 4; ++i) {
        go_guide[i] = -1;
    }
    go_guide_message.clear();
    postal_reward.clear();
    expert_job_level = 0;
    expert_job_level_max = -1;
    change_dungeon_info.clear();
    has_change_dungeon_info_message = false;
    change_dungeon_info_message.clear();
    exception_quest_types.clear();
    schedules.clear();
    value_2ec = false;
    is_not_clear_levelup_system.clear();
    hide_npc_on_clear.clear();
    return this;
}

// IDA_EA: 0x836E4E2
// IDA_NAME: _ZN11QuestScriptC2Ev
// DEMANGLED: QuestScript::QuestScript(void)
QuestScript::QuestScript()
    : name(),
      request_message(),
      condition_message(),
      solve_message(),
      substitutive_names(),
      target_characters(),
      conditions(),
      condition_values(),
      dungeon_clear_items(),
      monster_kill_items(),
      appear_map(),
      depend_give_items(),
      additional_dungeon_info(),
      limit_dungeon_indices(),
      value_18c(),
      value_198(),
      value_1b0(),
      value_1c8(),
      reward_items(),
      light_reward_items(),
      interactive_dialogs(),
      dungeon_event_dialogs(),
      clear_reward_items(),
      relation_quest_indices(),
      mail_sender(),
      mail_title(),
      mail_contents(),
      mail_enclose(),
      go_guide_message(),
      postal_reward(),
      change_dungeon_info(),
      change_dungeon_info_message(),
      exception_quest_types(),
      schedules(),
      value_2e8(),
      is_not_clear_levelup_system(),
      hide_npc_on_clear() {
    clear();
}

// IDA_EA: 0x836EAF0
// IDA_NAME: _ZN11QuestScriptD2Ev
// DEMANGLED: QuestScript::~QuestScript()
QuestScript::~QuestScript() { destroy(); }

// IDA_EA: 0x837ADC8
// IDA_NAME: _ZN11QuestScriptC2ERKS_
// DEMANGLED: QuestScript::QuestScript(QuestScript const&)
QuestScript::QuestScript(const QuestScript& other)
    : quest_index(other.quest_index),
      name(other.name),
      request_message(other.request_message),
      condition_message(other.condition_message),
      solve_message(other.solve_message),
      job_message_enabled(other.job_message_enabled),
      substitutive_names(other.substitutive_names),
      quest_grade(other.quest_grade),
      quest_type(other.quest_type),
      value_0dc(other.value_0dc),
      value_0e0(other.value_0e0),
      value_0e4(other.value_0e4),
      value_0e8(other.value_0e8),
      value_0ec(other.value_0ec),
      value_0f0(other.value_0f0),
      target_characters(other.target_characters),
      value_100(other.value_100),
      value_104(other.value_104),
      value_108(other.value_108),
      conditions(other.conditions),
      condition_values(other.condition_values),
      value_124(other.value_124),
      value_128(other.value_128),
      value_12c(other.value_12c),
      value_130(other.value_130),
      value_134(other.value_134),
      dungeon_clear_items(other.dungeon_clear_items),
      monster_kill_items(other.monster_kill_items),
      appear_map(other.appear_map),
      depend_give_items(other.depend_give_items),
      additional_dungeon_info(other.additional_dungeon_info),
      value_178(other.value_178),
      value_17c(other.value_17c),
      limit_dungeon_indices(other.limit_dungeon_indices),
      value_18c(other.value_18c),
      value_198(other.value_198),
      value_1b0(other.value_1b0),
      value_1bc(other.value_1bc),
      value_1c0(other.value_1c0),
      value_1c4(other.value_1c4),
      value_1c8(other.value_1c8),
      value_1d4(other.value_1d4),
      reward_items(other.reward_items),
      value_1e4(other.value_1e4),
      light_reward_items(other.light_reward_items),
      interactive_dialogs(other.interactive_dialogs),
      dungeon_event_dialogs(other.dungeon_event_dialogs),
      clear_reward_items(other.clear_reward_items),
      value_218(other.value_218),
      relation_quest_indices(other.relation_quest_indices),
      value_228(other.value_228),
      value_22c(other.value_22c),
      exposed_by_npc(other.exposed_by_npc),
      event(other.event),
      has_substitutive_names(other.has_substitutive_names),
      difficulty(other.difficulty),
      gold_multiple(other.gold_multiple),
      mail_sender(other.mail_sender),
      mail_value_248(other.mail_value_248),
      mail_value_24c(other.mail_value_24c),
      mail_value_250(other.mail_value_250),
      mail_value_254(other.mail_value_254),
      mail_time(other.mail_time),
      mail_title(other.mail_title),
      mail_contents(other.mail_contents),
      mail_enclose(other.mail_enclose),
      cant_giveup(other.cant_giveup),
      searching_information(other.searching_information),
      belong_to_winning_power(other.belong_to_winning_power),
      grow_number(other.grow_number),
      value_278(other.value_278),
      online(other.online),
      quest_point(other.quest_point),
      quest_piece(other.quest_piece),
      go_guide_message(other.go_guide_message),
      postal_reward(other.postal_reward),
      expert_job_level(other.expert_job_level),
      expert_job_level_max(other.expert_job_level_max),
      change_dungeon_info(other.change_dungeon_info),
      has_change_dungeon_info_message(other.has_change_dungeon_info_message),
      change_dungeon_info_message(other.change_dungeon_info_message),
      exception_quest_types(other.exception_quest_types),
      schedules(other.schedules),
      value_2e8(other.value_2e8),
      value_2ec(other.value_2ec),
      is_not_clear_levelup_system(other.is_not_clear_levelup_system),
      hide_npc_on_clear(other.hide_npc_on_clear),
      show_npc_on_clear(other.show_npc_on_clear),
      show_click_on_complete_npc(other.show_click_on_complete_npc),
      no_dungeon_name(other.no_dungeon_name) {
    int i;
    for (i = 0; i != oldsrv::config::kCharacterJobCount; ++i) {
        job_names[i] = other.job_names[i];
        job_request_messages[i] = other.job_request_messages[i];
        job_condition_messages[i] = other.job_condition_messages[i];
        job_solve_messages[i] = other.job_solve_messages[i];
    }
    for (i = 0; i != 3; ++i) {
        auto_display[i] = other.auto_display[i];
    }
    for (i = 0; i != 4; ++i) {
        go_guide[i] = other.go_guide[i];
    }
}

// IDA_EA: 0x8A611FC
// IDA_NAME: _ZN11QuestScript7getNameE17ENUM_CHARACTERJOB
// DEMANGLED: QuestScript::getName(ENUM_CHARACTERJOB)
std::string* QuestScript::getName(ENUM_CHARACTERJOB job) {
    const unsigned int index = static_cast<unsigned int>(job);
    if (job_message_enabled != 0 &&
        index < static_cast<unsigned int>(oldsrv::config::kCharacterJobCount) &&
        job_names[index] != 0) {
        return job_names[index];
    }
    return &name;
}

// IDA_EA: 0x8A61240
// IDA_NAME: _ZN11QuestScript17getRequestMessageE17ENUM_CHARACTERJOB
// DEMANGLED: QuestScript::getRequestMessage(ENUM_CHARACTERJOB)
std::string* QuestScript::getRequestMessage(ENUM_CHARACTERJOB job) {
    const unsigned int index = static_cast<unsigned int>(job);
    if (job_message_enabled != 0 &&
        index < static_cast<unsigned int>(oldsrv::config::kCharacterJobCount) &&
        job_request_messages[index] != 0) {
        return job_request_messages[index];
    }
    return &request_message;
}

// IDA_EA: 0x8A61284
// IDA_NAME: _ZN11QuestScript19getConditionMessageE17ENUM_CHARACTERJOB
// DEMANGLED: QuestScript::getConditionMessage(ENUM_CHARACTERJOB)
std::string* QuestScript::getConditionMessage(ENUM_CHARACTERJOB job) {
    const unsigned int index = static_cast<unsigned int>(job);
    if (job_message_enabled != 0 &&
        index < static_cast<unsigned int>(oldsrv::config::kCharacterJobCount) &&
        job_condition_messages[index] != 0) {
        return job_condition_messages[index];
    }
    return &condition_message;
}

// IDA_EA: 0x8A612C6
// IDA_NAME: _ZN11QuestScript15getSolveMessageE17ENUM_CHARACTERJOB
// DEMANGLED: QuestScript::getSolveMessage(ENUM_CHARACTERJOB)
std::string* QuestScript::getSolveMessage(ENUM_CHARACTERJOB job) {
    const unsigned int index = static_cast<unsigned int>(job);
    if (job_message_enabled != 0 &&
        index < static_cast<unsigned int>(oldsrv::config::kCharacterJobCount) &&
        job_solve_messages[index] != 0) {
        return job_solve_messages[index];
    }
    return &solve_message;
}

// IDA_EA: 0x8A6130A
// IDA_NAME: _ZN11QuestScript7destroyEv
// DEMANGLED: QuestScript::destroy(void)
bool QuestScript::destroy() {
    int i;
    for (i = 0; i != oldsrv::config::kCharacterJobCount; ++i) {
        delete job_names[i];
        job_names[i] = 0;
        delete job_request_messages[i];
        job_request_messages[i] = 0;
        delete job_condition_messages[i];
        job_condition_messages[i] = 0;
        delete job_solve_messages[i];
        job_solve_messages[i] = 0;
    }
    return true;
}

// IDA_EA: 0x8A6143F
// IDA_NAME: _Z10ScanJobStrPSsPS_
// DEMANGLED: ScanJobStr(std::string *,std::string **)
int ScanJobStr(std::string* output, std::string** job_strings) {
    bool found = false;
    const DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_STRING) {
        *output = g_dnfLex.getString();
        return 1;
    }
    if (token != DNF_TOK_PAIR_INT) {
        return 0;
    }

    const int file_index = g_dnfLex.getPairInt();
    if (g_dnfLex.getToken() == DNF_TOK_PAIR_STR) {
        *output = g_scriptStringManager_.findString(
            file_index, g_dnfLex.getPairStr(), 0);
        found = true;
    }

    if (job_strings != 0) {
        char string_key[1024];
        for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
            std::snprintf(string_key,
                          sizeof(string_key),
                          "%s_%d",
                          g_dnfLex.getPairStr(),
                          job);
            bool job_found = false;
            const char* text = g_scriptStringManager_.findString(
                file_index, string_key, &job_found);
            if (text != 0 && job_found) {
                job_strings[job] = new std::string;
                *job_strings[job] = text;
            }
        }
    }
    return found ? 1 : 0;
}

// IDA_EA: 0x8A65DA5
// IDA_NAME: _Z15ScanRewardItemsRSt6vectorI14RewardItemInfoSaIS0_EESs
// DEMANGLED: ScanRewardItems(std::vector<RewardItemInfo,...> &,std::string)
bool ScanRewardItems(std::vector<RewardItemInfo>& output,
                     std::string closing_tag) {
    bool is_type = false;
    bool is_string = false;
    bool success = false;
    std::string text;

    while (true) {
        const int item_id = ScanTypeOrInt(is_type, text, success, true);
        if (is_type) {
            return text == closing_tag;
        }
        if (!success) {
            return false;
        }

        RewardItemInfo item;
        item.item_id = item_id;
        // 0828 has twelve concrete jobs; the common/all sentinel is 12.
        item.item_type = oldsrv::config::kCharacterJobCommon;
        item.period = -1;
        item.item_count = ScanStrOrInt(is_string, text, success, true);
        if (!success) {
            return false;
        }
        if (is_string) {
            if (text == "[job]") {
                if (ScanInt(&item.item_type) != 1 ||
                    ScanInt(&item.period) != 1) {
                    return false;
                }
            }
            if (ScanInt(&item.item_count) != 1) {
                return false;
            }
        }
        output.push_back(item);
    }
}

namespace {

bool ScanEventSkillRewards(std::vector<RewardItemInfo>& output,
                           const std::string& closing_tag) {
    // Official 0828 event quests encode the unlocked stage as a single
    // integer per row (0/1/2), unlike item rewards which carry a count.
    // Preserve the common RewardItemInfo carrier used by QuestScript while
    // assigning a neutral count; the quest-clear packet is driven by reward
    // type 24 and does not insert these rows into inventory.
    while (true) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int stage = ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            return token == closing_tag;
        }
        if (!success) {
            return false;
        }
        RewardItemInfo reward;
        reward.item_id = stage;
        reward.item_type = oldsrv::config::kCharacterJobCommon;
        reward.period = -1;
        reward.item_count = 1;
        output.push_back(reward);
    }
}

}  // namespace

// IDA_EA: 0x8A61813
// IDA_NAME: _Z17importQuestScriptP11QuestScriptPKc
// DEMANGLED: importQuestScript(QuestScript *,char const*)
int importQuestScript(QuestScript* script, const char* filename) {
    // These two function-local values deliberately retain their old local
    // class identity.  Their C2/D2 symbols are part of the old df_game_r
    // import closure and their data members reproduce 0x8A616FE/0x8A617A6.
    struct QuestData {
        std::string condition_message;                 // +0x00
        std::vector<int> int_data;                     // +0x04
        int level_min;                                 // +0x10
        int level_max;                                 // +0x14
        bool exposed_by_npc;                           // +0x18
        uint8_t reserved_19[3];                        // +0x19
        bool first_exposed_by_npc;                     // +0x1c
        uint8_t reserved_1d[3];                        // +0x1d, uninitialised
        std::vector<MonsterKillItem> monster_items;    // +0x20
        bool has_monster_items;                        // +0x2c
        uint8_t reserved_2d[3];                        // +0x2d, uninitialised

        // IDA_EA: 0x8A616FE
        // IDA_NAME: _ZZ17importQuestScriptP11QuestScriptPKcEN9QuestDataC2Ev
        // DEMANGLED: importQuestScript(QuestScript *,char const*)::QuestData::QuestData(void)
        __attribute__((noinline)) QuestData()
            : condition_message(),
              int_data(),
              level_min(0),
              level_max(0),
              exposed_by_npc(false),
              reserved_19(),
              first_exposed_by_npc(false),
              monster_items(),
              has_monster_items(false) {}

        // IDA_EA: 0x8A617A6
        // IDA_NAME: _ZZ17importQuestScriptP11QuestScriptPKcEN9QuestDataD2Ev
        // DEMANGLED: importQuestScript(QuestScript *,char const*)::QuestData::~QuestData(void)
        __attribute__((noinline)) ~QuestData() {}
    };

    QuestData light_data;
    QuestData first_data;
    static_assert(sizeof(QuestData) == 0x30u,
                  "local QuestData must retain the old 0x30-byte ABI");

    if (!loadRDARScriptFile(g_questScriptBaseDirectory, filename)) {
        return 0;
    }

    script->clear();
    bool has_light_condition_message = false;
    bool has_light_int_data = false;
    bool has_light_level = false;
    bool has_light_exposed_by_npc = false;
    bool has_light_monster_items = false;

    std::string tag;
    std::string value;
    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            if (script->job_message_enabled != 0) {
                ScanJobStr(&script->name, script->job_names);
            } else {
                ScanStr(&script->name);
            }
        } else if (tag == "[depend message]") {
            if (script->job_message_enabled != 0) {
                ScanJobStr(&script->request_message,
                           script->job_request_messages);
            } else {
                ScanStr(&script->request_message);
            }
        } else if (tag == "[light condition message]") {
            ScanStr(&light_data.condition_message);
            has_light_condition_message = true;
        } else if (tag == "[first condition message]") {
            ScanStr(&first_data.condition_message);
        } else if (tag == "[condition message]") {
            if (script->job_message_enabled != 0) {
                ScanJobStr(&script->condition_message,
                           script->job_condition_messages);
            } else {
                ScanStr(&script->condition_message);
            }
            if (script->quest_grade == 7) {
                script->name = script->condition_message;
                std::remove(script->name.begin(), script->name.end(), '\'');
                std::remove(script->name.begin(), script->name.end(), '`');
                script->name.resize(0x1d);
            }
        } else if (tag == "[solve message]") {
            if (script->job_message_enabled != 0) {
                ScanJobStr(&script->solve_message, script->job_solve_messages);
            } else {
                ScanStr(&script->solve_message);
            }
        } else if (tag == "[grade]") {
            ScanStr(&value);
            if (value == "[normaly repeat]") {
                script->quest_grade = 4;
            } else if (value == "[common unique]") {
                script->quest_grade = 5;
            } else if (value == "[epic]") {
                script->quest_grade = 0;
            } else if (value == "[achievement]") {
                script->quest_grade = 2;
            } else if (value == "[training]") {
                script->quest_grade = 1;
            } else if (value == "[special]") {
                script->quest_grade = 6;
            } else if (value == "[urgent]") {
                script->quest_grade = 8;
            } else if (value == "[daily random]") {
                // CLIENT_EA 0x16296d4: grade 3 with daily subtype 0.
                script->quest_grade = 3;
                script->value_0e0 = 0;
            } else if (value == "[daily]") {
                script->quest_grade = 3;
                script->value_0e0 = 0;
            } else if (value == "[special daily]") {
                script->quest_grade = 3;
                ScanStr(&value);
                if (value == "[village attack revenge]") {
                    script->value_0e0 = 1;
                } else if (value == "[schedule]") {
                    script->value_0e0 = 2;
                } else {
                    script->value_0e0 = 0;
                }
            } else if (value == "[title]") {
                script->quest_grade = 7;
            }
        } else if (tag == "[schedule]") {
            if (!ParseQuestSchedule(*script, filename)) {
                return 0;
            }
        } else if (tag == "[job change quest]") {
            script->value_0dc = ScanInt(static_cast<bool*>(0));
            if (script->value_0dc == 1 && script->value_0f0 != -1) {
                std::printf("questScript->classChangeQuestType_(%d) > 0  and "
                            "questScript->growType_(%d) != -1",
                            script->value_0dc,
                            script->value_0f0);
                return 0;
            }
        } else if (tag == "[npc index]") {
            script->value_0e4 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[complete npc index]") {
            script->value_0e8 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[job]") {
            ScanStr(&value);
            script->value_0ec = getCharacterJob(value.c_str());
        } else if (tag == "[grow type]") {
            script->value_0f0 = ScanInt(static_cast<bool*>(0));
            if (script->value_0dc == 1 && script->value_0f0 != -1) {
                std::printf("questScript->classChangeQuestType_(%d) > 0  and "
                            "questScript->growType_(%d) != -1",
                            script->value_0dc,
                            script->value_0f0);
                return 0;
            }
        } else if (tag == "[awakening type]") {
            script->value_0e0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[target character]") {
            while (ScanStr(&value) == 1) {
                if (checkValiedJobName(value.c_str()) != 1) {
                    return 0;
                }
                int grow_type = 0;
                int awakening_type = 0;
                if (!ScanRequiredInt(grow_type) || !ScanRequiredInt(awakening_type)) {
                    return 0;
                }
                script->target_characters.push_back(stQuestTargetCharacter_t(
                    getCharacterJob(value.c_str()), grow_type, awakening_type));
            }
        } else if (tag == "[reward one more check]") {
            ScanStr(&script->value_2e8);
        } else if (tag == "[quest window extension]") {
            bool success = false;
            script->value_2ec = ScanInt(&success) > 0;
        } else if (tag == "[light level]") {
            light_data.level_min = ScanInt(static_cast<bool*>(0));
            light_data.level_max = ScanInt(static_cast<bool*>(0));
            has_light_level = true;
        } else if (tag == "[level]") {
            script->value_100 = ScanInt(static_cast<bool*>(0));
            script->value_104 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ignore quest level 4 exp]") {
            script->value_108 = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[pre required quest]") {
            std::vector<int> condition;
            while (true) {
                int quest = 0;
                if (!ScanRequiredInt(quest)) {
                    break;
                }
                condition.push_back(quest);
            }
            if (!condition.empty()) {
                script->conditions.push_back(condition);
            }
        } else if (tag == "[collision quest]") {
            script->condition_values.clear();
            while (true) {
                int quest = 0;
                if (!ScanRequiredInt(quest)) {
                    break;
                }
                script->condition_values.push_back(quest);
            }
        } else if (tag == "[PVP RANK]") {
            script->value_124 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature kind]") {
            script->value_128 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature level]") {
            script->value_12c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[effect after clear quest]") {
            script->value_130 = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[delete npc index]") {
            script->value_134 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[clear reward item]") {
            script->dungeon_clear_items.clear();
            while (true) {
                DungeonClearItem item;
                bool success = false;
                item.reward_type = ScanInt(&success);
                if (!success) {
                    break;
                }
                item.dungeon_index = ScanInt(static_cast<bool*>(0));
                item.item_id = ScanInt(static_cast<bool*>(0));
                item.item_count = ScanInt(static_cast<bool*>(0));
                item.difficulty = ScanInt(static_cast<bool*>(0));
                item.raw_14 = ScanInt(static_cast<bool*>(0));
                script->dungeon_clear_items.push_back(item);
            }
        } else if (tag == "[light monster reward item]" ||
                   tag == "[light enemy reward item]") {
            ParseMonsterRewardItems(light_data.monster_items,
                                    tag == "[light enemy reward item]");
            light_data.has_monster_items = true;
            has_light_monster_items = true;
        } else if (tag == "[monster reward item]" ||
                   tag == "[enemy reward item]") {
            ParseMonsterRewardItems(script->monster_kill_items,
                                    tag == "[enemy reward item]");
        } else if (tag == "[appear map]") {
            script->appear_map.map_id = ScanInt(static_cast<bool*>(0));
            script->appear_map.x = ScanInt(static_cast<bool*>(0));
            script->appear_map.y = ScanInt(static_cast<bool*>(0));
            script->appear_map.area_id = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[depend give item]") {
            script->depend_give_items.clear();
            while (true) {
                int item_id = 0;
                if (!ScanRequiredInt(item_id)) {
                    break;
                }
                script->depend_give_items.push_back(std::make_pair(
                    item_id, ScanInt(static_cast<bool*>(0))));
            }
        } else if (tag == "[limit dungeon index]") {
            while (true) {
                int dungeon_index = 0;
                if (!ScanRequiredInt(dungeon_index)) {
                    break;
                }
                script->value_1c8.push_back(dungeon_index);
            }
        } else if (tag == "[type]") {
            if (ScanStr(&value) != 1) {
                return 0;
            }
            const int type = QuestConditionType(value);
            if (type < 0) {
                return 0;
            }
            script->value_178 = type;
        } else if (tag == "[dungeon info]") {
            while (true) {
                AdditionalDungeonInfo info;
                if (!ScanRequiredInt(info.dungeon_index)) {
                    break;
                }
                info.value = ScanInt(static_cast<bool*>(0));
                script->additional_dungeon_info.push_back(info);
            }
        } else if (tag == "[sub type]") {
            script->value_17c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[light int data]") {
            light_data.int_data.clear();
            while (true) {
                int data = 0;
                if (!ScanRequiredInt(data)) {
                    break;
                }
                light_data.int_data.push_back(data);
            }
            has_light_int_data = true;
        } else if (tag == "[first int data]") {
            first_data.int_data.clear();
            while (true) {
                int data = 0;
                if (!ScanRequiredInt(data)) {
                    break;
                }
                first_data.int_data.push_back(data);
            }
        } else if (tag == "[int data]") {
            script->limit_dungeon_indices.clear();
            while (true) {
                int data = 0;
                if (!ScanRequiredInt(data)) {
                    break;
                }
                script->limit_dungeon_indices.push_back(data);
            }
        } else if (tag == "[string data]") {
            script->value_18c.clear();
            std::string data;
            while (ScanStr(&data) == 1) {
                script->value_18c.push_back(data);
            }
        } else if (tag == "[send postal]") {
            script->postal_reward.first_dword =
                static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
            script->postal_reward.second_dword =
                static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
            ScanStr(&script->postal_reward.first_string);
            ScanStr(&script->postal_reward.second_string);
        } else if (tag == "[condition data]") {
            script->value_198.clear();
            while (true) {
                int index = 0;
                if (!ScanRequiredInt(index)) {
                    break;
                }
                std::string text;
                ScanStr(&text);
                script->value_198[index] = std::make_pair(text, std::string());
            }
        } else if (tag == "[check count]") {
            script->value_1b0.clear();
            while (true) {
                int count = 0;
                if (!ScanRequiredInt(count)) {
                    break;
                }
                script->value_1b0.push_back(count);
            }
        } else if (tag == "[limit showing msg]") {
            script->value_1c0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[reward type]") {
            ScanStr(&value);
            if (value == "[item]") {
                script->value_1d4 = 0;
            } else if (value == "[grow type]") {
                script->value_1d4 = 1;
            } else if (value == "[awakening type]") {
                script->value_1d4 = 2;
            } else if (value == "[hell challenge]") {
                script->value_1d4 = 3;
            } else if (value == "[creature evolution]") {
                script->value_1d4 = 10;
            } else if (value == "[expert job]") {
                script->value_1d4 = 20;
            } else if (value == "[hero]") {
                script->value_1d4 = 21;
            } else if (value == "[slot expansion]") {
                script->value_1d4 = 22;
            } else if (value == "[ridable]") {
                script->value_1d4 = 23;
            } else if (value == "[title]") {
                // CLIENT_EA 0x162b883: the 0828 title reward enum is 26.
                script->value_1d4 = 26;
            } else if (value == "[event skill]") {
                // CLIENT_EA 0x162b8a7: event-skill reward enum is 24.
                script->value_1d4 = 24;
            } else if (value == "[event creature evolution]") {
                script->value_1d4 = 25;
            }
        } else if (tag == "[reward selection int data]") {
            script->light_reward_items.clear();
            if (script->value_1d4 == 0 || script->value_1d4 == 25) {
                if (!ScanRewardItems(script->light_reward_items,
                                     "[/reward selection int data]")) {
                    return 0;
                }
            } else {
                ScanType(value, true);
            }
        } else if (tag == "[dialog]") {
            ParseQuestDialog(*script);
        } else if (tag == "[reward int data]") {
            script->reward_items.clear();
            if (script->value_1d4 == 24) {
                if (!ScanEventSkillRewards(script->reward_items,
                                           "[/reward int data]")) {
                    return 0;
                }
            } else if (script->value_1d4 == 0 || script->value_1d4 == 25) {
                if (!ScanRewardItems(script->reward_items,
                                     "[/reward int data]")) {
                    return 0;
                }
            } else {
                script->value_1e4 = ScanInt(static_cast<bool*>(0));
                ScanType(value, true);
            }
        } else if (tag == "[collision reward int data]") {
            script->clear_reward_items.clear();
            while (true) {
                int value_int = 0;
                if (!ScanRequiredInt(value_int)) {
                    return 0;
                }
                if (value_int == -1) {
                    break;
                }
                script->clear_reward_items.push_back(value_int);
            }
            if (ScanStr(&value) != 1) {
                return 0;
            }
            if (value == "[and]") {
                script->value_218 = 0;
            } else if (value == "[or]") {
                script->value_218 = 1;
            } else {
                return 0;
            }
            script->relation_quest_indices.clear();
            while (true) {
                int quest = 0;
                if (!ScanRequiredInt(quest)) {
                    break;
                }
                script->relation_quest_indices.push_back(quest);
            }
        } else if (tag == "[relation quest]") {
            script->value_228 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[light exposed by npc]") {
            bool success = false;
            light_data.exposed_by_npc = ScanInt(&success) == 1;
            if (!success) {
                return 0;
            }
            has_light_exposed_by_npc = true;
        } else if (tag == "[first exposed by npc]") {
            bool success = false;
            first_data.first_exposed_by_npc = ScanInt(&success) == 1;
            if (!success) {
                return 0;
            }
        } else if (tag == "[exposed by npc]") {
            bool success = false;
            script->exposed_by_npc = ScanInt(&success) == 1;
            if (!success) {
                return 0;
            }
        } else if (tag == "[event]") {
            bool success = false;
            script->event = ScanInt(&success) == 1;
            if (!success) {
                return 0;
            }
        } else if (tag == "[auto display]") {
            for (int index = 0; index != 3; ++index) {
                if (!ScanRequiredInt(script->auto_display[index])) {
                    return 0;
                }
            }
            script->has_substitutive_names = true;
        } else if (tag == "[substitutive names]") {
            while (true) {
                bool is_type = false;
                if (ScanTypeOrStr(is_type, value, true) != 1) {
                    OutputErrorMessage(filename);
                    break;
                }
                if (is_type) {
                    break;
                }
                script->substitutive_names.push_back(value);
            }
            if (value != "[/substitutive names]") {
                OutputErrorMessage(filename);
            }
        } else if (tag == "[difficulty]") {
            ScanStr(&value);
            if (!value.empty()) {
                unsigned char difficulty = static_cast<unsigned char>(value[0]);
                if (std::islower(difficulty)) {
                    difficulty = static_cast<unsigned char>(std::toupper(difficulty));
                }
                script->difficulty = difficulty;
            }
        } else if (tag == "[gold multiple]") {
            bool success = false;
            script->gold_multiple = ScanInt(&success);
            if (!success) {
                break;
            }
        } else if (tag == "[mail time]") {
            bool success = false;
            script->mail_time = static_cast<int8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        } else if (tag == "[mail title]") {
            ScanStr(&script->mail_title);
        } else if (tag == "[mail contents]") {
            ScanStr(&script->mail_contents);
        } else if (tag == "[mail enclose]") {
            script->mail_enclose.clear();
            while (true) {
                int item = 0;
                if (!ScanRequiredInt(item)) {
                    break;
                }
                script->mail_enclose.push_back(item);
            }
        } else if (tag == "[cant giveup]") {
            bool success = false;
            script->cant_giveup = ScanInt(&success) != 0;
            if (!success) {
                break;
            }
        } else if (tag == "[searching information]") {
            bool success = false;
            script->searching_information = ScanInt(&success) != 0;
            if (!success) {
                break;
            }
        } else if (tag == "[grow number]") {
            bool success = false;
            script->grow_number = ScanInt(&success);
            if (!success) {
                break;
            }
        } else if (tag == "[belong to winning power]") {
            bool success = false;
            script->belong_to_winning_power =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        } else if (tag == "[online]") {
            bool success = false;
            script->online = ScanInt(&success);
            if (!success) {
                break;
            }
        } else if (tag == "[job message]") {
            script->job_message_enabled = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[quest point]") {
            int quest_point = 0;
            if (!ScanRequiredInt(quest_point)) {
                return 0;
            }
            script->quest_point = static_cast<uint16_t>(quest_point);
        } else if (tag == "[quest piece]") {
            int quest_piece = 0;
            if (!ScanRequiredInt(quest_piece)) {
                return 0;
            }
            script->quest_piece = static_cast<uint16_t>(quest_piece);
        } else if (tag == "[go guide]") {
            for (int index = 0; index != 4; ++index) {
                script->go_guide[index] = ScanInt(static_cast<bool*>(0));
            }
            ScanStr(&script->go_guide_message);
        } else if (tag == "[expertjob level]") {
            script->expert_job_level = ScanInt(static_cast<bool*>(0));
            script->expert_job_level_max = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[change dungeon info message]") {
            script->has_change_dungeon_info_message = true;
            ScanStr(&script->change_dungeon_info_message);
        } else if (tag == "[exception quest]") {
            if (!ParseQuestException(*script)) {
                return 0;
            }
        } else if (tag == "[isNotClear LevelUp System]") {
            int system_id = 0;
            if (!ScanRequiredInt(system_id)) {
                return 0;
            }
            script->is_not_clear_levelup_system.push_back(system_id);
        } else if (tag == "[hide npc on clear]") {
            int npc_index = 0;
            if (!ScanRequiredInt(npc_index)) {
                return 0;
            }
            script->hide_npc_on_clear.push_back(npc_index);
        } else if (tag == "[show npc on clear]") {
            int enabled = 0;
            if (!ScanRequiredInt(enabled)) {
                return 0;
            }
            script->show_npc_on_clear = enabled != 0;
        } else if (tag == "[show click on complete npc]") {
            int enabled = 0;
            if (!ScanRequiredInt(enabled)) {
                return 0;
            }
            // The 0828 client stores tag presence as true.  Consume the
            // compatibility integer carried by official PVF, but do not use
            // it as the stored policy value.
            script->show_click_on_complete_npc = true;
        } else if (tag == "[no dungeon name]") {
            int enabled = 0;
            if (!ScanRequiredInt(enabled)) {
                return 0;
            }
            script->no_dungeon_name = enabled != 0;
        }
    }

    if (IsLightServer()) {
        if (has_light_exposed_by_npc) {
            script->exposed_by_npc = light_data.exposed_by_npc;
        }
        if (has_light_int_data) {
            script->limit_dungeon_indices = light_data.int_data;
        }
        if (has_light_level) {
            script->value_100 = light_data.level_min;
            script->value_104 = light_data.level_max;
        }
        if (has_light_condition_message) {
            script->condition_message = light_data.condition_message;
        }
        if (has_light_monster_items) {
            script->monster_kill_items = light_data.monster_items;
        }
    }

    script->quest_index = g_questScriptFileList.FindIndexByFullScan(filename);
    if (script->value_0e8 == -1) {
        script->value_0e8 = script->value_0e4;
    }
    if (script->target_characters.empty() ||
        script->value_0ec != oldsrv::config::kCharacterJobCommon ||
        script->value_0f0 != -1) {
        const stQuestTargetCharacter_t target(script->value_0ec,
                                              script->value_0f0,
                                              script->value_0e0);
        if (std::find(script->target_characters.begin(),
                      script->target_characters.end(),
                      target) == script->target_characters.end()) {
            script->target_characters.push_back(target);
        }
    }
    return 1;
}
