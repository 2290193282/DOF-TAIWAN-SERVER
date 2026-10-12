#include "old_game/player/core/cuser.h"

// IDA_EA: 0x868642E
// IDA_NAME: _ZN15cUserHistoryLog6EndPvPE23ENUM_PVP_HISTORY_REASONbj
// DEMANGLED: cUserHistoryLog::EndPvP(ENUM_PVP_HISTORY_REASON,bool,unsigned int)
int cUserHistoryLog::EndPvP(ENUM_PVP_HISTORY_REASON reason,
                            bool team_result,
                            unsigned int elapsed_seconds) {
    return owner_->LogHistory("PVP,%d,%d,%u",
                              static_cast<int>(reason),
                              team_result,
                              elapsed_seconds);
}

// IDA_EA: 0x868646C
// IDA_NAME: _ZN15cUserHistoryLog9GiveupPvPE23ENUM_PVP_HISTORY_REASON
// DEMANGLED: cUserHistoryLog::GiveupPvP(ENUM_PVP_HISTORY_REASON)
int cUserHistoryLog::GiveupPvP(ENUM_PVP_HISTORY_REASON reason) {
    return owner_->LogHistory("PVP,%d", static_cast<int>(reason));
}

// IDA_EA: 0x8684AC4
// IDA_NAME: _ZN15cUserHistoryLog16DungeonClearInfoEil
// DEMANGLED: cUserHistoryLog::DungeonClearInfo(int,long)
int cUserHistoryLog::DungeonClearInfo(int cleared, long elapsed_seconds) {
    return owner_->LogHistory("DungeonClearInfo,%d,%d",
                              cleared,
                              elapsed_seconds);
}

// IDA_EA: 0x8684A6E
// IDA_NAME: _ZN15cUserHistoryLog16DungeonClearInfoEiliii17ENUM_DUNGEON_MODEPKci
int cUserHistoryLog::DungeonClearInfo(int entrance_count,
                                      long elapsed_seconds,
                                      int stage_index,
                                      int layer,
                                      int clear_state,
                                      ENUM_DUNGEON_MODE dungeon_mode,
                                      const char* member_names,
                                      int character_gage) {
    return owner_->LogHistory(
        "AdvanceDungeonClearInfo,%d,%d,%d,%d,%d,%d,%s,%d",
        stage_index,
        layer,
        entrance_count,
        elapsed_seconds,
        clear_state,
        static_cast<int>(dungeon_mode),
        member_names,
        character_gage);
}

// IDA_EA: 0x86869E8
// IDA_NAME: _ZN15cUserHistoryLog13pvpMissionAddEiiii
// DEMANGLED: cUserHistoryLog::pvpMissionAdd(int,int,int,int)
int cUserHistoryLog::pvpMissionAdd(int mission_kind,
                                   int mission_index,
                                   int current_value,
                                   int target_value) {
    return owner_->LogHistory("PvPMission+,%d,%d,%d,%d",
                              mission_kind,
                              mission_index,
                              current_value,
                              target_value);
}

// IDA_EA: 0x8686A22
// IDA_NAME: _ZN15cUserHistoryLog13pvpMissionDelEii
// DEMANGLED: cUserHistoryLog::pvpMissionDel(int,int)
int cUserHistoryLog::pvpMissionDel(int mission_kind, int mission_index) {
    return owner_->LogHistory(
        "PvPMission-,%d,%d", mission_kind, mission_index);
}

// IDA_EA: 0x8686A4E
// IDA_NAME: _ZN15cUserHistoryLog21pvpMissionClearRewardEiiii
// DEMANGLED: cUserHistoryLog::pvpMissionClearReward(int,int,int,int)
int cUserHistoryLog::pvpMissionClearReward(int pvp_point,
                                           int mission_exp,
                                           int mission_kind,
                                           int mission_index) {
    return owner_->LogHistory("PvPMissionReward+,%d,%d,%d,%d",
                              pvp_point,
                              mission_exp,
                              mission_kind,
                              mission_index);
}
