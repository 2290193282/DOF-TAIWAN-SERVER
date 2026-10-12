#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_CHARACTER_MISSION_LIST_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_CHARACTER_MISSION_LIST_H_

#include "old_data/PVF/pvp_mission/datamanager/pvp_mission_system.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/player/core/charac_expand_cdata.h"

#include <bitset>
#include <vector>

#include <stdint.h>

class CMissionList_Charac : public charac_expand::CData {
public:
    CMissionList_Charac();
    virtual ~CMissionList_Charac();

    virtual bool loadData(CUser* user, char* data);
    virtual void getData(char* data) const;
    virtual void ResetDaily();

    bool isUserNewbie() const;
    int getIndex_byKind(int mission_kind) const;
    charac_expand::CData* Remove_Mission(int mission_kind);
    int addNewMission(const MissionInfo& info);
    bool MakeMissionList_forNewbies(const CUser& user);
    bool MakeMissionList_forOldUser(const CUser& user);
    int MakeMissionList_JustKind(const CUser& user,
                                 unsigned int mission_kind);
    bool isClearMission(int mission_index) const;
    int getWinningRate() const;
    int getIssue_mode(bool battle_mode) const;
    int getIssue_all() const;
    CMissionList_Charac* setIssue(bool result, bool battle_mode);
    int Send_MissionList(CUser& user);
    void Reset_DailyMission();
    charac_expand::CData* GM_SetMissionValue(int mission_kind, int value);
    int empty_progress() const;
    void Send_Cleared_Mission_List(
        CUser& user, const std::vector<int>& mission_indices);
    void Send_Fail_Mission(CUser& user, short mission_index);
    void Update_Kill_event(
        CUser& user,
        const MissionClearCondition_Parameter& parameter);
    void Update_Win_event(
        CUser& user,
        const MissionClearCondition_Parameter& parameter,
        bool& within_mission_cleared);
    char Update_RecvPacket_event(CUser& user,
                                 std::vector<short>& mission_indices,
                                 std::vector<int>& cleared_missions);
    void Update_GameOver_event(
        CUser& user,
        const MissionClearCondition_Parameter& parameter);
    void Update_EnterWorld_event(CUser& user);
    void Update_Revenge_event(CUser& user,
                              PVP_BATTLE_MODE battle_mode,
                              bool revenge);
    void check_mission_kind(
        unsigned int mission_kind,
        CUser& user,
        const MissionClearCondition_Parameter& parameter,
        std::vector<int>& cleared_missions);
    char ClearMissionKind(CUser& user, short mission_kind);
    char ClearMission(CUser& user, short mission_index);
    char getNewMission(CUser& user, int mission_index);
    bool ResetMissionList(CUser& user);
    void Update_ComboMission_Event(CUser& user,
                                   int mission_kind,
                                   short position);
    void Update_Within_Mission_event(
        CUser& user,
        const MissionClearCondition_Parameter& parameter,
        bool& within_mission_cleared);
    int isClearStealingSkillMission() const;

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    MissionInfo missions_[36];       // +0x005
    std::bitset<256> cleared_;       // +0x128
    uint16_t issue_mode_win_;        // +0x148
    uint16_t issue_mode_loss_;       // +0x14a
    uint16_t issue_all_;             // +0x14c
    uint8_t clear_stealing_skill_;   // +0x14e
    uint8_t pad_14f_;                // +0x14f
    uint32_t charac_no_;             // +0x150
    CUser* owner_;                   // +0x154
};

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_CHARACTER_MISSION_LIST_H_
