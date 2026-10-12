#include "old_game/pvp/battlefield/battle_field.h"

#include "old_game/player/core/cuser.h"

// IDA_EA: 0x81468AA
// IDA_NAME: _ZN6Global14CBossTowerFunc6DieMobEP5CUserttRK15MSG_MONSTER_DIEPi
// DEMANGLED: Global::CBossTowerFunc::DieMob(CUser *,unsigned short,
//             unsigned short,MSG_MONSTER_DIE const&,int *)
int Global::CBossTowerFunc::DieMob(CUser* user,
                                   unsigned short monster_id,
                                   unsigned short monster_type,
                                   const MSG_MONSTER_DIE& message,
                                   int* result) {
    WongWork::CBossTower* const tower = user ? user->getBossTower() : 0;
    if (tower) {
        tower->handleKillMonster(user,
                                 monster_id,
                                 monster_type,
                                 message,
                                 result);
    }
    return 1;
}
