#include "old_game/pvp/battlefield/war_room.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/history_log_abi.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x86BF324
// IDA_NAME: _ZN7WarRoom16ReviveUserByCoinEP5CUsert
// DEMANGLED: WarRoom::ReviveUserByCoin(CUser *,unsigned short)
int WarRoom::ReviveUserByCoin(CUser* user, uint16_t target_unique_id) {
    if (GetState() != 3 && GetState() != 4) {
        return 19;
    }

    CUser* target = 0;
    int target_slot = -1;
    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const member = war_field_.room_users_[slot];
        if (member && member->get_unique_id() == target_unique_id) {
            if (war_field_.live_flags_[slot]) {
                return 18;
            }
            target = member;
            target_slot = slot;
            break;
        }
    }
    if (!target) {
        return 21;
    }
    if (user->GetUsedCoinCount() >= war_field_.dungeon_->coin_limit_) {
        return 22;
    }

    HistoryLog::WriteUseCoin(user->history_stream(), target->getCurCharacName());
    SetCharacterLive(target_slot, target, true, true, 0, 0xffu);
    return 0;
}
