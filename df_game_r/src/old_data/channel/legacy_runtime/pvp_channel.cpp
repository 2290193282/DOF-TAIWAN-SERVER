#include "old_data/channel/legacy_runtime/pvp_channel.h"
#include "old_data/db/account/login_payloads.h"
#include "old_game/world/region/game_world.h"


namespace ARAD {

// IDA_EA: 0x819763C
// IDA_NAME: _ZN4ARAD11PVP_CHANNEL25is_integrated_pvp_channelEv
// DEMANGLED: ARAD::PVP_CHANNEL::is_integrated_pvp_channel(void)
bool PVP_CHANNEL::is_integrated_pvp_channel() {
    const unsigned int channel_type = static_cast<unsigned int>(
        G_GameWorld()->GetChannelType());
    return channel_type <= 0x10u && ((1u << channel_type) & 0x1E700u) != 0;
}

// IDA_EA: 0x8197684
// IDA_NAME: _ZN4ARAD11PVP_CHANNEL20is_broadcast_channelEv
// DEMANGLED: ARAD::PVP_CHANNEL::is_broadcast_channel(void)
bool PVP_CHANNEL::is_broadcast_channel() {
    const int channel_type =
        G_GameWorld()->GetChannelType();
    return is_integrated_pvp_channel() || channel_type == 7;
}

// IDA_EA: 0x81976C7
// IDA_NAME: _ZN4ARAD11PVP_CHANNEL18is_all_pvp_channelEv
// DEMANGLED: ARAD::PVP_CHANNEL::is_all_pvp_channel(void)
bool PVP_CHANNEL::is_all_pvp_channel() {
    const int channel_type =
        G_GameWorld()->GetChannelType();
    return is_integrated_pvp_channel() || channel_type == 7;
}

// IDA_EA: 0x819770A
// IDA_NAME: _ZN4ARAD11PVP_CHANNEL23is_winpoint_pvp_channelEv
// DEMANGLED: ARAD::PVP_CHANNEL::is_winpoint_pvp_channel(void)
bool PVP_CHANNEL::is_winpoint_pvp_channel() {
    const int channel_type =
        G_GameWorld()->GetChannelType();
    return channel_type == 4 || channel_type == 8;
}

// IDA_EA: 0x819773B
// IDA_NAME: _ZN4ARAD11PVP_CHANNEL17is_common_channelEv
// DEMANGLED: ARAD::PVP_CHANNEL::is_common_channel(void)
bool PVP_CHANNEL::is_common_channel() {
    const int channel_type =
        G_GameWorld()->GetChannelType();
    return !is_integrated_pvp_channel() && channel_type != 7;
}

// IDA_EA: 0x8197781
// IDA_NAME: _ZN4ARAD11PVP_CHANNEL24check_Login_server_groupEP14SIG_LOGIN_DATA
// DEMANGLED: ARAD::PVP_CHANNEL::check_Login_server_group(SIG_LOGIN_DATA *)
int PVP_CHANNEL::check_Login_server_group(SIG_LOGIN_DATA* login_data) {
    (void)G_GameWorld()->GetChannelType();
    if (!is_all_pvp_channel()) {
        login_data->server_group = 0;
    }
    return 1;
}

}  // namespace ARAD
