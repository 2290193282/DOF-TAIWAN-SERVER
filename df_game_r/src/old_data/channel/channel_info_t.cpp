#include "old_data/channel/channel_info_t.h"

#include <cstring>


typedef char channel_info_t_Size_Must_Be_0x40[(sizeof(channel_info_t) == 0x40) ? 1 : -1];

// IDA_EA: 0x88DD2A8
// IDA_NAME: _ZN14channel_info_tC2Ev
// DEMANGLED: channel_info_t::channel_info_t(void)
channel_info_t::channel_info_t()
    : channel_name(),
      channel_type(0),
      dungeon_group_name(),
      display_name(),
      dungeon_indices(0),
      dungeon_bonus_rate(0),
      pad_15() {
    std::memset(level_grid_percent, 0, sizeof(level_grid_percent));
}

// IDA_EA: 0x88DE0C8
// IDA_NAME: _ZN14channel_info_tC2ERKS_
// DEMANGLED: channel_info_t::channel_info_t(channel_info_t const&)
channel_info_t::channel_info_t(const channel_info_t& other)
    : channel_name(other.channel_name),
      channel_type(other.channel_type),
      dungeon_group_name(other.dungeon_group_name),
      display_name(other.display_name),
      dungeon_indices(other.dungeon_indices),
      dungeon_bonus_rate(other.dungeon_bonus_rate),
      pad_15() {
    std::memcpy(pad_15, other.pad_15, sizeof(pad_15));
    for (int i = 0; i < 10; ++i) {
        level_grid_percent[i] = other.level_grid_percent[i];
    }
}

// IDA_EA: 0x83F11DA
// IDA_NAME: _ZN14channel_info_tD2Ev
// DEMANGLED: channel_info_t::~channel_info_t()
channel_info_t::~channel_info_t() {}

// IDA_EA: 0x88DD3C6
// IDA_NAME: _ZN14channel_info_taSERKS_
// DEMANGLED: channel_info_t::operator=(channel_info_t const&)
channel_info_t& channel_info_t::operator=(const channel_info_t& other) {
    channel_name = other.channel_name;
    channel_type = other.channel_type;
    dungeon_group_name = other.dungeon_group_name;
    display_name = other.display_name;
    dungeon_indices = other.dungeon_indices;
    dungeon_bonus_rate = other.dungeon_bonus_rate;
    std::memcpy(pad_15, other.pad_15, sizeof(pad_15));
    for (int i = 0; i < 10; ++i) {
        level_grid_percent[i] = other.level_grid_percent[i];
    }
    return *this;
}
