#include "old_data/channel/channel_dungeon_info_t.h"


typedef char channel_dungeon_info_t_Size_Must_Be_0x10[(sizeof(channel_dungeon_info_t) == 0x10) ? 1 : -1];

// IDA_EA: 0x88DD34A
// IDA_NAME: _ZN22channel_dungeon_info_tC2Ev
// DEMANGLED: channel_dungeon_info_t::channel_dungeon_info_t(void)
channel_dungeon_info_t::channel_dungeon_info_t()
    : dungeon_group_name(), dungeon_indices() {}

// IDA_EA: 0x88DDD78
// IDA_NAME: _ZN22channel_dungeon_info_tC2ERKS_
// DEMANGLED: channel_dungeon_info_t::channel_dungeon_info_t(channel_dungeon_info_t const&)
channel_dungeon_info_t::channel_dungeon_info_t(
    const channel_dungeon_info_t& other)
    : dungeon_group_name(other.dungeon_group_name),
      dungeon_indices(other.dungeon_indices) {}

// IDA_EA: 0x83D8B2C
// IDA_NAME: _ZN22channel_dungeon_info_tD2Ev
// DEMANGLED: channel_dungeon_info_t::~channel_dungeon_info_t()
channel_dungeon_info_t::~channel_dungeon_info_t() {}

// IDA_EA: 0x88DD390
// IDA_NAME: _ZN22channel_dungeon_info_taSERKS_
// DEMANGLED: channel_dungeon_info_t::operator=(channel_dungeon_info_t const&)
channel_dungeon_info_t& channel_dungeon_info_t::operator=(
    const channel_dungeon_info_t& other) {
    dungeon_group_name = other.dungeon_group_name;
    dungeon_indices = other.dungeon_indices;
    return *this;
}
