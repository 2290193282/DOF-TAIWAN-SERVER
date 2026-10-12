#include "old_game/pvp/battlefield/timer_ultimate_next_round_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

// IDA_EA: 0x863C5E6
// IDA_NAME: _ZN27TimerUltimateNextRoundStartC2Ev
TimerUltimateNextRoundStart::TimerUltimateNextRoundStart()
    : TimerSig() {}

// IDA_EA: 0x863F01E (D2), 0x863F04E (D0)
TimerUltimateNextRoundStart::~TimerUltimateNextRoundStart() {}

// IDA_EA: 0x8638D2C
// IDA_NAME: _ZN27TimerUltimateNextRoundStart12dispatch_sigEiij
// DEMANGLED: TimerUltimateNextRoundStart::dispatch_sig(int,int,unsigned int)
int TimerUltimateNextRoundStart::dispatch_sig(int party_index,
                                               int timer_key,
                                               unsigned int param2) {
    (void)param2;
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_ULTIMATE_NEXT_ROUND_START, timer_key)) {
        return 0;
    }

    CBattle_Field& field = party->battle_field();
    CDungeon* const dungeon = field.GetDungeon();
    if (dungeon && dungeon->get_blood_dungeon_type() != 2) {
        return 0;
    }
    if (field.getUltimateDifficylty()) {
        return 0;
    }

    const int difficulty = field.getNextRoundRandomDifficulty();
    field.setUltimateDifficulty(static_cast<char>(difficulty));
    const short blood_round = static_cast<short>(
        field.blood_round().getBloodRoundOrder());
    if (blood_round != field.getBloodMaxRound() - 1) {
        field.sendUltimateRandomDifficulty(difficulty, blood_round);
    }
    field.startUltimateRound();
    return 1;
}
