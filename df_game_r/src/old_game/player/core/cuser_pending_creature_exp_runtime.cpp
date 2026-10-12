#include "old_game/player/core/cuser.h"

// IDA provenance: CParty::ClearDungeon @ 0x85AB830 and
// WarRoom::HandleMonsterKill @ 0x86BE1FE both consume this exact CUser
// field, apply it through CCreatureMgr::GainExp when appropriate, then set it
// to zero.  The field has no separately exported accessor in the old binary.
int CUser::takePendingCreatureExp() {
    const int pending_exp = creature_exp_fatigue_;
    creature_exp_fatigue_ = 0;
    return pending_exp;
}
