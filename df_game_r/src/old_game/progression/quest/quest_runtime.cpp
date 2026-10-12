#include "old_game/progression/quest/quest_runtime.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"

#include <algorithm>
#include <utility>

// IDA_EA: 0x8352203
// IDA_NAME: _Z18makeRewardItemInfoRKSt6vectorIiSaIiEERS_I14RewardItemInfoSaIS4_EE
// DEMANGLED: makeRewardItemInfo(std::vector<int> const&,std::vector<RewardItemInfo>&)
void makeRewardItemInfo(const std::vector<int>& values,
                        std::vector<RewardItemInfo>& rewards) {
    RewardItemInfo reward;
    std::vector<int>::const_iterator it = values.begin();
    while (it != values.end()) {
        reward.item_id = *it;
        ++it;
        if (it == values.end()) {
            cMyTrace trace(
                "void makeRewardItemInfo(const INT_VECTOR&, RewardItemInfoVector&)",
                4173,
                5);
            trace("[%s][%d]_it == data.rewardSelectItems_.end()",
                  "void makeRewardItemInfo(const INT_VECTOR&, RewardItemInfoVector&)");
            return;
        }
        reward.item_count = *it;
        ++it;
        rewards.push_back(reward);
    }
}

// IDA_EA: 0x837AA28
// IDA_NAME: _ZN5QuestC2Ev
// DEMANGLED: Quest::Quest(void)
Quest::Quest()
    : name(),
      target_characters(),
      conditions(),
      condition_values(),
      dungeon_clear_items(),
      monster_kill_items(),
      appear_map(),
      depend_give_items(),
      clear_condition_values(),
      achievement_trigger_values(),
      postal_reward(),
      reward_select_pairs(),
      reward_items(),
      light_reward_items(),
      clear_reward_items(),
      reward_select_items(),
      mail_title(),
      mail_contents(),
      mail_enclose(),
      dungeon_event_dialogs(),
      exception_quest_types() {}

// IDA_EA: 0x837A6E0
// IDA_NAME: _ZN5QuestD2Ev
// DEMANGLED: Quest::~Quest()
Quest::~Quest() {}

// IDA_EA: 0x835233E
// IDA_NAME: _ZN5Quest9set_questE11QuestScript
// DEMANGLED: Quest::set_quest(QuestScript)
void Quest::set_quest(QuestScript script) {
    name = script.name;
    quest_index = script.quest_index;
    quest_grade = script.quest_grade;
    value_0c = script.value_0e4;
    target_characters = script.target_characters;
    check_grow_type_mode = script.value_0dc;
    level_limit_value = script.value_124;
    minimum_level = script.value_100;
    maximum_level = script.value_104;
    conditions = script.conditions;
    condition_values = script.condition_values;
    clear_send_enabled = script.value_130;
    dungeon_clear_items = script.dungeon_clear_items;
    monster_kill_items = script.monster_kill_items;
    appear_map = script.appear_map;
    depend_give_items = script.depend_give_items;
    trigger_type = script.value_178;
    trigger_subtype = script.value_17c;
    clear_condition_values = script.limit_dungeon_indices;
    achievement_trigger_values = script.value_1b0;
    postal_reward = script.postal_reward;
    event = script.event;
    difficulty = script.difficulty;
    value_130 = script.value_108;

    if (trigger_type) {
        if (trigger_type == 25) {
            reward_select_pairs.clear();
            const unsigned int pair_count =
                static_cast<unsigned int>(clear_condition_values.size()) >> 1;
            std::vector<int>::iterator it = clear_condition_values.begin();
            for (unsigned int index = 0; index < pair_count; ++index) {
                if (it == clear_condition_values.end()) {
                    break;
                }
                const int first = *it;
                ++it;
                const int second = *it;
                ++it;
                reward_select_pairs.push_back(std::make_pair(first, second));
            }
        }
    } else {
        std::vector<int>::iterator it = clear_condition_values.begin();
        while (it != clear_condition_values.end()) {
            const int first = *it;
            ++it;
            if (it == clear_condition_values.end()) {
                cMyTrace trace("void Quest::set_quest(QuestScript)", 4253, 5);
                trace("[%s][%d]_(it == intDatas_.end())",
                      "void Quest::set_quest(QuestScript)");
                break;
            }
            const int second = *it;
            ++it;
            reward_select_pairs.push_back(std::make_pair(first, second));
        }
    }

    clear_reward_items.clear();
    value_e8 = 0;
    reward_select_items.clear();
    reward_type = script.value_1d4;
    value_c0 = script.value_1e4;
    value_bc = script.value_228;
    if (!reward_type || reward_type == 24 || reward_type == 25) {
        reward_items = script.reward_items;
        light_reward_items = script.light_reward_items;
        clear_reward_items = script.clear_reward_items;
        value_e8 = script.value_218;
        makeRewardItemInfo(script.relation_quest_indices, reward_select_items);
    }

    value_f8 = script.value_128;
    value_fc = script.value_12c;
    exposed_by_npc = script.exposed_by_npc;
    gold_multiple = script.gold_multiple;
    mail_time = script.mail_time;
    mail_title = script.mail_title;
    mail_contents = script.mail_contents;
    mail_enclose = script.mail_enclose;
    cant_giveup = script.cant_giveup;
    dungeon_event_dialogs = script.dungeon_event_dialogs;
    belong_to_winning_power = script.belong_to_winning_power;
    quest_type = script.quest_type;
    quest_point = static_cast<int16_t>(script.quest_point);
    quest_piece = static_cast<int16_t>(script.quest_piece);
    expert_job_level = script.expert_job_level;
    expert_job_level_max = script.expert_job_level_max;
    exception_quest_types = script.exception_quest_types;
}

// IDA_EA: 0x822B5E2
// IDA_NAME: _ZNK5Quest11exposeQuestEv
// DEMANGLED: Quest::exposeQuest(void)const
int Quest::exposeQuest() const {
    return static_cast<unsigned char>(exposed_by_npc);
}

// IDA_EA: 0x8352A5C
// IDA_NAME: _ZNK5Quest16checkPossibleJobE17ENUM_CHARACTERJOB
// DEMANGLED: Quest::checkPossibleJob(ENUM_CHARACTERJOB)const
int Quest::checkPossibleJob(ENUM_CHARACTERJOB charac_job) const {
    for (std::vector<stQuestTargetCharacter_t>::const_iterator it =
             target_characters.begin();
         it != target_characters.end();
         ++it) {
        if (it->character_job == static_cast<int>(charac_job) ||
            it->character_job == oldsrv::config::kCharacterJobCommon) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8352AFC
// IDA_NAME: _ZNK5Quest20checkMatchedGrowTypeE17ENUM_CHARACTERJOBii
// DEMANGLED: Quest::checkMatchedGrowType(ENUM_CHARACTERJOB,int,int)const
int Quest::checkMatchedGrowType(ENUM_CHARACTERJOB charac_job,
                                int first_grow_type,
                                int second_grow_type) const {
    for (std::vector<stQuestTargetCharacter_t>::const_iterator it =
             target_characters.begin();
         it != target_characters.end();
         ++it) {
        if ((it->character_job == static_cast<int>(charac_job) ||
             it->character_job == oldsrv::config::kCharacterJobCommon) &&
            (it->grow_type == first_grow_type || it->grow_type == -1) &&
            (it->level == second_grow_type || it->level == -1)) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8352BEE
// IDA_NAME: _ZNK5Quest17isRepeatableQuestEv
// DEMANGLED: Quest::isRepeatableQuest(void)const
bool Quest::isRepeatableQuest() const {
    return quest_grade == 4 || quest_grade == 8;
}

// IDA_EA: 0x8352C10
// IDA_NAME: _ZNK5Quest22_isNeededCheckGrowTypeE17ENUM_CHARACTERJOB
// DEMANGLED: Quest::_isNeededCheckGrowType(ENUM_CHARACTERJOB)const
int Quest::_isNeededCheckGrowType(ENUM_CHARACTERJOB charac_job) const {
    for (std::vector<stQuestTargetCharacter_t>::const_iterator it =
             target_characters.begin();
         it != target_characters.end();
         ++it) {
        if ((it->character_job == static_cast<int>(charac_job) ||
             it->character_job == oldsrv::config::kCharacterJobCommon) &&
            it->grow_type == -1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8352D86
// IDA_NAME: _ZNK5Quest14check_possibleERK18stSelectQuestParam
// DEMANGLED: Quest::check_possible(stSelectQuestParam const&)const
bool Quest::check_possible(const stSelectQuestParam& param) const {
    const ENUM_CHARACTERJOB charac_job =
        static_cast<ENUM_CHARACTERJOB>(param.charac_job);
    if (checkPossibleJob(charac_job) != 1) {
        return false;
    }
    if (_isNeededCheckGrowType(charac_job)) {
        if (check_grow_type_mode == 1) {
            LogManager::logFormat(
                1,
                "data_manager.cpp",
                "bool Quest::check_possible(const stSelectQuestParam&) const",
                0x1193,
                "Quest Script Error Break !, [grow type] != -1 and [job change quest] = 1");
            return false;
        }
        if (check_grow_type_mode == 2) {
            if (checkMatchedGrowType(charac_job, param.first_grow_type, -1) !=
                1) {
                return false;
            }
        } else if (check_grow_type_mode != 10 &&
                   check_grow_type_mode != 20 &&
                   checkMatchedGrowType(charac_job,
                                        param.first_grow_type,
                                        -1) != 1) {
            return false;
        }
    }
    if (minimum_level > param.charac_level) {
        return false;
    }
    if (maximum_level < param.charac_level) {
        return false;
    }
    if (level_limit_value && level_limit_value > param.pvp_grade) {
        return false;
    }
    if (quest_grade == 1 &&
        G_CDataManager()->isThereDailyTrainingQuestList(
            param.charac_level, quest_index) != 1) {
        return false;
    }
    if (!expert_job_level || expert_job_level_max == -1) {
        return true;
    }
    if (expert_job_level == -1 ||
        expert_job_level == param.expert_job_type) {
        return expert_job_level_max <= param.expert_job_level;
    }
    return false;
}

// IDA_EA: 0x8352FB4
// IDA_NAME: _ZNK5Quest28get_init_achievement_triggerERtS0_S0_
// DEMANGLED: Quest::get_init_achievement_trigger(unsigned short &,unsigned short &,unsigned short &)const
int Quest::get_init_achievement_trigger(unsigned short& first,
                                        unsigned short& second,
                                        unsigned short& third) const {
    if (quest_grade != 7) {
        return 0;
    }

    first = 1;
    second = 0;
    third = 0;
    const unsigned int count = std::min<unsigned int>(
        3u, static_cast<unsigned int>(achievement_trigger_values.size()));
    unsigned short* output[3] = {&first, &second, &third};
    for (unsigned int index = 0; index < count; ++index) {
        *output[index] =
            static_cast<unsigned short>(achievement_trigger_values[index]);
    }
    return 1;
}

// IDA_EA: 0x8353082
// IDA_NAME: _ZNK5Quest16get_title_rewardEv
// DEMANGLED: Quest::get_title_reward(void)const
int Quest::get_title_reward() const {
    if (quest_grade != 7) {
        return 0;
    }
    return reward_items.empty() ? 0 : reward_items[0].item_id;
}

// IDA_EA: 0x83530D6
// IDA_NAME: _ZNK5Quest16get_init_triggerEv
// DEMANGLED: Quest::get_init_trigger(void)const
int Quest::get_init_trigger() const {
    if (trigger_type == 2 || trigger_type == 6) {
        const bool extended_hunt_monster =
            trigger_type == 2 && !clear_condition_values.empty() &&
            clear_condition_values.front() == -2 &&
            clear_condition_values.size() % 5u == 0u;
        int16_t trigger_values[3] = {0, 0, 0};
        unsigned int trigger_index = 0;
        std::vector<int>::const_iterator it = clear_condition_values.begin();
        while (it != clear_condition_values.end()) {
            const int ignored_0 = *it;
            ++it;
            const int ignored_1 = *it;
            ++it;
            const int ignored_2 = *it;
            ++it;
            if (trigger_type == 6 || extended_hunt_monster) {
                const int ignored_3 = *it;
                ++it;
                (void)ignored_3;
            }
            const int trigger = *it;
            ++it;
            if (trigger_index < 3u) {
                trigger_values[trigger_index] =
                    static_cast<int16_t>(trigger);
            }
            ++trigger_index;
            (void)ignored_0;
            (void)ignored_1;
            (void)ignored_2;
        }
        stQuestTriggerState_t state;
        state.setTrigger(trigger_values[0],
                         trigger_values[1],
                         trigger_values[2]);
        return state.getTrigger();
    }
    if (trigger_type == 25) {
        stQuestTriggerState_t state;
        state.setTrigger(1, 1, 0);
        return state.getTrigger();
    }
    if (trigger_type == 1 && trigger_subtype == 6 &&
        clear_condition_values.size() == 3u) {
        const int trigger = clear_condition_values.at(2);
        if (trigger > 0) {
            return trigger;
        }
    }
    return 1;
}

// IDA_EA: 0x8353342
// IDA_NAME: _ZNK5Quest16check_clear_itemEii
// DEMANGLED: Quest::check_clear_item(int,int)const
int Quest::check_clear_item(int dungeon_index, int difficulty) const {
    for (std::vector<DungeonClearItem>::const_iterator it =
             dungeon_clear_items.begin();
         it != dungeon_clear_items.end();
         ++it) {
        if (it->dungeon_index == -1) {
            if (it->reward_type == dungeon_index) {
                return 1;
            }
        } else if (it->reward_type == dungeon_index &&
                   it->dungeon_index == difficulty) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8353404
// IDA_NAME: _ZNK5Quest18get_clear_itemlistEi
// DEMANGLED: Quest::get_clear_itemlist(int)const
std::vector<std::pair<int, int> > Quest::get_clear_itemlist(
    int dungeon_index) const {
    std::vector<std::pair<int, int> > result;
    for (std::vector<DungeonClearItem>::const_iterator it =
             dungeon_clear_items.begin();
         it != dungeon_clear_items.end();
         ++it) {
        const DungeonClearItem& clear_item = *it;
        if (clear_item.reward_type != dungeon_index) {
            continue;
        }
        CItem* const item = G_CDataManager()->find_item(clear_item.item_id);
        if (!item) {
            LogManager::logFormat(
                1,
                "data_manager.cpp",
                "IntPairVector Quest::get_clear_itemlist(int) const",
                0x1279,
                "G_CDataManager()->find_item(%d)",
                clear_item.item_id);
            continue;
        }
        if (item->is_stackable()) {
            int item_count = 0;
            for (int index = 0; clear_item.item_count > index; ++index) {
                if (clear_item.difficulty >=
                    static_cast<int>(get_rand_int(100))) {
                    ++item_count;
                }
            }
            if (item_count > 0) {
                result.push_back(std::make_pair(clear_item.item_id,
                                                item_count));
            }
        } else if (clear_item.difficulty >=
                   static_cast<int>(get_rand_int(100))) {
            result.push_back(std::make_pair(clear_item.item_id, 1));
        }
    }
    return result;
}

// IDA_EA: 0x83535D6
// IDA_NAME: _ZNK5Quest16CheckKillMonsterEiii21ENUM_QUEST_ENEMY_TYPERSt6vectorI15MonsterKillItemSaIS2_EE
// DEMANGLED: Quest::CheckKillMonster(int,int,int,ENUM_QUEST_ENEMY_TYPE,std::vector<MonsterKillItem>&)const
bool Quest::CheckKillMonster(int dungeon_index,
                             int dungeon_difficulty,
                             int monster_index,
                             ENUM_QUEST_ENEMY_TYPE enemy_type,
                             std::vector<MonsterKillItem>& matched) const {
    if (monster_kill_items.empty()) {
        return false;
    }
    matched.reserve(monster_kill_items.size());
    for (std::vector<MonsterKillItem>::const_iterator it =
             monster_kill_items.begin();
         it != monster_kill_items.end();
         ++it) {
        if ((it->current_count == -1 ||
             it->current_count == dungeon_difficulty) &&
            (it->required_count == -1 ||
             it->required_count == dungeon_index) &&
            it->target_type == static_cast<int>(enemy_type) &&
            it->monster_index == monster_index) {
            matched.push_back(*it);
        }
    }
    return !matched.empty();
}

// IDA_EA: 0x83536F2
// IDA_NAME: _ZNK5Quest13get_appearmapEii
// DEMANGLED: Quest::get_appearmap(int,int)const
int Quest::get_appearmap(int index, int compare_index) const {
    if (appear_map.x == -1) {
        if (appear_map.map_id == index &&
            appear_map.area_id >= static_cast<int>(get_rand_int(100))) {
            return appear_map.y;
        }
    } else if (appear_map.x == index && appear_map.x == compare_index &&
               appear_map.area_id >= static_cast<int>(get_rand_int(100))) {
        return appear_map.y;
    }
    return 0;
}

// IDA_EA: 0x8353776
// IDA_NAME: _ZNK5Quest19IsOpenScheduleQuestEv
// DEMANGLED: Quest::IsOpenScheduleQuest(void)const
bool Quest::IsOpenScheduleQuest() const {
    const int state = GlobalData::s_DailyScheduleManager->GetScheduleState(
        static_cast<_DailyScheduleType>(1),
        static_cast<unsigned int>(quest_index));
    return state == 1 || state == 2;
}

// IDA_EA: 0x83537C2
// IDA_NAME: _ZN5Quest24ConvertRewardSelectIndexEPK5CUseri
// DEMANGLED: Quest::ConvertRewardSelectIndex(CUser const*,int)
int Quest::ConvertRewardSelectIndex(const CUser* user, int select_index) {
    if (!user || select_index == -1 || select_index == 0xFFFF) {
        return -1;
    }
    int matched_index = 0;
    int index = 0;
    for (; index < static_cast<int>(light_reward_items.size()); ++index) {
        const RewardItemInfo& reward = light_reward_items[index];
        if (reward.item_type != oldsrv::config::kCharacterJobCommon) {
            if (reward.item_type != user->get_charac_job()) {
                continue;
            }
            if (reward.period != -1 &&
                reward.period != user->getCurCharFirstGrowType()) {
                continue;
            }
        }
        if (select_index == matched_index) {
            return static_cast<int>(index);
        }
        ++matched_index;
    }
    return static_cast<int>(index);
}

// IDA_EA: 0x84E923C
// IDA_NAME: _ZNK5Quest16check_clear_sendEv
// DEMANGLED: Quest::check_clear_send(void)const
int Quest::check_clear_send() const {
    return static_cast<unsigned char>(clear_send_enabled);
}
