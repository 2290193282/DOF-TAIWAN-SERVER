#ifndef OLDSRV_CHANNEL_PVP_CHANNEL_H_
#define OLDSRV_CHANNEL_PVP_CHANNEL_H_

struct SIG_LOGIN_DATA;

namespace ARAD {

// IDA unit: 03_channel_select/ARAD_PVP_CHANNEL.
class PVP_CHANNEL {
public:
    static bool is_integrated_pvp_channel();
    static bool is_broadcast_channel();
    static bool is_all_pvp_channel();
    static bool is_winpoint_pvp_channel();
    static bool is_common_channel();
    static int check_Login_server_group(SIG_LOGIN_DATA* login_data);
};

}  // namespace ARAD

#endif  // OLDSRV_CHANNEL_PVP_CHANNEL_H_
