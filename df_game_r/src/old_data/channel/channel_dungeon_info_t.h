#ifndef OLDSRV_OLD_DATA_CHANNEL_CHANNEL_DUNGEON_INFO_T_H_
#define OLDSRV_OLD_DATA_CHANNEL_CHANNEL_DUNGEON_INFO_T_H_

#include <string>
#include <vector>


// IDA unit: 03_channel_select/channel_dungeon_info_t. Old i386 size: 0x10.
struct channel_dungeon_info_t {
    channel_dungeon_info_t();
    channel_dungeon_info_t(const channel_dungeon_info_t& other);
    ~channel_dungeon_info_t();

    channel_dungeon_info_t& operator=(const channel_dungeon_info_t& other);

    std::string dungeon_group_name;  // +0x00
    std::vector<int> dungeon_indices;  // +0x04
};

#endif  // OLDSRV_OLD_DATA_CHANNEL_CHANNEL_DUNGEON_INFO_T_H_
