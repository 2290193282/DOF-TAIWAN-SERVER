#include "old_data/PVF/pvp_mission/pvp_channel_list.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>
#include <utility>
#include <vector>


typedef char live_server_info_t_Size_Must_Be_0x18[
    (sizeof(live_server_info_t) == 0x18) ? 1 : -1];

// IDA_EA: 0x8901F10
// IDA_NAME: _Z20importPVPChannelListPcP18live_server_info_ti
// DEMANGLED: importPVPChannelList(char *,live_server_info_t *,int)
int importPVPChannelList(char* filename,
                         live_server_info_t* live_server_info,
                         int value) {
    (void)value;

    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[PVP CHANNEL INFO]") {
            while (1) {
                const int server_group = ScanInt(&success);
                if (!success) {
                    break;
                }

                const int row_count = ScanInt(&success);
                if (!success) {
                    break;
                }

                std::vector<pvp_channel_info_t> channels;
                std::string text;
                for (int index = 0; index < row_count; ++index) {
                    ScanStr(&text);

                    pvp_channel_info_t channel;
                    channel.channel_no = ScanInt(&success);
                    if (!success) return 0;
                    channel.min_grade = ScanInt(&success);
                    if (!success) return 0;
                    channel.max_grade = ScanInt(&success);
                    if (!success) return 0;
                    channel.channel_type = ScanInt(&success);
                    if (!success) return 0;
                    channel.upgrade_revision = ScanInt(&success);
                    if (!success) return 0;
                    channel.min_level = ScanInt(&success);
                    if (!success) return 0;
                    channel.max_level = ScanInt(&success);
                    if (!success) return 0;
                    channel.pvp_mode = ScanInt(&success);
                    if (!success) return 0;
                    channel.team_count = ScanInt(&success);
                    if (!success) return 0;

                    channels.push_back(channel);
                    ScanStr(&text);
                }

                live_server_info->insert(std::make_pair(server_group,
                                                        channels));
            }
        }
    }

    return 1;
}
