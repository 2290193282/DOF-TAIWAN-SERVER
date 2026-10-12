#include "old_game/pvp/battlefield/battle_field.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace WongWork {

// IDA_EA: 0x8466208
// IDA_NAME: _ZN8WongWork11CDeathTower23handleSelectEPLPCommandEP5CUseri
// DEMANGLED: WongWork::CDeathTower::handleSelectEPLPCommand(CUser *,int)
int CDeathTower::handleSelectEPLPCommand(CUser* user, int delay_sec) {
    if (!party_) {
        return 0;
    }

    const int seat = party_->GetMemberSlotNo(user);
    if (seat == -1 || party_->checkValidUser(seat) != 1) {
        return 0;
    }

    party_->SetEPLPState(3);
    const int timer_key = _genTimerKey(static_cast<TIMER_MESSAGE>(78));
    G_TimerQueue()->InsertTimer(6,
                                static_cast<int>(index_),
                                78,
                                delay_sec,
                                static_cast<unsigned int>(timer_key),
                                78);
    return 1;
}

}  // namespace WongWork
