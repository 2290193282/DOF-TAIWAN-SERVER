#include "old_game/pvp/match/league_match_static_state_runtime.h"

#include "old_game/pvp/season/pvp_rating_abi.h"

// IDA_EA: 0x93ABB74
// IDA_NAME: _ZN12CLeagueMatch23nPlayerCountPerOneTeam_E
uint8_t CLeagueMatch::nPlayerCountPerOneTeam_ = 2;

// IDA_EA: 0x93ABB75
// IDA_NAME: _ZN12CLeagueMatch12nBattleMode_E
uint8_t CLeagueMatch::nBattleMode_ = 3;

// IDA_EA: 0x943E060
// IDA_NAME: _ZN12CLeagueMatch11bMatchable_E
bool CLeagueMatch::bMatchable_ = false;

// IDA_EA: 0x943E064
// IDA_NAME: _ZN12CLeagueMatch17pWaitingRoomList_E
// The target resides in .bss; the reconstructed static starts as a null
// CLeagueMatchRoomList pointer.
CLeagueMatchRoomList* CLeagueMatch::pWaitingRoomList_ = 0;

// IDA_EA: 0x943E068
// IDA_NAME: _ZN12CLeagueMatch25nWaitingRoomListRefCount_E
// The target resides in .bss and starts at zero.
int CLeagueMatch::nWaitingRoomListRefCount_ = 0;

// IDA_EA: 0x943E06C
// IDA_NAME: _ZN12CLeagueMatch14nextAuthValue_E
// The target resides in .bss and starts at zero.
int CLeagueMatch::nextAuthValue_ = 0;

// IDA_EA: 0x93ABB60
// IDA_NAME: _ZN28PvPGlobalEnvironmentVariable25school_match_daily_point_E
// DEMANGLED: PvPGlobalEnvironmentVariable::school_match_daily_point_
int PvPGlobalEnvironmentVariable::school_match_daily_point_ = 1;

// IDA_EA: 0x93ABB64
// IDA_NAME: _ZN28PvPGlobalEnvironmentVariable34school_match_daily_point_weekends_E
// DEMANGLED: PvPGlobalEnvironmentVariable::school_match_daily_point_weekends_
int PvPGlobalEnvironmentVariable::school_match_daily_point_weekends_ = 2;

// IDA_EA: 0x93ABB68
// IDA_NAME: _ZN28PvPGlobalEnvironmentVariable40school_match_fight_count_for_gain_point_E
// DEMANGLED: PvPGlobalEnvironmentVariable::school_match_fight_count_for_gain_point_
int PvPGlobalEnvironmentVariable::school_match_fight_count_for_gain_point_ = 5;

// IDA_EA: 0x93ABB6C
// IDA_NAME: _ZN28PvPGlobalEnvironmentVariable19school_match_elo_k_E
// DEMANGLED: PvPGlobalEnvironmentVariable::school_match_elo_k_
int PvPGlobalEnvironmentVariable::school_match_elo_k_ = 3;

// IDA_EA: 0x93ABB70
// IDA_NAME: _ZN28PvPGlobalEnvironmentVariable20school_match_elo_k2_E
// DEMANGLED: PvPGlobalEnvironmentVariable::school_match_elo_k2_
int PvPGlobalEnvironmentVariable::school_match_elo_k2_ = 3;

// IDA_EA: 0x84ED37B
// IDA_NAME: _ZN12CLeagueMatch22GetPlayCountPerOneTeamEv
// DEMANGLED: CLeagueMatch::GetPlayCountPerOneTeam(void)
int CLeagueMatch::GetPlayCountPerOneTeam() {
    return static_cast<int>(nPlayerCountPerOneTeam_);
}

// IDA_EA: 0x84ED387
// IDA_NAME: _ZN12CLeagueMatch13GetBattleModeEv
// DEMANGLED: CLeagueMatch::GetBattleMode(void)
int CLeagueMatch::GetBattleMode() {
    return static_cast<int>(nBattleMode_);
}

// IDA_EA: 0x855D070
// IDA_NAME: _ZN12CLeagueMatch22SetPlayCountPerOneTeamEj
// DEMANGLED: CLeagueMatch::SetPlayCountPerOneTeam(unsigned int)
void CLeagueMatch::SetPlayCountPerOneTeam(uint32_t player_count_per_team) {
    // Exact old body: store only AL to the static byte.
    nPlayerCountPerOneTeam_ = static_cast<uint8_t>(player_count_per_team);
}

// IDA_EA: 0x855D07E
// IDA_NAME: _ZN12CLeagueMatch12SetMatchAbleEb
// DEMANGLED: CLeagueMatch::SetMatchAble(bool)
int CLeagueMatch::SetMatchAble(bool matchable) {
    // Exact old body: store AL in the one-byte static, then return MOVZX AL.
    bMatchable_ = matchable;
    return static_cast<int>(matchable);
}

// IDA_EA: 0x82743A0
// IDA_NAME: _ZN12CLeagueMatch13SetBattleModeEi
// DEMANGLED: CLeagueMatch::SetBattleMode(int)
void CLeagueMatch::SetBattleMode(int battle_mode) {
    // Exact old body: store only AL to the static byte.
    nBattleMode_ = static_cast<uint8_t>(battle_mode);
}
