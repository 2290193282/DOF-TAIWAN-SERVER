#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "old_data/channel/channel_script_t.h"

#include <stdint.h>

// IDA_EA: 0x8298940
// IDA_NAME: _ZN12CGameManager31WarRoomCountAdjustByChannelInfoEv
// DEMANGLED: CGameManager::WarRoomCountAdjustByChannelInfo(void)
bool CGameManager::WarRoomCountAdjustByChannelInfo() {
    channel_script_t* const channel_script = G_CDataManager()->GetChannelScript();
    CEnvironment* const environment = G_CEnvironment();
    const channel_info_t* const channel_info = channel_script->getChannelInfo(
        static_cast<uint8_t>(environment->get_server_group()),
        static_cast<uint32_t>(environment->get_channel_no()));

    for (int level_grid = 0; level_grid <= 9; ++level_grid) {
        for (int time_table = 0; time_table <= 23; ++time_table) {
            WarAreaCounter* const war_area_counter =
                G_CDataManager()->get_war_area_counter();
            const float level_grid_percent =
                channel_info->level_grid_percent[level_grid];
            const int base_room_count =
                G_CDataManager()->get_war_area_counter()->counter[time_table];

            // The old x87 sequence changes its rounding mode to truncation
            // before FISTP. A C++ conversion to int has the same direction.
            war_area_counter->war_room_count[time_table][level_grid] =
                static_cast<int>(
                    static_cast<long double>(base_room_count) *
                    static_cast<long double>(level_grid_percent));
        }
    }

    // At the raw return site AL holds the failed `level_grid <= 9` test.
    return false;
}

// IDA_EA: 0x82A24E8
// IDA_NAME: _ZN12CGameManager10GetWarRoomEv
// DEMANGLED: CGameManager::GetWarRoom(void)
WarRoom* CGameManager::GetWarRoom() {
    WarRoom* const room = war_room_pool_.Acquire();
    if (!room) {
        return 0;
    }

    room->SetIndex(war_room_pool_.GetIndex(room));
    const int room_index = room->GetIndex();
    if (war_rooms_.find(room_index) != war_rooms_.end()) {
        // IDA leaves the acquired pool node in this state on duplicate key.
        return 0;
    }

    war_rooms_[room->GetIndex()] = room;
    return room;
}

// IDA_EA: 0x8298A30
// IDA_NAME: _ZN12CGameManager12WarRoomAllocEv
// DEMANGLED: CGameManager::WarRoomAlloc(void)
void CGameManager::WarRoomAlloc() {
    WarAreaCounter* const war_area_counter =
        G_CDataManager()->get_war_area_counter();
    const int current_time_table = war_area_counter->GetCurrenTimeTable();

    for (int room_mode = 0; room_mode <= 9; ++room_mode) {
        int room_level = 0;
        for (int count_index = 0;; ++count_index) {
            if (war_area_counter->GetWarRoomCountAtPeekTime(room_mode) <=
                count_index) {
                break;
            }

            // The original dereferences this result immediately: capacity is
            // a prerequisite supplied by the 40-entry StaticPool, not a
            // recoverable allocation fallback in this startup path.
            WarRoom* const room = GetWarRoom();
            room->room_mode_ = static_cast<uint8_t>(room_mode);
            room->room_level_ = static_cast<uint8_t>(room_level++);
            room->Prepare();

            if (war_area_counter->war_room_count[current_time_table][room_mode] <=
                count_index) {
                room->SetState(static_cast<WARROOM_STATE>(-1));
            } else {
                room->SetState(static_cast<WARROOM_STATE>(0));
            }
        }
    }

    PrintWarRoomList();
}

// IDA_EA: 0x8298B40
// IDA_NAME: _ZN12CGameManager16PrintWarRoomListEv
// DEMANGLED: CGameManager::PrintWarRoomList(void)
void CGameManager::PrintWarRoomList() {
    // Old body is only `push ebp; mov ebp, esp; sub esp, 0x50; leave; ret`.
}
