#include "old_game/progression/quest/quest_state_runtime.h"

#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"

// IDA_EA: 0x86ABDB0
// IDA_NAME: _ZN9UserQuest15set_authen_dataE15QUEST_CONDITIONii
// DEMANGLED: UserQuest::set_authen_data(QUEST_CONDITION,int,int)
int UserQuest::set_authen_data(QUEST_CONDITION condition,
                               int value,
                               int auxiliary) {
    switch (static_cast<int>(condition)) {
    case 0:
        authen_data.quest_id = static_cast<unsigned short>(value);
        authen_data.clear_count = static_cast<unsigned short>(auxiliary);
        return static_cast<int>(reinterpret_cast<intptr_t>(this));
    case 1:
        authen_data.condition_value = static_cast<unsigned short>(value);
        return static_cast<int>(reinterpret_cast<intptr_t>(this));
    case 2:
        authen_data.auth_time = static_cast<unsigned int>(value);
        return static_cast<int>(reinterpret_cast<intptr_t>(this));
    case 3:
        authen_data.auth_flag = value != 0;
        return static_cast<int>(reinterpret_cast<intptr_t>(this));
    case 4:
        authen_data.auth_value_10 = static_cast<unsigned int>(value);
        return static_cast<int>(reinterpret_cast<intptr_t>(this));
    case 5:
        authen_data.auth_value_14 = static_cast<unsigned int>(value);
        return static_cast<int>(reinterpret_cast<intptr_t>(this));
    default:
        return static_cast<int>(condition);
    }
}

// IDA_EA: 0x86AC6AC
// IDA_NAME: _ZN9UserQuest11set_triggerEicb
// DEMANGLED: UserQuest::set_trigger(int,char,bool)
int UserQuest::set_trigger(int quest_index,
                           char trigger_state,
                           bool trigger_value) {
    int slot = 0;
    for (;; ++slot) {
        if (slot > 19) {
            return -1;
        }
        if (play_quest[slot] == static_cast<uint32_t>(quest_index)) {
            break;
        }
    }

    if (parent->getGmQuestFlag()) {
        play_quest_trigger[slot] = 0;
    }

    if (trigger_state == 1) {
        ++play_quest_trigger[slot];
    } else {
        if (check_quest_condition(quest_index) != 1) {
            return -1;
        }

        if (trigger_value) {
            stQuestTriggerState_t trigger;
            trigger = static_cast<int>(play_quest_trigger[slot]);
            trigger.incTirggerState(trigger_state);
            play_quest_trigger[slot] =
                static_cast<uint32_t>(trigger.getTrigger());
        } else if (static_cast<int>(play_quest_trigger[slot]) > 0) {
            stQuestTriggerState_t trigger;
            trigger = static_cast<int>(play_quest_trigger[slot]);
            trigger.subTriggerState(trigger_state);
            play_quest_trigger[slot] =
                static_cast<uint32_t>(trigger.getTrigger());
        }
    }

    return static_cast<int>(play_quest_trigger[slot]);
}

// IDA_EA: 0x86ACB52
// IDA_NAME: _ZN9UserQuest29check_quest_subtype_conditionEP5Quest
// DEMANGLED: UserQuest::check_quest_subtype_condition(Quest *)
int UserQuest::check_quest_subtype_condition(Quest* quest) {
    const std::vector<int>& values = quest->clear_condition_values;

    switch (quest->trigger_subtype) {
    case 0: {
        const int quest_id = values.at(0);
        const int clear_count = values.at(1);
        const int duration = values.at(2);
        if (clear_count != -1 &&
            static_cast<int>(authen_data.clear_count) != clear_count) {
            return 0;
        }
        if (static_cast<int>(authen_data.quest_id) != quest_id) {
            return 0;
        }
        const int elapsed_limit = static_cast<int>(
            static_cast<uint32_t>(duration) * 1000u);
        return elapsed_limit >= static_cast<int>(authen_data.auth_time);
    }
    case 1:
    case 2:
    case 3:
    case 6:
        return 1;
    case 4: {
        const int quest_id = values.at(0);
        const int clear_count = values.at(1);
        if (clear_count != -1 &&
            static_cast<int>(authen_data.clear_count) != clear_count) {
            return 0;
        }
        return static_cast<int>(authen_data.quest_id) == quest_id &&
               authen_data.auth_flag == 0;
    }
    case 5: {
        const int quest_id = values.at(0);
        const int minimum_clear_count = values.at(1);
        const int required_value = values.at(2);
        if (minimum_clear_count != -1 &&
            static_cast<int>(authen_data.clear_count) <
                minimum_clear_count) {
            return 0;
        }
        if (static_cast<int>(authen_data.quest_id) != quest_id) {
            return 0;
        }
        if (required_value == 1 &&
            static_cast<int>(authen_data.auth_value_10) != 1) {
            return 0;
        }
        return static_cast<int>(authen_data.auth_value_10) >= required_value;
    }
    case 7: {
        const int quest_id = values.at(0);
        const int clear_count = values.at(1);
        const int required_value = values.at(2);
        if (clear_count == -1 ||
            static_cast<int>(authen_data.clear_count) == clear_count) {
            if (static_cast<int>(authen_data.quest_id) == quest_id &&
                static_cast<int>(authen_data.auth_value_14) >=
                    required_value) {
                // The original jump returns through the default switch arm.
                return 0;
            }
        }
        return 0;
    }
    default:
        return 0;
    }
}

// IDA_EA: 0x86ACE46
// IDA_NAME: _ZN9UserQuest26check_quest_type_conditionEP5Quest
// DEMANGLED: UserQuest::check_quest_type_condition(Quest *)
int UserQuest::check_quest_type_condition(Quest* quest) {
    switch (quest->trigger_type) {
    case 0:
    case 1:
    case 2:
    case 4:
    case 6:
    case 15:
    case 25:
        return 1;
    case 3:
        return quest->clear_condition_values.at(0) ==
               static_cast<int>(authen_data.condition_value);
    case 5:
        return quest->clear_condition_values.at(0) <=
               parent->getPVPResultR()->values[5];
    case 7:
        return static_cast<char>(
                   GlobalData::s_power_manager->GetWinnerSide()) ==
               static_cast<char>(parent->getPowerSide());
    case 8:
        return quest->clear_condition_values.at(0) <=
               parent->GetUserPowerWarPoint();
    case 16:
    case 19:
        return 1;
    case 18: {
        // 0828 adds [get item check index] on top of the legacy [get item]
        // trigger number.  The new schema is identified by subtype -1 and
        // carries an any-of item-id list in [int data].  Keep the legacy
        // subtype 1/2 achievement path below, and authorize the new request
        // only from the server-owned inventory/equipment state.
        if (quest->trigger_subtype != -1) {
            return check_quest_subtype_condition(quest);
        }
        if (quest->clear_condition_values.empty()) {
            return 0;
        }

        CInventory* const inventory = parent->getCurCharacInvenR();
        if (!inventory) {
            return 0;
        }

        bool owns_candidate = false;
        for (std::vector<int>::const_iterator candidate =
                 quest->clear_condition_values.begin();
             candidate != quest->clear_condition_values.end();
             ++candidate) {
            if (*candidate <= 0 || !G_CDataManager()->find_item(*candidate)) {
                return 0;
            }
            int slot = 0;
            bool equipped = false;
            if (inventory->CheckExistItemInInventoryOrEquipment(
                    *candidate, slot, equipped) == 1) {
                owns_candidate = true;
            }
        }
        return owns_candidate ? 1 : 0;
    }
    default:
        return check_quest_subtype_condition(quest);
    }
}

// IDA_EA: 0x86ACF8A
// IDA_NAME: _ZN9UserQuest21check_quest_conditionEi
// DEMANGLED: UserQuest::check_quest_condition(int)
int UserQuest::check_quest_condition(int quest_index) {
    Quest* const quest =
        G_CDataManager()->get_arad_jumping_quest_list()->find_quest(
            quest_index);
    if (quest) {
        return check_quest_type_condition(quest);
    }

    LogManager::logFormat(1,
                          "user_quest.cpp",
                          "bool UserQuest::check_quest_condition(int)",
                          0x403,
                          "G_CDataManager()->m_pQuestList->find_quest(%d) fail",
                          quest_index);
    return 0;
}
