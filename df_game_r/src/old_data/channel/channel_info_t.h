#ifndef OLDSRV_OLD_DATA_CHANNEL_CHANNEL_INFO_T_H_
#define OLDSRV_OLD_DATA_CHANNEL_CHANNEL_INFO_T_H_

#include <string>
#include <vector>


// IDA unit: 03_channel_select/channel_info_t. Old i386 size: 0x40.
struct channel_info_t {
    channel_info_t();
    channel_info_t(const channel_info_t& other);
    ~channel_info_t();

    channel_info_t& operator=(const channel_info_t& other);

    std::string channel_name;        // +0x00
    int channel_type;                // +0x04
    std::string dungeon_group_name;  // +0x08
    std::string display_name;        // +0x0C
    std::vector<int>* dungeon_indices;  // +0x10
    unsigned char dungeon_bonus_rate;   // +0x14
    char pad_15[3];                     // +0x15
    float level_grid_percent[10];       // +0x18..+0x3F
};

#endif  // OLDSRV_OLD_DATA_CHANNEL_CHANNEL_INFO_T_H_
