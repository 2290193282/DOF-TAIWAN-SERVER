#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x822CEFC
// IDA_NAME: _ZN13CBattle_Field11getMapWidthEv
// DEMANGLED: CBattle_Field::getMapWidth(void)
int CBattle_Field::getMapWidth() {
    return map_width_;
}

// IDA_EA: 0x822CF0A
// IDA_NAME: _ZN13CBattle_Field12getMapHeightEv
// DEMANGLED: CBattle_Field::getMapHeight(void)
int CBattle_Field::getMapHeight() {
    return map_height_;
}

// IDA_EA: 0x8300DA6
// IDA_NAME: _ZN13CBattle_Field11getCurPosXYERiS0_
// DEMANGLED: CBattle_Field::getCurPosXY(int &,int &)
int* CBattle_Field::getCurPosXY(int& pos_x, int& pos_y) {
    CDungeon* const dungeon = get_dungeon();
    const unsigned int seat = static_cast<unsigned int>(current_seat_no_);
    if (dungeon && dungeon->IsEnterEachMap() &&
        seat < 4u) {
        pos_x = start_positions_[seat].first;
        pos_y = start_positions_[seat].second;
    } else {
        pos_x = current_position_.first;
        pos_y = current_position_.second;
    }
    return &pos_y;
}

// IDA_EA: 0x822CF50
// IDA_NAME: _ZN13CBattle_Field17check_start_pointEv
// DEMANGLED: CBattle_Field::check_start_point(void)
bool CBattle_Field::check_start_point() {
    int cur_x = -1;
    int cur_y = -1;
    getCurPosXY(cur_x, cur_y);
    return maze_start_position_.first == cur_x &&
           maze_start_position_.second == cur_y;
}
