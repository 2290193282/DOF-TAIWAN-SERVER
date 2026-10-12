#include "common/manager/game_manager.h"

#include "common/manager/data_manager.h"
#include "old_data/PVF/event/blue_marble_script.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"

// IDA_EA: 0x82A22B6
// IDA_NAME: _ZN12CGameManager13getBlueMarbleEv
// DEMANGLED: CGameManager::getBlueMarble(void)
BlueMarble* CGameManager::getBlueMarble() {
    BlueMarble* const blue_marble = blue_marble_pool_.Acquire();
    if (!blue_marble) {
        return 0;
    }

    blue_marble->setMemoryPoolIndex(
        blue_marble_pool_.GetIndex(blue_marble));
    const int pool_index = blue_marble->getMemoryPoolIndex();
    if (blue_marbles_.find(pool_index) != blue_marbles_.end()) {
        // The original returns without returning this acquired node to the
        // pool when the index already exists.
        return 0;
    }

    blue_marbles_[blue_marble->getMemoryPoolIndex()] = blue_marble;
    return blue_marble;
}

// IDA_EA: 0x82A3396
// IDA_NAME: _ZN12CGameManager15allocBlueMarbleEv
// DEMANGLED: CGameManager::allocBlueMarble(void)
bool CGameManager::allocBlueMarble() {
    BlueMarbleScriptManager* const script =
        G_CDataManager()->getBlueMarbleScript();

    for (int allocation_index = 0; allocation_index <= 299;
         ++allocation_index) {
        BlueMarble* const blue_marble = getBlueMarble();
        blue_marble->getMemoryPoolIndex();

        for (int user_index = 0;
             script->info_script.player_max > user_index;
             ++user_index) {
            blue_marble->setUserInfo(script);
        }
        blue_marble->setScript(script);
    }

    // The terminating `allocation_index <= 299` test leaves AL clear in the
    // old body, so its observed boolean return is false.
    return false;
}
