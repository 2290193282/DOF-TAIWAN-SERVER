#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

enum ENUM_ERROR : int {};

namespace WongWork {

// IDA_EA: 0x814F66A
// IDA_NAME: _ZN8WongWork10CBossStage13handleMoveMapEPt
// DEMANGLED: WongWork::CBossStage::handleMoveMap(unsigned short *)
int CBossStage::handleMoveMap(unsigned short* map_path) {
    if (_checkMemberDie()) {
        return 0;
    }

    // IDA 0x814F66A reads the physical GlobalData::s_systemTime_ object
    // directly; preserve that original call edge instead of routing it
    // through the later convenience accessor.
    const int elapsed_seconds =
        (GlobalData::s_systemTime_.getCurTickCount() - stage_start_tick_) /
        1000;
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (user && party_->checkValidUser(member_slot) == 1) {
            if (elapsed_seconds <= 4) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user, static_cast<ENUM_HACKTYPE>(0x19f), 1, 0, 0);
            }
            if (!map_path[member_slot]) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user, static_cast<ENUM_HACKTYPE>(0x19b), 1, 0, 0);
            }
            if (!member_alive_[member_slot]) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user, static_cast<ENUM_HACKTYPE>(0xdd), 1, 0, 0);
            }
            user->history_log_.MapClear(map_path[member_slot], elapsed_seconds,
                                        0);
        }
    }

    if (!moveNextStage()) {
        return 0;
    }

    int map_index = 0;
    if (randomMapIndex(map_index) != 1) {
        return 0;
    }

    CUser* const manager = party_->getManager();
    // The original dereferences the party manager immediately after
    // CParty::getManager().  Do not add a null-rejection branch here: it
    // changes the original state-machine contract.
    if (manager->isGMUser()) {
        if (manager->gm_boss_tower_map_index_override_) {
            map_index =
                static_cast<int>(manager->gm_boss_tower_map_index_override_);
            manager->gm_boss_tower_map_index_override_ = 0;
        }
        if (manager->gm_boss_tower_stage_override_) {
            member_count_class_ = manager->gm_boss_tower_stage_override_;
            manager->gm_boss_tower_stage_override_ = 0;
        }
    }

    CMap* const map = G_CDataManager()->find_map(map_index);
    if (!map) {
        return 0;
    }

    reset();
    if (consistMap(map) != 1) {
        throw static_cast<ENUM_ERROR>(1);
    }
    stage_start_tick_ = GlobalData::s_systemTime_.getCurTickCount();
    SendMapInfoPacket();
    return 1;
}

}  // namespace WongWork
