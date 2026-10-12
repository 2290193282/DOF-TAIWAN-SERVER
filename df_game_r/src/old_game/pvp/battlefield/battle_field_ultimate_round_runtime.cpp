#include "old_game/pvp/battlefield/battle_field.h"

#include "common/manager/server_utility.h"

// IDA_EA: 0x8306156
// IDA_NAME: _ZN13CBattle_Field28getNextRoundRandomDifficultyEv
// DEMANGLED: CBattle_Field::getNextRoundRandomDifficulty(void)
int CBattle_Field::getNextRoundRandomDifficulty() {
    const short blood_round = static_cast<short>(
        blood_round_.getBloodRoundOrder());
    if (blood_round == getBloodMaxRound() - 1) {
        return 1;
    }
    if (static_cast<int>(get_rand_int(100)) <= 49) {
        return 2;
    }
    return 1;
}
