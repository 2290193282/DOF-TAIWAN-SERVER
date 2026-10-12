#include "old_game/pvp/arena/pvp_room.h"

#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85DDF36
// IDA_NAME: _ZN8PvP_Room14revengeMissionEb
// DEMANGLED: PvP_Room::revengeMission(bool)
bool PvP_Room::revengeMission(bool revenge) {
    for (int seat = 0; seat <= 7; ++seat) {
        if (!users_[seat]) continue;
        CMissionList_Charac* const missions =
            reinterpret_cast<CMissionList_Charac*>(
                users_[seat]->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
        missions->Update_Revenge_event(
            *users_[seat], battle_mode_, revenge);
    }
    return false;
}
