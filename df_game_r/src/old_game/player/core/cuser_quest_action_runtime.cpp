#include "old_game/player/core/cuser.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/history_log_abi.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <list>
#include <vector>


// IDA_EA: 0x866DA8A
// IDA_NAME: _ZN5CUser12quest_actionEiiii
// DEMANGLED: CUser::quest_action(int,int,int,int)
void CUser::quest_action(int action,
                         int quest_index,
                         int trigger_state,
                         int trigger_value) {
    Quest* const action_quest = G_CDataManager()->find_quest(quest_index);
    LogManager::logFormat(
        1,
        "quest_world_enter_diag.cpp",
        "void CUser::quest_action(int,int,int,int)",
        0,
        "[WORLD_ENTER][QUEST_ACTION] uid=%d state=%d action=%d quest=%d arg2=%d arg3=%d lookup=%d grade=%d type=%d",
        get_acc_id(),
        get_state(),
        action,
        quest_index,
        trigger_state,
        trigger_value,
        action_quest ? 1 : 0,
        action_quest ? action_quest->quest_grade : -1,
        action_quest ? action_quest->quest_type : -1);
    if (!action_quest || action_quest->quest_grade == 7) {
        return;
    }

    PacketGuard packet;
    packet.put_header(1, action);

    if (action == ENUM_CMDPACKET_ACCEPT_QUEST) {
        Quest* const accepted_quest =
            G_CDataManager()->find_quest(quest_index);
        if ((accepted_quest &&
             accepted_quest->quest_type == 1 &&
             GlobalData::s_revengeDungeonMgr->IsOpenRevengeDungeon() != 1) ||
            (accepted_quest &&
             accepted_quest->quest_type == 2 &&
             !accepted_quest->IsOpenScheduleQuest())) {
            packet.put_byte(0);
            packet.put_byte(21);
            packet.finalize(true);
            Send(packet);
            return;
        }

        int accepted_slot = 0;
        const int result = getCurCharacQuestW()->accept_quest(
            quest_index,
            reinterpret_cast<char*>(&packet),
            accepted_slot);
        if (accepted_quest && accepted_quest->trigger_type == 2 &&
            accepted_quest->clear_condition_values.size() >= 5u &&
            accepted_quest->clear_condition_values.front() == -2) {
            const std::vector<int>& values =
                accepted_quest->clear_condition_values;
            LogManager::logFormat(
                1,
                "quest_monster_diag.cpp",
                "void CUser::quest_action(int,int,int,int)",
                0,
                "[QUEST_MONSTER][ACCEPT] uid=%d quest=%d sentinel=%d area=%d min_diff=%d selector=%d count=%d packed=%d lane0=%d lane1=%d lane2=%d result=%d",
                get_acc_id(),
                quest_index,
                values[0],
                values[1],
                values[2],
                values[3],
                values[4],
                accepted_slot,
                accepted_slot & 0x1FF,
                (accepted_slot >> 9) & 0x1FF,
                (accepted_slot >> 18) & 0x1FF,
                result);
        } else if (accepted_quest && accepted_quest->trigger_type == 6 &&
                   accepted_quest->clear_condition_values.size() >= 5u) {
            const std::vector<int>& values =
                accepted_quest->clear_condition_values;
            LogManager::logFormat(
                1,
                "quest_monster_diag.cpp",
                "void CUser::quest_action(int,int,int,int)",
                0,
                "[QUEST_MONSTER][ACCEPT_HUNT] uid=%d quest=%d group=0 dungeon=%d min_diff=%d template=%d target_enemy_type=%d count=%d groups=%u packed=%d lanes=%d/%d/%d result=%d",
                get_acc_id(),
                quest_index,
                values[0],
                values[1],
                values[2],
                values[3],
                values[4],
                static_cast<unsigned int>(values.size() / 5u),
                accepted_slot,
                accepted_slot & 0x1FF,
                (accepted_slot >> 9) & 0x1FF,
                (accepted_slot >> 18) & 0x1FF,
                result);
        }
        LogManager::logFormat(
            1,
            "quest_world_enter_diag.cpp",
            "void CUser::quest_action(int,int,int,int)",
            0,
            "[WORLD_ENTER][QUEST_ACCEPT] uid=%d quest=%d slot=%d result=%d",
            get_acc_id(),
            quest_index,
            accepted_slot,
            result);
        if (result > 0) {
            packet.put_byte(0);
            packet.put_byte(result);
        }
        packet.finalize(true);
        Send(packet);
        return;
    }

    if (action == ENUM_CMDPACKET_GIVEUP_QUEST) {
        const int result =
            getCurCharacQuestW()->giveup_quest(quest_index);
        if (result > 0) {
            packet.put_byte(0);
            packet.put_byte(result);
        } else {
            packet.put_byte(1);
            packet.put_short(quest_index);

            Quest* const quest =
                G_CDataManager()->find_quest(quest_index);
            if (!quest) {
                return;
            }
            if (!quest->depend_give_items.empty()) {
                bool equipment_changed = false;
                std::vector<int> deleted_slots;
                deleted_slots.reserve(
                    3u * quest->depend_give_items.size());
                getCurCharacInvenW()->delete_event_items(
                    quest->depend_give_items,
                    deleted_slots,
                    equipment_changed,
                    static_cast<eMoneySubReason>(4),
                    static_cast<eItemDelReason>(4),
                    false);
                for (std::vector<int>::const_iterator it =
                         deleted_slots.begin();
                     it != deleted_slots.end();) {
                    const int slot = *it++;
                    const int item_space = *it++;
                    SendUpdateItem(
                        static_cast<eSendTarget>(1),
                        static_cast<ENUM_ITEMSPACE>(item_space),
                        slot);
                    ++it;
                }
            }
            SendHasNoGoWithApcQuestInParty(quest);
        }
        packet.finalize(true);
        Send(packet);
        return;
    }

    if (action == ENUM_CMDPACKET_SET_QUEST_TRIGGER) {
        UserQuest* const quest_state = getCurCharacQuestW();
        int quest_slot = -1;
        int trigger_before = -1;
        for (int slot = 0; slot <= 19; ++slot) {
            if (quest_state->play_quest[slot] ==
                static_cast<uint32_t>(quest_index)) {
                quest_slot = slot;
                trigger_before = static_cast<int>(
                    quest_state->play_quest_trigger[slot]);
                break;
            }
        }
        const int result = quest_state->set_trigger(
            quest_index,
            static_cast<char>(trigger_state),
            trigger_value != 0);
        if (action_quest &&
            (action_quest->trigger_type == 2 ||
             action_quest->trigger_type == 6)) {
            const int trigger_after = quest_slot >= 0
                ? static_cast<int>(quest_state->play_quest_trigger[quest_slot])
                : -1;
            LogManager::logFormat(
                1,
                "quest_monster_diag.cpp",
                "void CUser::quest_action(int,int,int,int)",
                0,
                "[QUEST_MONSTER][CMD33] uid=%d quest=%d trigger_type=%d subtype=%d lane=0x%02X add=%d slot=%d before=%d before_lanes=%d/%d/%d after=%d after_lanes=%d/%d/%d result=%d",
                get_acc_id(),
                quest_index,
                action_quest->trigger_type,
                action_quest->trigger_subtype,
                trigger_state & 0xFF,
                trigger_value != 0 ? 1 : 0,
                quest_slot,
                trigger_before,
                trigger_before < 0 ? -1 : trigger_before & 0x1FF,
                trigger_before < 0 ? -1 : (trigger_before >> 9) & 0x1FF,
                trigger_before < 0 ? -1 : (trigger_before >> 18) & 0x1FF,
                trigger_after,
                trigger_after < 0 ? -1 : trigger_after & 0x1FF,
                trigger_after < 0 ? -1 : (trigger_after >> 9) & 0x1FF,
                trigger_after < 0 ? -1 : (trigger_after >> 18) & 0x1FF,
                result);
        }
        if (result == -1) {
            packet.put_byte(0);
            packet.put_byte(22);
        } else {
            packet.put_byte(1);
            packet.put_short(quest_index);
            packet.put_int(result);
            if (!result) {
                Quest* const quest =
                    G_CDataManager()->find_quest(quest_index);
                if (!quest) {
                    return;
                }
                SendHasNoGoWithApcQuestInParty(quest);
            }
        }
        packet.finalize(true);
        Send(packet);
        return;
    }

    if (action != ENUM_CMDPACKET_FINISH_QUEST) {
        return;
    }

    const uint32_t current_tick = static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
    if (current_tick - quest_reward_request_tick_ > 1000u) {
        quest_reward_request_count_ = 0u;
        quest_reward_request_tick_ = current_tick;
    } else {
        ++quest_reward_request_count_;
        if (quest_reward_request_count_ > 1u) {
            getHackAnalyzer()->addServerHackCnt(
                this,
                static_cast<WongWork::ENUM_HACKTYPE>(402),
                1u,
                0u,
                0u);
            packet.put_byte(0);
            packet.put_byte(1);
            packet.finalize(true);
            Send(packet);
            return;
        }
    }

    const int result =
        quest_reward(quest_index, trigger_state, trigger_value);
    LogManager::logFormat(
        1,
        "quest_world_enter_diag.cpp",
        "void CUser::quest_action(int,int,int,int)",
        0,
        "[WORLD_ENTER][QUEST_REWARD] uid=%d quest=%d selected=%d multiplier=%d result=%d cond_clear=%d clear=%d trigger_type=%d reward_type=%d grade=%d",
        get_acc_id(),
        quest_index,
        trigger_state,
        trigger_value,
        result,
        getCurCharacQuestR()->check_cond_clear(quest_index),
        getCurCharacQuestR()->isClearQuest(quest_index),
        action_quest ? action_quest->trigger_type : -1,
        action_quest ? action_quest->reward_type : -1,
        action_quest ? action_quest->quest_grade : -1);
    if (result > 0) {
        packet.put_byte(0);
        packet.put_byte(result);
        packet.finalize(true);
        Send(packet);
        HistoryLog::WriteFineQuest(history_stream(), quest_index);
        return;
    }

    std::list<int> mail_quests;
    const stSelectQuestParam select_parameter(this);
    if (getCurCharacQuestR()->get_mail_quest_info(
            mail_quests,
            select_parameter)) {
        for (std::list<int>::const_iterator it = mail_quests.begin();
             it != mail_quests.end();
             ++it) {
            Quest* const mail_quest =
                G_CDataManager()->find_quest(*it);
            if (mail_quest &&
                mail_quest->mail_time &&
                mail_quest->quest_index == quest_index) {
                _postQuestTriggerItem(mail_quest);
            }
        }
    }

    getCurCharacQuestW()->finish_quest(quest_index);
    PacketGuard quest_packet;
    getCurCharacQuestR()->get_quest_info(
        reinterpret_cast<char*>(&quest_packet));
    Send(quest_packet);

    Quest* const finished_quest =
        G_CDataManager()->find_quest(quest_index);
    if (finished_quest && finished_quest->quest_grade == 3) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            this,
            static_cast<APSystem::_ActionGroupIndex>(14),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
    }
    history_log().QuestComplete(quest_index);
    _onQuestClear(quest_index, trigger_value);
}

// IDA_EA: 0x866D972
// IDA_NAME: _ZN5CUser30SendHasNoGoWithApcQuestInPartyEPK5Quest
// DEMANGLED: CUser::SendHasNoGoWithApcQuestInParty(Quest const*)
void CUser::SendHasNoGoWithApcQuestInParty(const Quest* quest) {
    if ((get_state() == 5 || get_state() == 10) &&
        quest->trigger_type == 3 && !quest->trigger_subtype) {
        CParty* const party = GetParty();
        if (party && party->HasNoQuestGoWithApcInParty(quest)) {
            PacketGuard packet;
            packet.put_header(0, 168);
            packet.put_int(quest->quest_index);
            packet.finalize(true);
            party->send_to_party(packet);
        }
    }
}

// IDA_EA: 0x8657570
// IDA_NAME: _ZN5CUser15ResetDailyQuestEv
// DEMANGLED: CUser::ResetDailyQuest(void)
void CUser::ResetDailyQuest() {
    std::list<int> daily_quests;
    G_CDataManager()->get_quest_list()->GetDailyQuestList(daily_quests);

    for (std::list<int>::const_iterator quest_index = daily_quests.begin();
         quest_index != daily_quests.end(); ++quest_index) {
        if (getCurCharacQuestRefR()->isClearQuest(*quest_index)) {
            getCurCharacQuestW()->resetClearQuest(*quest_index);
        }
    }
}

// IDA_EA: 0x8657662
// IDA_NAME: _ZN5CUser18ResetTrainingQuestEv
// DEMANGLED: CUser::ResetTrainingQuest(void)
void CUser::ResetTrainingQuest() {
    std::vector<int> training_quests;
    const int16_t charac_level = static_cast<int16_t>(get_charac_level());
    G_CDataManager()->get_quest_list()->GetTrainingQuestList(
        training_quests,
        charac_level);

    for (std::vector<int>::const_iterator quest_index =
             training_quests.begin();
         quest_index != training_quests.end(); ++quest_index) {
        if (getCurCharacQuestRefR()->isClearQuest(*quest_index)) {
            getCurCharacQuestW()->resetClearQuest(*quest_index);
        }
    }
}

// IDA_EA: 0x868D144
// IDA_NAME: _ZN5CUser16verifyDoingQuestEv
// DEMANGLED: CUser::verifyDoingQuest(void)
bool CUser::verifyDoingQuest() {
    for (int slot = 0; slot <= 19; ++slot) {
        const uint32_t quest_index = getCurCharacQuestR()->play_quest[slot];
        if (quest_index &&
            getCurCharacQuestR()->isClearQuest(
                static_cast<int>(quest_index))) {
            getCurCharacQuestW()->play_quest[slot] = 0;
            getCurCharacQuestW()->play_quest_trigger[slot] = 0;
        }
    }

    // The old loop's final `slot <= 19` comparison is its return register.
    return false;
}
