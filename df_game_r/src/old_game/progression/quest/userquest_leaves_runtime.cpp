#include "old_game/progression/quest/quest_state_runtime.h"

#include "common/config.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/timer/time_gate.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/interface_packet_buf.h"

#include <list>
#include <vector>

// IDA_EA: 0x86AD054
// IDA_NAME: _ZNK9UserQuest27CheckDungeonEventStoryPauseEiii
// DEMANGLED: UserQuest::CheckDungeonEventStoryPause(int,int,int)const
int UserQuest::CheckDungeonEventStoryPause(int playing_dungeon_index,
                                           int current_map_index,
                                           int maze_event_value) const {
    // The third ABI argument is passed by the dungeon dispatcher but is not
    // read by the original function body at 0x86AD054.
    (void)maze_event_value;

    for (int index = 0; index <= 19; ++index) {
        if (play_quest[index] == 0) {
            continue;
        }

        Quest* const quest = G_CDataManager()->find_quest(
            static_cast<int>(play_quest[index]));
        if (!quest) {
            continue;
        }

        const std::vector<dungeonEventHanaseStruct>& dialogs =
            quest->dungeon_event_dialogs;
        for (std::vector<dungeonEventHanaseStruct>::const_iterator dialog =
                 dialogs.begin();
             dialog != dialogs.end();
             ++dialog) {
            if (dialog->raw_00 != 0) {
                if (dialog->raw_00 == 1 &&
                    static_cast<int>(dialog->raw_08) == playing_dungeon_index) {
                    return 1;
                }
            } else if (static_cast<int>(dialog->raw_08) == current_map_index) {
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x86ABD7A
// IDA_NAME:
// _ZNK9UserQuest19get_mail_quest_infoERSt4listIiSaIiEERK18stSelectQuestParam
// DEMANGLED: UserQuest::get_mail_quest_info(
//             std::list<int>&,stSelectQuestParam const&)const
bool UserQuest::get_mail_quest_info(
    std::list<int>& selected_quests,
    const stSelectQuestParam& parameter) const {
    return G_CDataManager()->get_quest_list()->select_mail_quest(
        selected_quests, clear_quest, parameter);
}

// IDA_EA: 0x86AD178
// IDA_NAME: _ZN9UserQuest27ResetUrgentQuestWaitingListEv
// DEMANGLED: UserQuest::ResetUrgentQuestWaitingList(void)
void UserQuest::ResetUrgentQuestWaitingList() {
    for (int index = 0; index <= 19; ++index) {
        Quest* const active_quest = G_CDataManager()->find_quest(
            static_cast<int>(play_quest[index]));
        if (active_quest && active_quest->quest_grade == 8) {
            return;
        }
    }

    if (hasEpicQuest()) {
        urgent_quest_index = static_cast<uint32_t>(-1);
        return;
    }

    Quest* const urgent_quest = G_CDataManager()->find_quest(
        static_cast<int>(urgent_quest_index));
    if (!urgent_quest) {
        urgent_quest_index = static_cast<uint32_t>(-1);
        return;
    }

    int maximum_level = urgent_quest->maximum_level;
    if (maximum_level > oldsrv::config::kCharacterLevelMaximum) {
        maximum_level = oldsrv::config::kCharacterLevelMaximum;
    }
    if (parent->get_charac_level() + 1 > maximum_level) {
        urgent_quest_index = static_cast<uint32_t>(-1);
    }
}

// IDA_EA: 0x86AD266
// IDA_NAME: _ZN9UserQuest12hasEpicQuestEv
// DEMANGLED: UserQuest::hasEpicQuest(void)
bool UserQuest::hasEpicQuest() {
    if (!parent) {
        return false;
    }

    std::list<int> selected_quests;
    stSelectQuestParam parameter(parent);
    const UserQuest* const current_quest = parent->getCurCharacQuestR();
    WongWork::CQuestClear clear_snapshot = current_quest->clear_quest;

    QuestList* const quest_list =
        G_CDataManager()->get_arad_jumping_quest_list();
    quest_list->allowable_questlist_as_npc(
        selected_quests,
        *quest_list->getQuestNPCList(static_cast<ENUM_QUEST_GRADE>(0)),
        parameter,
        clear_snapshot,
        false);

    for (std::list<int>::const_iterator it = selected_quests.begin();
         it != selected_quests.end();
         ++it) {
        Quest* const quest = G_CDataManager()->find_quest(*it);
        if (!quest) {
            continue;
        }

        bool urgent_exception = false;
        for (std::vector<ENUM_QUEST_EXCEPTION_TYPE>::const_iterator exception =
                 quest->exception_quest_types.begin();
             exception != quest->exception_quest_types.end();
             ++exception) {
            if (static_cast<int>(*exception) == 0) {
                urgent_exception = true;
                break;
            }
        }
        if (!urgent_exception &&
            parameter.charac_level < quest->minimum_level + 12) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x86AC854
// IDA_NAME: _ZN9UserQuest12finish_questEi
// DEMANGLED: UserQuest::finish_quest(int)
int UserQuest::finish_quest(int quest_index) {
    int index = 0;
    for (;; ++index) {
        if (index > 19) {
            return 19;
        }
        if (play_quest[index] == static_cast<uint32_t>(quest_index)) {
            break;
        }
    }
    if (play_quest_trigger[index]) {
        return 22;
    }
    play_quest[index] = 0;
    play_quest_trigger[index] = 0;
    Quest* quest = G_CDataManager()->find_quest(quest_index);
    if (quest && !quest->isRepeatableQuest()) {
        clear_quest.setClearedQuest(static_cast<unsigned int>(quest_index));
    }
    if (quest->quest_grade == 8) {
        urgent_quest_index = static_cast<uint32_t>(-1);
    }
    return 0;
}

// IDA_EA: 0x86AC954
// IDA_NAME: _ZNK9UserQuest17item_reward_questEi
// DEMANGLED: UserQuest::item_reward_quest(int)const
int UserQuest::item_reward_quest(int dungeon_index) const {
    for (int index = 0; index <= 19; ++index) {
        if (play_quest[index] && play_quest_trigger[index]) {
            Quest* quest = G_CDataManager()->find_quest(
                static_cast<int>(play_quest[index]));
            if (!quest) {
                return 0;
            }
            if (static_cast<unsigned char>(quest->check_clear_item(
                    dungeon_index,
                    static_cast<int>(authen_data.clear_count)))) {
                return static_cast<int>(play_quest[index]);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x86ACA16
// IDA_NAME: _ZNK9UserQuest19get_rescue_questmapEi
// DEMANGLED: UserQuest::get_rescue_questmap(int)const
int UserQuest::get_rescue_questmap(int quest_index) const {
    for (int index = 0; index <= 19; ++index) {
        if (static_cast<int>(play_quest[index]) > 0 &&
            play_quest_trigger[index]) {
            Quest* quest = G_CDataManager()->find_quest(
                static_cast<int>(play_quest[index]));
            if (!quest) {
                return 0;
            }
            const int appearmap = quest->get_appearmap(
                quest_index,
                static_cast<int>(authen_data.clear_count));
            if (appearmap > 0) {
                return appearmap;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x86ACAD0
// IDA_NAME: _ZNK9UserQuest16check_cond_clearEi
// DEMANGLED: UserQuest::check_cond_clear(int)const
int UserQuest::check_cond_clear(int quest_index) const {
    for (int index = 0; index <= 19; ++index) {
        if (parent->getGmQuestFlag()) {
            if (play_quest[index] == static_cast<uint32_t>(quest_index)) {
                return 1;
            }
        } else if (play_quest[index] == static_cast<uint32_t>(quest_index) &&
                   !play_quest_trigger[index]) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x86AD48A
// IDA_NAME: _ZNK9UserQuest21isDoingJobChangeQuestEv
// DEMANGLED: UserQuest::isDoingJobChangeQuest(void)const
int UserQuest::isDoingJobChangeQuest() const {
    for (int index = 0; index <= 19; ++index) {
        if (play_quest[index]) {
            Quest* quest = G_CDataManager()->find_quest(
                static_cast<int>(play_quest[index]));
            if (quest && (quest->check_grow_type_mode == 1 ||
                          quest->check_grow_type_mode == 2)) {
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x86ABBA8
// IDA_NAME: _ZNK9UserQuest14get_quest_infoEPc
// DEMANGLED: UserQuest::get_quest_info(char *)const
void UserQuest::get_quest_info(char* packet_storage) const {
    stSelectQuestParam parameter(parent);
    InterfacePacketBuf& packet =
        *reinterpret_cast<InterfacePacketBuf*>(packet_storage);
    packet.put_header(0, 21);

    std::list<int> selected_quests;
    G_CDataManager()->get_arad_jumping_quest_list()->select_quest(
        selected_quests, parameter, clear_quest);
    if (urgent_quest_index != static_cast<uint32_t>(-1)) {
        selected_quests.push_back(static_cast<int>(urgent_quest_index));
    }

    packet.put_byte(parent->get_charac_level());
    int quest_count_index = packet.get_index();
    int quest_count = 0;
    packet.put_short(0);
    for (std::list<int>::const_iterator it = selected_quests.begin();
         it != selected_quests.end(); ++it) {
        const int quest_index = *it;
        if (!::GlobalData::s_timeGate_->setBlindTimeGateStateQuest(
                quest_index)) {
            ++quest_count;
            packet.put_short(quest_index);
        }
    }
    packet.put_short(quest_count_index, quest_count);
    packet.finalize(true);
}
