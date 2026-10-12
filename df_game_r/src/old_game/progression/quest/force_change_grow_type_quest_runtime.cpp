#include "old_game/progression/quest/force_change_grow_type_quest_runtime.h"

#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x86686ED
// IDA_NAME: _Z40_postCheckForceChangeGrowType4AheadQuestP5CUserP5Quest
// DEMANGLED: _postCheckForceChangeGrowType4AheadQuest(CUser *,Quest *)
char _postCheckForceChangeGrowType4AheadQuest(CUser* user, Quest* quest) {
    char result = static_cast<char>(quest->conditions.empty());
    if (!result) {
        for (std::vector<std::vector<int> >::const_iterator condition =
                 quest->conditions.begin();
             condition != quest->conditions.end(); ++condition) {
            for (std::vector<int>::const_iterator quest_index =
                     condition->begin();
                 quest_index != condition->end(); ++quest_index) {
                Quest* const ahead_quest =
                    G_CDataManager()->find_quest(*quest_index);
                if (ahead_quest) {
                    _postCheckForceChangeGrowType4AheadQuest(user,
                                                              ahead_quest);
                    user->getCurCharacQuestW()->clear_quest.setClearedQuest(
                        static_cast<unsigned int>(*quest_index));
                }
            }
        }
        result = 0;
    }
    return result;
}

// IDA_EA: 0x8668823
// IDA_NAME: _Z29_postCheckForceChangeGrowTypeP5CUserii
// DEMANGLED: _postCheckForceChangeGrowType(CUser *,int,int)
void _postCheckForceChangeGrowType(CUser* user,
                                   int first_grow_type,
                                   int second_grow_type) {
    QuestList* const quest_list =
        G_CDataManager()->get_arad_jumping_quest_list();
    const __gnu_cxx::hash_map<int, Quest*>& quests =
        *quest_list->getQuestList();

    for (__gnu_cxx::hash_map<int, Quest*>::const_iterator it = quests.begin();
         it != quests.end(); ++it) {
        Quest* const quest = it->second;
        const ENUM_CHARACTERJOB character_job =
            static_cast<ENUM_CHARACTERJOB>(user->get_charac_job());
        if (static_cast<unsigned char>(quest->checkPossibleJob(character_job)) !=
            1u) {
            continue;
        }

        if (quest->reward_type == 1) {
            if (quest->value_c0 != first_grow_type) {
                continue;
            }
        } else if (quest->reward_type == 2) {
            const int current_first_grow_type =
                user->getCurCharFirstGrowType();
            const ENUM_CHARACTERJOB current_character_job =
                static_cast<ENUM_CHARACTERJOB>(user->get_charac_job());
            if (static_cast<unsigned char>(quest->checkMatchedGrowType(
                    current_character_job, current_first_grow_type, -1)) != 1u ||
                quest->value_c0 != second_grow_type) {
                continue;
            }
        } else {
            continue;
        }

        _postCheckForceChangeGrowType4AheadQuest(user, quest);
        user->getCurCharacQuestW()->clear_quest.setClearedQuest(
            static_cast<unsigned int>(quest->quest_index));
    }

    PacketGuard packet;
    user->getCurCharacQuestR()->get_quest_info(
        reinterpret_cast<char*>(&packet));
    user->Send(packet);
}
