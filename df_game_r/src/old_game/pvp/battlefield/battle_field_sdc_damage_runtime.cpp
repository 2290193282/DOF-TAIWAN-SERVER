#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x8308F4E
// IDA_NAME: _ZN13CBattle_Field19SetDamagedPacketCntEi
// DEMANGLED: CBattle_Field::SetDamagedPacketCnt(int)
int CBattle_Field::SetDamagedPacketCnt(int monster_unique_id) {
    MapInfo* const map_info = GetCurrentMapInfo();
    std::map<int, map_monster>::iterator monster =
        map_info->monsters_.find(monster_unique_id);
    if (monster == map_info->monsters_.end()) {
        return 0;
    }

    ++monster->second.sdc_damaged_packet_count_;
    return 1;
}

// IDA_EA: 0x8308FF8
// IDA_NAME: _ZN13CBattle_Field14SetDamageValueEii
// DEMANGLED: CBattle_Field::SetDamageValue(int,int)
int CBattle_Field::SetDamageValue(int monster_unique_id, int damage_value) {
    MapInfo* const map_info = GetCurrentMapInfo();
    std::map<int, map_monster>::iterator monster =
        map_info->monsters_.find(monster_unique_id);
    if (monster == map_info->monsters_.end()) {
        return 0;
    }

    monster->second.sdc_damage_value_ += damage_value;
    return 1;
}
