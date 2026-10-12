#include "old_game/activity/schedule/bingo_runtime.h"

#include "core/config/env/environment.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x80C9BB8
// IDA_NAME: _ZN10BingoEvent11StartActionEv
// DEMANGLED: BingoEvent::StartAction(void)
void BingoEvent::StartAction() {
    CGameManager::UserMap& users = G_CGameManager()->users_by_account_;
    for (CGameManager::UserMap::iterator it = users.begin();
         it != users.end();
         ++it) {
        if (it->second) {
            it->second->sendBingoData();
        }
    }
}

// IDA_EA: 0x80C9D82
// IDA_NAME: _ZN10BingoEvent14dailyresetDataEv
// DEMANGLED: BingoEvent::dailyresetData(void)
void BingoEvent::dailyresetData() {
    const int reset_hour = G_CEnvironment()->get_fatigue_time();
    const std::time_t current_time =
        static_cast<std::time_t>(GlobalData::system_time()->getCurSec());
    const int today = bingo_->getToday(current_time, reset_hour);
    if (today > 0 && today <= 31) {
        bingo_->resetBoard(today);
        bingo_->resetQuiz(today);
        cMyTrace trace("virtual void BingoEvent::dailyresetData()", 80, 0);
        trace("BingoEvent Day(%d)", today);
    }

    CGameManager::UserMap& users = G_CGameManager()->users_by_account_;
    for (CGameManager::UserMap::iterator it = users.begin();
         it != users.end();
         ++it) {
        CUser* const user = it->second;
        if (user) {
            user->getBingoData()->clear();
            user->sendBingoData();
            user->sendDecorationObjectCnt(0);
        }
    }
}
