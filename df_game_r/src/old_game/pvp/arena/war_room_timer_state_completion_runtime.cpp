#include "old_game/pvp/battlefield/war_room.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x86BC4EC
// IDA_NAME: _ZN7WarRoom13CheckFatuigueEv
// DEMANGLED: WarRoom::CheckFatuigue(void)
bool WarRoom::CheckFatuigue() {
    for (int slot = 0;; ++slot) {
        const bool result = slot <= 5;
        if (!result) {
            return result;
        }

        CUser* const user = war_field_.room_users_[slot];
        if (!user) {
            continue;
        }

        if (!user->CheckFatigue()) {
            WalkOutUserBySlot(slot);
            SendWalkOut(user, static_cast<ENUM_WALKOUT_TYPE>(5));
            continue;
        }

        // 0x86BC583..0x86BC5A4 reads CDataManager+(0x8874 + 4*room_mode_).
        // The B-owned typed accessor exposes that exact WarArea.etc table.
        if (user->getCurCharacMoney() <
            G_CDataManager()->get_war_area_counter()->door_fee[room_mode_]) {
            WalkOutUserBySlot(slot);
            SendWalkOut(user, static_cast<ENUM_WALKOUT_TYPE>(8));
        }
    }
}

// IDA_EA: 0x86C0404
// IDA_NAME: _ZN7WarRoom22HandleTimerResultCountEv
// DEMANGLED: WarRoom::HandleTimerResultCount(void)
int WarRoom::HandleTimerResultCount() {
    ReviveAll();
    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (user && war_field_.walkout_types_[slot]) {
            AddBanList(user);
            WalkOutUserBySlot(slot);
            SendWalkOut(user, war_field_.walkout_types_[slot]);
        }
    }

    BattleReset();
    CheckFatuigue();
    return ChangeState(static_cast<WARROOM_STATE>(
        GetWaiterCount() > 2 ? 1 : 0));
}

// IDA_EA: 0x86C0512
// IDA_NAME: _ZN7WarRoom19HandleTimerTimeBombEv
// DEMANGLED: WarRoom::HandleTimerTimeBomb(void)
int WarRoom::HandleTimerTimeBomb() {
    war_field_.map_info_.monsters_.clear();
    result_phase_active_ = 1;
    if (war_field_.current_spawn_group_index_ ==
        war_field_.dungeon_->war_room_data_.timing_values_[0]) {
        return ChangeState(static_cast<WARROOM_STATE>(5));
    }
    return ChangeState(static_cast<WARROOM_STATE>(4));
}

// IDA_EA: 0x86C078E
// IDA_NAME: _ZN7WarRoom18SendWarRoomFailMsgEv
// DEMANGLED: WarRoom::SendWarRoomFailMsg(void)
int WarRoom::SendWarRoomFailMsg() {
    const int timer_key = GenTimerKey(static_cast<TIMER_MESSAGE>(70));
    return G_TimerQueue()->InsertTimer(
        3, room_index_, 70, 10, static_cast<unsigned int>(timer_key), 0);
}

// IDA_EA: 0x86C07EE
// IDA_NAME: _ZN7WarRoom11WarRoomFailEv
// DEMANGLED: WarRoom::WarRoomFail(void)
char WarRoom::WarRoomFail() {
    const char result = GetLiveCount() != 0;
    if (result) {
        return result;
    }

    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (!user) {
            continue;
        }

        AddBanList(user);
        WalkOutUserBySlot(slot);
        SendWalkOut(user, static_cast<ENUM_WALKOUT_TYPE>(4));
    }
    return static_cast<char>(ChangeState(static_cast<WARROOM_STATE>(0)));
}
