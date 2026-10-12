#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_data/PVF/etc/time_spiral_script.h"

// IDA_EA: 0x85DE15A
// IDA_NAME: _ZN15CRelayBattleMgr12OnChangeTeamEii
// DEMANGLED: CRelayBattleMgr::OnChangeTeam(int,int)
void CRelayBattleMgr::OnChangeTeam(int seat, int team) {
    if (seat >= 0 && seat <= 7) {
        battles[seat].team = static_cast<int8_t>(team);
    }
}

// IDA_EA: 0x860E1EE
// IDA_NAME: _ZN19CDungeonEntranceLog24IncrementDungeonEntranceEib
// DEMANGLED: CDungeonEntranceLog::IncrementDungeonEntrance(int,bool)
CDungeonEntranceLog::Entry*
CDungeonEntranceLog::IncrementDungeonEntrance(int dungeon_index, bool tower) {
    Base::iterator it = find(dungeon_index);
    if (it != end()) {
        if (tower) {
            ++it->second.tower_entrance_total;
        } else {
            ++it->second.normal_entrance_total;
            ++it->second.normal_entrance_hour;
            ++it->second.normal_active_count;
        }
        return &*it;
    }

    STDungeonInoutCnt count;
    if (tower) {
        ++count.tower_entrance_total;
    } else {
        ++count.normal_entrance_total;
        ++count.normal_entrance_hour;
        ++count.normal_active_count;
    }
    return &*insert(std::make_pair(dungeon_index, count)).first;
}

// IDA_EA: 0x822CFA6
// IDA_NAME: _ZN13CBattle_Field15check_end_pointEv
// DEMANGLED: CBattle_Field::check_end_point(void)
bool CBattle_Field::check_end_point() {
    int pos_x = -1;
    int pos_y = -1;
    getCurPosXY(pos_x, pos_y);
    return time_spiral::IsTerminalCoordinate(
               get_dungeon_index(), pos_x, pos_y) ||
           (maze_boss_position_.first == pos_x &&
            maze_boss_position_.second == pos_y);
}

// IDA_EA: 0x86BABCC
// IDA_NAME: _ZNK7WarRoom14GetWaiterCountEv
// DEMANGLED: WarRoom::GetWaiterCount(void)const
int WarRoom::GetWaiterCount() const {
    int count = 0;
    for (int slot = 0; slot <= 5; ++slot) {
        if (war_field_.room_users_[slot]) {
            ++count;
        }
    }
    return count;
}
