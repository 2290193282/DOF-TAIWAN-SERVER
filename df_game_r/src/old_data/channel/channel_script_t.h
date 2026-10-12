#ifndef OLDSRV_OLD_DATA_CHANNEL_CHANNEL_SCRIPT_T_H_
#define OLDSRV_OLD_DATA_CHANNEL_CHANNEL_SCRIPT_T_H_

#include "old_data/channel/channel_dungeon_info_t.h"
#include "old_data/channel/channel_info_t.h"

#include <map>
#include <stdint.h>
#include <string>


// IDA unit: 03_channel_select/channel_script_t. Old i386 size: 0x30.
struct channel_script_t {
    typedef std::map<std::string, channel_dungeon_info_t> DungeonInfoMap;
    typedef std::map<int, channel_info_t> ChannelInfoMap;
    typedef std::map<int, ChannelInfoMap> ServerChannelInfoMap;

    channel_script_t();
    ~channel_script_t();

    int GetPvPChannelNumber(uint8_t server_group,
                            uint32_t channel_no) const;
    const channel_info_t* getChannelInfo(uint8_t server_group,
                                         uint32_t channel_no) const;
    long double getBonusRate(uint8_t server_group,
                             uint32_t channel_no,
                             uint32_t dungeon_index) const;
    long double getBonusRateEventServer(uint8_t server_group,
                                        uint32_t channel_no) const;

    DungeonInfoMap dungeon_infos;          // +0x00
    ServerChannelInfoMap channel_infos;    // +0x18
};

int importChannelScript(channel_script_t* channel_script);
int applyChannelScript(channel_script_t* channel_script);
int traversalChannelScript(channel_script_t* channel_script);

#endif  // OLDSRV_OLD_DATA_CHANNEL_CHANNEL_SCRIPT_T_H_
