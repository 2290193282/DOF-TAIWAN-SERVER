#include "old_game/progression/quest/character_mission_list.h"

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/pvp_mission_stream.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <cstring>
#include <list>

#include <stddef.h>
#include <stdint.h>

typedef char CMissionList_Charac_Size_Must_Be_0x158[
    (sizeof(CMissionList_Charac) == 0x158) ? 1 : -1];

// IDA_EA: 0x832AF8A
// IDA_NAME: _ZN19CMissionList_CharacC2Ev
// DEMANGLED: CMissionList_Charac::CMissionList_Charac(void)
CMissionList_Charac::CMissionList_Charac() : cleared_() {}

// IDA_EA: 0x85E4576
// IDA_NAME: _ZN19CMissionList_Charac9_saveDataEP5CUser
// DEMANGLED: CMissionList_Charac::_saveData(CUser *)
int CMissionList_Charac::_saveData(CUser* user) {
    Stream* stream =
        GlobalData::s_stream_pool->Acquire("PvP_MissionSystem.cpp", 1189);
    CStreamGuard guard(stream, true);
    *guard << 0x263;
    *guard << user->GetUID();
    SIG_REQUEST_PVP_MISSION_LIST* payload =
        guard->GetInBuffer<SIG_REQUEST_PVP_MISSION_LIST>();
    if (!payload) {
        return 0;
    }
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    for (int i = 0; i <= 35; ++i) {
        payload->missions[i] = missions_[i];
    }
    BitsetToStr(cleared_,
                reinterpret_cast<unsigned char*>(payload->mission_state));
    payload->issue_mode_win = issue_mode_win_;
    payload->issue_mode_loss = issue_mode_loss_;
    payload->issue_all = issue_all_;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x85E476C
// IDA_NAME: _ZN19CMissionList_Charac6_resetEv
// DEMANGLED: CMissionList_Charac::_reset(void)
int CMissionList_Charac::_reset() {
    std::memset(missions_, 0, sizeof(missions_));
    cleared_.reset();
    issue_mode_win_ = 0;
    issue_mode_loss_ = 0;
    issue_all_ = 0;
    clear_stealing_skill_ = 0;
    owner_ = 0;
    return static_cast<int>(reinterpret_cast<intptr_t>(this));
}

// IDA_EA: 0x85E47DE
// IDA_NAME: _ZN19CMissionList_Charac8loadDataEP5CUserPc
// DEMANGLED: CMissionList_Charac::loadData(CUser *,char *)
bool CMissionList_Charac::loadData(CUser* user, char* data) {
    _reset();
    owner_ = user;
    const SIG_REQUEST_PVP_MISSION_LIST* payload =
        reinterpret_cast<const SIG_REQUEST_PVP_MISSION_LIST*>(data);
    for (int i = 0; i <= 35; ++i) {
        missions_[i] = payload->missions[i];
    }
    StrToBitset(
        const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(
            payload->mission_state)),
        cleared_);
    issue_mode_win_ = payload->issue_mode_win;
    issue_mode_loss_ = payload->issue_mode_loss;
    issue_all_ = payload->issue_all;

    const std::list<CMission*>& stealing_missions =
        G_CDataManager()->get_kind_mission_list(6);
    for (std::list<CMission*>::const_iterator it = stealing_missions.begin();
         it != stealing_missions.end(); ++it) {
        if (isClearMission((*it)->GetMissionIndex())) {
            clear_stealing_skill_ = 1;
            break;
        }
    }
    if (isUserNewbie()) {
        MakeMissionList_forNewbies(*user);
        Reset_DailyMission();
    } else {
        MakeMissionList_forOldUser(*user);
    }
    if (CheckDailyScheduleTime(
            G_CEnvironment()->get_fatigue_time(),
            user->getCurCharacLastPlayTick(),
            GlobalData::s_systemTime_.getCurSec())) {
        Reset_DailyMission();
    }
    return true;
}

// IDA_EA: 0x85E4A04
// IDA_NAME: _ZNK19CMissionList_Charac12isUserNewbieEv
// DEMANGLED: CMissionList_Charac::isUserNewbie(void)const
bool CMissionList_Charac::isUserNewbie() const {
    static const std::bitset<256> zero_bit(0);
    return empty_progress() && cleared_ == zero_bit;
}

// IDA_EA: 0x85E4AC4
// IDA_NAME: _ZNK19CMissionList_Charac7getDataEPc
// DEMANGLED: CMissionList_Charac::getData(char *)const
void CMissionList_Charac::getData(char* data) const {
    SIG_REQUEST_PVP_MISSION_LIST* payload =
        reinterpret_cast<SIG_REQUEST_PVP_MISSION_LIST*>(data);
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = charac_no_;
    for (int i = 0; i <= 35; ++i) {
        payload->missions[i] = missions_[i];
    }
    BitsetToStr(cleared_,
                reinterpret_cast<unsigned char*>(payload->mission_state));
    payload->issue_mode_win = issue_mode_win_;
    payload->issue_mode_loss = issue_mode_loss_;
    payload->issue_all = issue_all_;
}

// IDA_EA: 0x85E4BC8
// IDA_NAME: _ZNK19CMissionList_Charac15getIndex_byKindEi
// DEMANGLED: CMissionList_Charac::getIndex_byKind(int)const
int CMissionList_Charac::getIndex_byKind(int mission_kind) const {
    if (mission_kind > 0 && mission_kind <= 35) {
        return missions_[mission_kind].mission_index;
    }
    return 0;
}

// IDA_EA: 0x85E4BEC
// IDA_NAME: _ZN19CMissionList_Charac14Remove_MissionEi
// DEMANGLED: CMissionList_Charac::Remove_Mission(int)
charac_expand::CData* CMissionList_Charac::Remove_Mission(int mission_kind) {
    charac_expand::CData* result;
    if (mission_kind > 0 && mission_kind <= 35) {
        if (owner_) {
            CUser_history_log(owner_)->pvpMissionDel(
                static_cast<int16_t>(missions_[mission_kind].mission_kind),
                static_cast<int16_t>(missions_[mission_kind].mission_index));
        }
        missions_[mission_kind].clear();
        return alter();
    }
    return result;
}

// IDA_EA: 0x85E4C6C
// IDA_NAME: _ZN19CMissionList_Charac13addNewMissionERK11MissionInfo
// DEMANGLED: CMissionList_Charac::addNewMission(MissionInfo const&)
int CMissionList_Charac::addNewMission(const MissionInfo& info) {
    if (!G_CDataManager()->find_mission(
            static_cast<int16_t>(info.mission_index))) {
        return 0;
    }
    const int mission_kind = static_cast<int16_t>(info.mission_kind);
    missions_[mission_kind] = info;
    if (owner_) {
        return CUser_history_log(owner_)->pvpMissionAdd(
            static_cast<int16_t>(info.mission_kind),
            static_cast<int16_t>(info.mission_index),
            static_cast<int16_t>(info.current_value),
            static_cast<int16_t>(info.target_value));
    }
    return 0;
}

// IDA_EA: 0x85E4D14
// IDA_NAME: _ZN19CMissionList_Charac26MakeMissionList_forNewbiesERK5CUser
// DEMANGLED: CMissionList_Charac::MakeMissionList_forNewbies(CUser const&)
bool CMissionList_Charac::MakeMissionList_forNewbies(const CUser& user) {
    for (unsigned int kind = 1; kind <= 35; ++kind) {
        std::list<CMission*>& list =
            G_CDataManager()->get_kind_mission_list(kind);
        if (list.empty()) {
            continue;
        }
        MissionInfo info;
        std::list<CMission*>::const_iterator it = list.begin();
        if ((*it)->GetMissionGrade() == 2) {
            continue;
        }
        for (; it != list.end(); ++it) {
            if ((*it)->isAcceptCondition(user)) {
                (*it)->makeMissionInfo(info, user);
                addNewMission(info);
                alter();
                break;
            }
        }
    }
    return false;
}

// IDA_EA: 0x85E4E4C
// IDA_NAME: _ZN19CMissionList_Charac26MakeMissionList_forOldUserERK5CUser
// DEMANGLED: CMissionList_Charac::MakeMissionList_forOldUser(CUser const&)
bool CMissionList_Charac::MakeMissionList_forOldUser(const CUser& user) {
    for (unsigned int kind = 1; kind <= 35; ++kind) {
        std::list<CMission*>& list =
            G_CDataManager()->get_kind_mission_list(kind);
        if (list.empty()) {
            continue;
        }
        MissionInfo info;
        std::list<CMission*>::const_iterator it = list.begin();
        if (getIndex_byKind((*it)->GetMissionKind()) != 0 ||
            (*it)->GetMissionGrade() == 2) {
            continue;
        }
        for (; it != list.end(); ++it) {
            if ((*it)->isAcceptCondition(user)) {
                (*it)->makeMissionInfo(info, user);
                addNewMission(info);
                alter();
                break;
            }
        }
    }
    return false;
}

// IDA_EA: 0x85E4FBA
// IDA_NAME: _ZN19CMissionList_Charac24MakeMissionList_JustKindERK5CUserj
// DEMANGLED: CMissionList_Charac::MakeMissionList_JustKind(CUser const&,unsigned int)
int CMissionList_Charac::MakeMissionList_JustKind(
    const CUser& user, unsigned int mission_kind) {
    std::list<CMission*>& list =
        G_CDataManager()->get_kind_mission_list(mission_kind);
    if (list.empty()) {
        return 0;
    }
    MissionInfo info;
    std::list<CMission*>::const_iterator it = list.begin();
    if (getIndex_byKind((*it)->GetMissionKind()) != 0) {
        return 0;
    }
    for (; it != list.end(); ++it) {
        if ((*it)->isAcceptCondition(user)) {
            (*it)->makeMissionInfo(info, user);
            addNewMission(info);
            alter();
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85E50FA
// IDA_NAME: _ZNK19CMissionList_Charac14isClearMissionEi
// DEMANGLED: CMissionList_Charac::isClearMission(int)const
bool CMissionList_Charac::isClearMission(int mission_index) const {
    return mission_index >= 0 && mission_index < 0x100 &&
           cleared_[static_cast<size_t>(mission_index)];
}

// IDA_EA: 0x85E513A
// IDA_NAME: _ZNK19CMissionList_Charac14getWinningRateEv
// DEMANGLED: CMissionList_Charac::getWinningRate(void)const
int CMissionList_Charac::getWinningRate() const {
    uint16_t history = issue_all_;
    int victories = 0;
    for (int i = 0; i <= 9; ++i) {
        if ((history & 1u) != 0) {
            ++victories;
        }
        history = static_cast<uint16_t>(history >> 1);
    }
    return 10 * victories;
}

// IDA_EA: 0x85E5194
// IDA_NAME: _ZNK19CMissionList_Charac13getIssue_modeEb
// DEMANGLED: CMissionList_Charac::getIssue_mode(bool)const
int CMissionList_Charac::getIssue_mode(bool battle_mode) const {
    return battle_mode ? issue_mode_win_ : issue_mode_loss_;
}

// IDA_EA: 0x85E51BE
// IDA_NAME: _ZNK19CMissionList_Charac12getIssue_allEv
// DEMANGLED: CMissionList_Charac::getIssue_all(void)const
int CMissionList_Charac::getIssue_all() const { return issue_all_; }

// IDA_EA: 0x85E51CE
// IDA_NAME: _ZN19CMissionList_Charac8setIssueEbb
// DEMANGLED: CMissionList_Charac::setIssue(bool,bool)
CMissionList_Charac* CMissionList_Charac::setIssue(bool result,
                                                  bool battle_mode) {
    uint16_t& history = battle_mode ? issue_mode_win_ : issue_mode_loss_;
    history = static_cast<uint16_t>(history * 2 + result);
    issue_all_ = static_cast<uint16_t>(issue_all_ * 2 + result);
    return this;
}

// IDA_EA: 0x85E52A8
// IDA_NAME: _ZN19CMissionList_Charac16Send_MissionListER5CUser
// DEMANGLED: CMissionList_Charac::Send_MissionList(CUser &)
int CMissionList_Charac::Send_MissionList(CUser& user) {
    PacketGuard packet;
    packet.put_header(0, 320);
    int count_index = packet.get_index();
    packet.put_byte(0);
    int mission_count = 0;
    for (int kind = 1; kind <= 35; ++kind) {
        if (!missions_[kind].mission_index) {
            continue;
        }
        packet.put_short(missions_[kind].mission_index);
        if (kind == 27) {
            int victories = 0;
            for (int i = 0; i <= 4; ++i) {
                if (((static_cast<int16_t>(missions_[kind].current_value) >>
                      i) & 1) != 0) {
                    ++victories;
                }
            }
            packet.put_short(victories);
        } else {
            packet.put_short(missions_[kind].current_value);
        }
        CMission* mission = G_CDataManager()->find_mission(
            static_cast<int16_t>(missions_[kind].mission_index));
        if (mission) {
            if (kind == 27 || kind == 31 || kind == 32) {
                packet.put_short(missions_[kind].target_value);
            } else {
                packet.put_short(mission->getClearCondition());
            }
            ++mission_count;
        } else {
            missions_[kind].clear();
        }
    }
    packet.put_byte(count_index, mission_count);
    unsigned char clear_state[32];
    BitsetToStr(cleared_, clear_state);
    for (int i = 0; i <= 31; ++i) {
        packet.put_byte(clear_state[i]);
    }
    packet.finalize(true);
    user.Send(packet);
    return 1;
}

// IDA_EA: 0x85E5514
// IDA_NAME: _ZN19CMissionList_Charac10ResetDailyEv
// DEMANGLED: CMissionList_Charac::ResetDaily(void)
void CMissionList_Charac::ResetDaily() { Reset_DailyMission(); }

// IDA_EA: 0x85E5528
// IDA_NAME: _ZN19CMissionList_Charac18Reset_DailyMissionEv
// DEMANGLED: CMissionList_Charac::Reset_DailyMission(void)
void CMissionList_Charac::Reset_DailyMission() {
    const std::bitset<32> daily =
        G_CDataManager()->get_daily_mission_kind();
    for (int kind = 0; kind <= 31; ++kind) {
        if (daily[static_cast<size_t>(kind)]) {
            missions_[kind].clear();
            missions_[kind].mission_kind = static_cast<uint16_t>(kind);
        }
    }
    std::vector<std::pair<unsigned int, unsigned int> > new_missions;
    G_CDataManager()->get_New_DailyMission(new_missions);
    if (new_missions.empty()) {
        cMyTrace trace("void CMissionList_Charac::Reset_DailyMission()",
                       1641, 5);
        trace("[@missionSystem] Daily Mission List is Empty!!");
    }
    for (size_t i = 0; i < new_missions.size(); ++i) {
        const unsigned int kind = new_missions[i].first;
        if (daily[kind]) {
            MakeMissionList_JustKind(*owner_, kind);
        }
    }
    alter();
}

// IDA_EA: 0x85E56EE
// IDA_NAME: _ZN19CMissionList_Charac18GM_SetMissionValueEii
// DEMANGLED: CMissionList_Charac::GM_SetMissionValue(int,int)
charac_expand::CData* CMissionList_Charac::GM_SetMissionValue(
    int mission_kind, int value) {
    charac_expand::CData* result;
    if (mission_kind > 0 && mission_kind <= 35) {
        missions_[mission_kind].current_value = static_cast<uint16_t>(value);
        return alter();
    }
    return result;
}

// IDA_EA: 0x85E5724
// IDA_NAME: _ZNK19CMissionList_Charac14empty_progressEv
// DEMANGLED: CMissionList_Charac::empty_progress(void)const
int CMissionList_Charac::empty_progress() const {
    for (int kind = 1; kind <= 35; ++kind) {
        if (missions_[kind].mission_index) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x85E5760
// IDA_NAME: _ZN19CMissionList_Charac25Send_Cleared_Mission_ListER5CUserRKSt6vectorIiSaIiEE
// DEMANGLED: CMissionList_Charac::Send_Cleared_Mission_List(CUser &,std::vector<int> const&)
void CMissionList_Charac::Send_Cleared_Mission_List(
    CUser& user, const std::vector<int>& mission_indices) {
    if (mission_indices.empty()) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, 321);
    packet.put_byte(static_cast<int>(mission_indices.size()));
    for (size_t i = 0; i < mission_indices.size(); ++i) {
        packet.put_short(mission_indices[i]);
    }
    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x85E5858
// IDA_NAME: _ZN19CMissionList_Charac17Send_Fail_MissionER5CUsers
// DEMANGLED: CMissionList_Charac::Send_Fail_Mission(CUser &,short)
void CMissionList_Charac::Send_Fail_Mission(CUser& user,
                                           short mission_index) {
    PacketGuard packet;
    packet.put_header(0, 343);
    packet.put_short(mission_index);
    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x85E58F4
// IDA_NAME: _ZN19CMissionList_Charac17Update_Kill_eventER5CUserRK31MissionClearCondition_Parameter
// DEMANGLED: CMissionList_Charac::Update_Kill_event(CUser &,MissionClearCondition_Parameter const&)
void CMissionList_Charac::Update_Kill_event(
    CUser& user, const MissionClearCondition_Parameter& parameter) {
    if (G_GameWorld()->GetChannelType() == 15) {
        alter();
        std::vector<int> cleared;
        check_mission_kind(5, user, parameter, cleared);
        check_mission_kind(15, user, parameter, cleared);
        check_mission_kind(25, user, parameter, cleared);
        Send_Cleared_Mission_List(user, cleared);
    }
}

// IDA_EA: 0x85E59F0
// IDA_NAME: _ZN19CMissionList_Charac16Update_Win_eventER5CUserRK31MissionClearCondition_ParameterRb
// DEMANGLED: CMissionList_Charac::Update_Win_event(CUser &,MissionClearCondition_Parameter const&,bool &)
void CMissionList_Charac::Update_Win_event(
    CUser& user,
    const MissionClearCondition_Parameter& parameter,
    bool& within_mission_cleared) {
    if (G_GameWorld()->GetChannelType() != 15) {
        return;
    }
    Update_Within_Mission_event(user, parameter, within_mission_cleared);
    alter();
    if (within_mission_cleared || getIndex_byKind(27)) {
        return;
    }
    std::vector<int> cleared;
    const unsigned int kinds[] = {
        2, 4, 9, 10, 12, 14, 17, 19, 20, 22, 24, 26};
    for (size_t i = 0; i < sizeof(kinds) / sizeof(kinds[0]); ++i) {
        check_mission_kind(kinds[i], user, parameter, cleared);
    }
    Send_Cleared_Mission_List(user, cleared);
}

// IDA_EA: 0x85E5CAA
// IDA_NAME: _ZN19CMissionList_Charac23Update_RecvPacket_eventER5CUserRSt6vectorIsSaIsEERS2_IiSaIiEE
// DEMANGLED: CMissionList_Charac::Update_RecvPacket_event(CUser &,std::vector<short> &,std::vector<int> &)
char CMissionList_Charac::Update_RecvPacket_event(
    CUser& user,
    std::vector<short>& mission_indices,
    std::vector<int>& cleared_missions) {
    if (G_GameWorld()->GetChannelType() != 15) {
        return 1;
    }
    if (getIndex_byKind(27)) {
        return 1;
    }
    alter();
    PvP_Room* room = user.GetPVPRoom();
    if (!room) {
        return 0;
    }
    MissionClearCondition_Parameter parameter(
        static_cast<PVP_BATTLE_MODE>(room->get_pvp_battle_mode()));
    for (size_t i = 0; i < mission_indices.size(); ++i) {
        CMission* mission =
            G_CDataManager()->find_mission(mission_indices[i]);
        if (!mission) {
            continue;
        }
        const int kind = mission->GetMissionKind();
        if (kind != 6 && kind != 21) {
            cMyTrace trace(
                "void CMissionList_Charac::Update_RecvPacket_event(CUser&, std::vector<short int>&, std::vector<int>&)",
                1796,
                5);
            trace("[@missionSystem] HackUser! send [%d kind]\n", kind);
        } else {
            check_mission_kind(kind, user, parameter, cleared_missions);
        }
    }
    return 0;
}

// IDA_EA: 0x85E5E4A
// IDA_NAME: _ZN19CMissionList_Charac21Update_GameOver_eventER5CUserRK31MissionClearCondition_Parameter
// DEMANGLED: CMissionList_Charac::Update_GameOver_event(CUser &,MissionClearCondition_Parameter const&)
void CMissionList_Charac::Update_GameOver_event(
    CUser& user, const MissionClearCondition_Parameter& parameter) {
    if (G_GameWorld()->GetChannelType() != 15 || getIndex_byKind(27)) {
        return;
    }
    alter();
    std::vector<int> cleared;
    const unsigned int kinds[] = {1, 3, 8, 13, 16, 18, 23};
    for (size_t i = 0; i < sizeof(kinds) / sizeof(kinds[0]); ++i) {
        check_mission_kind(kinds[i], user, parameter, cleared);
    }
    Send_Cleared_Mission_List(user, cleared);
}

// IDA_EA: 0x85E600A
// IDA_NAME: _ZN19CMissionList_Charac23Update_EnterWorld_eventER5CUser
// DEMANGLED: CMissionList_Charac::Update_EnterWorld_event(CUser &)
void CMissionList_Charac::Update_EnterWorld_event(CUser& user) {
    const int channel_type = G_GameWorld()->GetChannelType();
    if (channel_type != 15 && channel_type != 16) {
        return;
    }
    alter();
    std::vector<int> cleared;
    MissionClearCondition_Parameter parameter(PVP_BATTLE_MODE_2);
    parameter.pvp_channel_type = G_CDataManager()->GetPvPChannelType();
    parameter.channel_type = G_GameWorld()->GetChannelType();
    check_mission_kind(7, user, parameter, cleared);
    check_mission_kind(30, user, parameter, cleared);
    Send_Cleared_Mission_List(user, cleared);
}

// IDA_EA: 0x85E612E
// IDA_NAME: _ZN19CMissionList_Charac20Update_Revenge_eventER5CUser15PVP_BATTLE_MODEb
// DEMANGLED: CMissionList_Charac::Update_Revenge_event(CUser &,PVP_BATTLE_MODE,bool)
void CMissionList_Charac::Update_Revenge_event(
    CUser& user, PVP_BATTLE_MODE battle_mode, bool revenge) {
    if (G_GameWorld()->GetChannelType() != 15 || getIndex_byKind(27)) {
        return;
    }
    if (revenge) {
        alter();
        std::vector<int> cleared;
        MissionClearCondition_Parameter parameter(battle_mode);
        parameter.issue_history = static_cast<uint16_t>(
            getIssue_mode(battle_mode == PVP_BATTLE_MODE_2));
        check_mission_kind(12, user, parameter, cleared);
        check_mission_kind(22, user, parameter, cleared);
        Send_Cleared_Mission_List(user, cleared);
    } else {
        missions_[13].current_value = 0;
        missions_[23].current_value = 0;
    }
}

// IDA_EA: 0x85E627C
// IDA_NAME: _ZN19CMissionList_Charac18check_mission_kindEjR5CUserRK31MissionClearCondition_ParameterRSt6vectorIiSaIiEE
// DEMANGLED: CMissionList_Charac::check_mission_kind(unsigned int,CUser &,MissionClearCondition_Parameter const&,std::vector<int> &)
void CMissionList_Charac::check_mission_kind(
    unsigned int mission_kind,
    CUser& user,
    const MissionClearCondition_Parameter& parameter,
    std::vector<int>& cleared_missions) {
    bool failed = false;
    if (!missions_[mission_kind].mission_index) {
        return;
    }
    CMission* mission = G_CDataManager()->find_mission(
        static_cast<int16_t>(missions_[mission_kind].mission_index));
    if (!mission || !mission->canUpdate(parameter)) {
        return;
    }
    mission->Update(missions_[mission_kind], parameter);
    if (mission->isClearable(missions_[mission_kind], failed)) {
        const int mission_index =
            static_cast<int16_t>(missions_[mission_kind].mission_index);
        cleared_missions.push_back(mission_index);
        if (mission_kind == 27) {
            missions_[7].clear();
            missions_[30].clear();
        }
        ClearMission(user, static_cast<short>(mission_index));
    }
    if (failed) {
        Send_Fail_Mission(
            user,
            static_cast<short>(missions_[mission_kind].mission_index));
    }
}

// IDA_EA: 0x85E63D6
// IDA_NAME: _ZN19CMissionList_Charac16ClearMissionKindER5CUsers
// DEMANGLED: CMissionList_Charac::ClearMissionKind(CUser &,short)
char CMissionList_Charac::ClearMissionKind(CUser& user,
                                          short mission_kind) {
    if (mission_kind > 0 && mission_kind <= 35) {
        return ClearMission(
            user,
            static_cast<short>(missions_[mission_kind].mission_index));
    }
    return static_cast<char>(mission_kind);
}

// IDA_EA: 0x85E641C
// IDA_NAME: _ZN19CMissionList_Charac12ClearMissionER5CUsers
// DEMANGLED: CMissionList_Charac::ClearMission(CUser &,short)
char CMissionList_Charac::ClearMission(CUser& user, short mission_index) {
    CMission* mission = G_CDataManager()->find_mission(mission_index);
    if (!mission) {
        cMyTrace trace(
            "void CMissionList_Charac::ClearMission(CUser&, short int)",
            1943,
            5);
        return static_cast<char>(
            trace("[@missionSystem] InvalidIndex: <index : %d>",
                  mission_index));
    }
    const int kind = mission->GetMissionKind();
    if (kind == 6) {
        clear_stealing_skill_ = 1;
    }
    if (missions_[kind].mission_index !=
        static_cast<uint16_t>(mission_index)) {
        cMyTrace trace(
            "void CMissionList_Charac::ClearMission(CUser&, short int)",
            1957,
            5);
        return static_cast<char>(trace(
            "[@missionSystem] NotMatchIndex: <valid index:%d invalid index:%d>",
            static_cast<int16_t>(missions_[kind].mission_index),
            mission_index));
    }
    alter();
    cleared_[static_cast<uint16_t>(mission_index)] = true;
    missions_[kind].clear();
    mission->Reward(user);
    const char daily = mission->GetMissionGrade() == 2;
    if (!daily) {
        return static_cast<char>(MakeMissionList_forOldUser(user));
    }
    return daily;
}

// IDA_EA: 0x85E65DA
// IDA_NAME: _ZN19CMissionList_Charac13getNewMissionER5CUseri
// DEMANGLED: CMissionList_Charac::getNewMission(CUser &,int)
char CMissionList_Charac::getNewMission(CUser& user, int mission_index) {
    CMission* mission = G_CDataManager()->find_mission(mission_index);
    if (!mission) {
        return 0;
    }
    return ClearMissionKind(
        user, static_cast<short>(mission->GetMissionKind()));
}

// IDA_EA: 0x85E6628
// IDA_NAME: _ZN19CMissionList_Charac16ResetMissionListER5CUser
// DEMANGLED: CMissionList_Charac::ResetMissionList(CUser &)
bool CMissionList_Charac::ResetMissionList(CUser& user) {
    _reset();
    return MakeMissionList_forNewbies(user);
}

// IDA_EA: 0x85E6654
// IDA_NAME: _ZN19CMissionList_Charac25Update_ComboMission_EventER5CUseris
// DEMANGLED: CMissionList_Charac::Update_ComboMission_Event(CUser &,int,short)
void CMissionList_Charac::Update_ComboMission_Event(
    CUser& user, int mission_kind, short position) {
    if (static_cast<uint16_t>(position) > 8u ||
        (mission_kind != 31 && mission_kind != 32)) {
        return;
    }
    short packed_position = position;
    uint16_t packed_value = missions_[mission_kind].current_value;
    if (position > 3) {
        packed_position = static_cast<short>(packed_position - 4);
        packed_value = missions_[mission_kind].target_value;
    }
    const int shift = 4 * (3 - packed_position);
    short value = static_cast<short>((packed_value >> shift) & 0x0f);
    packed_value = static_cast<uint16_t>(
        packed_value - (((packed_value >> shift) & 0x0f) << shift));
    if (--value < 0) {
        value = 0;
    }
    packed_value = static_cast<uint16_t>(packed_value + (value << shift));
    if (position <= 3) {
        missions_[mission_kind].current_value = packed_value;
    } else {
        missions_[mission_kind].target_value = packed_value;
    }
    MissionClearCondition_Parameter parameter(PVP_BATTLE_MODE_1);
    parameter.battle_mode_category = 1;
    std::vector<int> cleared;
    check_mission_kind(mission_kind, user, parameter, cleared);
    if (!cleared.empty()) {
        Send_Cleared_Mission_List(user, cleared);
    }
    alter();
}

// IDA_EA: 0x85E7130
// IDA_NAME: _ZN19CMissionList_Charac27Update_Within_Mission_eventER5CUserRK31MissionClearCondition_ParameterRb
// DEMANGLED: CMissionList_Charac::Update_Within_Mission_event(CUser &,MissionClearCondition_Parameter const&,bool &)
void CMissionList_Charac::Update_Within_Mission_event(
    CUser& user,
    const MissionClearCondition_Parameter& parameter,
    bool& within_mission_cleared) {
    std::vector<int> cleared;
    check_mission_kind(27, user, parameter, cleared);
    if (!cleared.empty()) {
        within_mission_cleared = true;
    }
    Send_Cleared_Mission_List(user, cleared);
}

// IDA_EA: 0x85EC626
// IDA_NAME: _ZN19CMissionList_CharacD2Ev
// DEMANGLED: CMissionList_Charac::~CMissionList_Charac()
// IDA_EA: 0x85EC656
// IDA_NAME: _ZN19CMissionList_CharacD0Ev
// DEMANGLED: CMissionList_Charac::~CMissionList_Charac() deleting destructor
// NOTE: The compiler-generated D0 wrapper calls this D2 body and operator delete.
CMissionList_Charac::~CMissionList_Charac() {}

// IDA_EA: 0x86978AA
// IDA_NAME: _ZNK19CMissionList_Charac27isClearStealingSkillMissionEv
// DEMANGLED: CMissionList_Charac::isClearStealingSkillMission(void)const
int CMissionList_Charac::isClearStealingSkillMission() const {
    return clear_stealing_skill_;
}
