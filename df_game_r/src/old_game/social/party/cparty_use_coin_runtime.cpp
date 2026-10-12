#include "old_game/social/party/party.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/history_log_abi.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85B27C8
// IDA_NAME: _ZN6CParty10ReviveUserEP5CUsert
// DEMANGLED: CParty::ReviveUser(CUser *,unsigned short)
int CParty::ReviveUser(CUser* user, uint16_t target_unique_id) {
    if ((user &&
         zombie_mode::IsRevivalBlocked(user->GetPVPRoom())) ||
        (manager_ &&
         zombie_mode::IsRevivalBlocked(manager_->GetPVPRoom()))) {
        return 17;
    }
    if (get_state() != 2 && !GetEPLPState()) {
        return 19;
    }

    CUser* target = 0;
    int target_seat = -1;
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = get_user(seat);
        if (_checkValidUser(seat) == 1 && member &&
            member->get_unique_id() == target_unique_id) {
            if (battle_data_.member_live(seat)) {
                return 18;
            }
            target = member;
            target_seat = seat;
            break;
        }
    }
    if (!target) {
        return 21;
    }

    if (!checkFreeRevivalCondition(0)) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (_checkValidUser(seat) != 1 || get_user(seat) != user) {
                continue;
            }
            int& remaining = battle_data_.member_coin_limit_remaining(seat);
            if (!remaining) {
                return 1;
            }
            HistoryLog::WriteUseCoin(user->history_stream(),
                                     target->getCurCharacName());
            --remaining;
        }
    }

    // target_seat is deliberately retained from the target lookup: this is
    // the member whose life flag changes, not necessarily the payer's seat.
    (void)target_seat;
    set_charac_live(target, static_cast<ENUM_USER_DUNGEON_STATE>(1));
    Reset_party_overlapped_drop_ratio();
    return 0;
}

// IDA_EA: 0x85BE4AE
// IDA_NAME: _ZN6CParty29checkUnLimitUsedCoinConditionEPK8CDungeon
// DEMANGLED: CParty::checkUnLimitUsedCoinCondition(CDungeon const*)
int CParty::checkUnLimitUsedCoinCondition(const CDungeon* dungeon) {
    if (!dungeon) {
        dungeon = G_CDataManager()->find_dungeon(battle_field_.get_dungeon_index());
        if (!dungeon) {
            return 0;
        }
    }
    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(94);
    return event->IsEventing(0) && dungeon->get_dimension_possible() > 0;
}

// IDA_EA: 0x85BE56C
// IDA_NAME: _ZN6CParty27checkLimitUsedCoinConditionEPK8CDungeon
// DEMANGLED: CParty::checkLimitUsedCoinCondition(CDungeon const*)
int CParty::checkLimitUsedCoinCondition(const CDungeon* dungeon) {
    if (!dungeon) {
        dungeon = G_CDataManager()->find_dungeon(battle_field_.get_dungeon_index());
        if (!dungeon) {
            return 19;
        }
    }

    if (GetPartyMemberCoinLimit() &&
        GetUsedCoinCount() >= GetPartyMemberCoinLimit()) {
        return 22;
    }

    int coin_limit = 0;
    if (dungeon->getLimitCoinDiff(battle_field_.get_dungeon_diff(),
                                  coin_limit) &&
        coin_limit != -1 && GetUsedCoinCount() >= coin_limit) {
        return 22;
    }
    if (dungeon->coin_limit_ != -1 &&
        GetUsedCoinCount() >= dungeon->coin_limit_) {
        return 22;
    }
    return dungeon->script_value_898_ == -1 ||
                   CheckHellCoinLimint(dungeon->script_value_898_)
               ? 0
               : 22;
}
