#include "old_game/player/core/cuser.h"

// IDA_EA: 0x86851AC
// IDA_NAME: _ZN15cUserHistoryLog11KillMonsterEijjttRK15MSG_MONSTER_DIEcbii13eExpAddReason
// DEMANGLED: cUserHistoryLog::KillMonster(int,unsigned int,unsigned int,
//             unsigned short,unsigned short,MSG_MONSTER_DIE const&,char,bool,
//             int,int,eExpAddReason)
int cUserHistoryLog::KillMonster(
    int monster_id,
    unsigned int message_value_a1e,
    unsigned int message_value_a1a,
    unsigned short message_value_a24,
    unsigned short message_value_a26,
    const MSG_MONSTER_DIE& message,
    char monster_kind,
    bool is_apc,
    int current_exp,
    int gained_exp,
    eExpAddReason reason) {
    return owner_->LogHistory(
        "KillMob,%d,%d,%d, (%d:%d),%d, (%d:%d),%d,%d,%d, "
        "(%d:%d),%d,%d,%d,%d,%d,%d,%d",
        monster_id,
        message_value_a1e,
        message_value_a1a,
        message_value_a24,
        message_value_a26,
        message.field_a22_,
        message.field_a29_,
        message.field_a2b_,
        message.field_a31_,
        message.field_a2d_,
        monster_kind,
        message.field_a35_,
        message.field_a37_,
        message.field_a39_,
        message.field_a3b_,
        message.field_a3d_,
        is_apc,
        current_exp,
        gained_exp,
        static_cast<int>(reason));
}
