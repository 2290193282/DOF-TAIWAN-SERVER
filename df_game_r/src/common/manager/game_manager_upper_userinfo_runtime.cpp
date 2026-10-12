#include "common/manager/game_manager.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/channel/packet_dispatch/channel_packets.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/udp_packets.h"

namespace {

// IDA_EA: 0x829611A
// IDA_NAME: _Z25SendUserInfoToUpperServerR33Packet_Monitor_UDP_Reply_UserInfohP5CUser17ENUM_SERVER_GROUP
// DEMANGLED: SendUserInfoToUpperServer(Packet_Monitor_UDP_Reply_UserInfo &,unsigned char,CUser *,ENUM_SERVER_GROUP)
void SendUserInfoToUpperServer(Packet_Monitor_UDP_Reply_UserInfo& packet,
                               unsigned char target,
                               CUser* user,
                               ENUM_SERVER_GROUP server_group) {
    if (target == static_cast<unsigned char>(0xcbu)) {
        GlobalData::s_guild_proxy_mgr->GetServerProxy(server_group)->SendTcpPacket(
            reinterpret_cast<char*>(&packet), packet.size);
    } else if (target == static_cast<unsigned char>(0xc9u)) {
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(server_group)->SendTcpPacket(
            reinterpret_cast<char*>(&packet), packet.size);
    }

    // The original helper clears precisely the 75 typed records at +0x0c;
    // header/result/channel stay intact for the following batch.
    std::memset(packet.users, 0, sizeof(packet.users));
    (void)user;
}

}  // namespace

// IDA_EA: 0x82961BE
// IDA_NAME: _ZN12CGameManager30Send_userinfos_to_upper_serverEh
// DEMANGLED: CGameManager::Send_userinfos_to_upper_server(unsigned char)
char CGameManager::Send_userinfos_to_upper_server(unsigned char target) {
    Packet_Monitor_UDP_Reply_UserInfo packet;
    packet.channel_no =
        static_cast<uint8_t>(G_CEnvironment()->get_channel_no());

    // The original sends a channel-type TCP frame to every monitor group for
    // integrated PvP when the monitor endpoint (0xc9) asks for user info.
    // Packet_ChannelType C2 is deliberately retained as its own recovered
    // packet ABI; the old code obtains the two channel values immediately
    // before sending that constructor-created frame.
    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        target == static_cast<unsigned char>(0xc9u)) {
        int iteration = 0;
        for (int server_group = GlobalData::s_monitor_proxy_mgr->GetStartIndex();
             GlobalData::s_monitor_proxy_mgr->GetEndIndex() > iteration;
             server_group = GlobalData::s_monitor_proxy_mgr->GetNextIndex(iteration)) {
            Packet_ChannelType channel_type;
            const int channel_no = G_CEnvironment()->get_channel_no();
            const int channel_kind = G_GameWorld()->GetChannelType();
            (void)channel_no;
            (void)channel_kind;
            GlobalData::s_monitor_proxy_mgr
                ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(server_group))
                ->SendTcpPacket(reinterpret_cast<char*>(&channel_type),
                                channel_type.header.size);
        }
    }

    const char empty = users_by_account_.empty() ? 1 : 0;
    if (empty) {
        return empty;
    }

    int iteration = 0;
    for (int server_group = GlobalData::s_monitor_proxy_mgr->GetStartIndex();
         GlobalData::s_monitor_proxy_mgr->GetEndIndex() > iteration;
         server_group = GlobalData::s_monitor_proxy_mgr->GetNextIndex(iteration)) {
        int user_count = 0;
        CUser* last_user = 0;
        for (UserMap::const_iterator it = users_by_account_.begin();
             it != users_by_account_.end(); ++it) {
            CUser* const user = it->second;
            if (user->get_state() <= 1 || user->GetServerGroup() != server_group) {
                continue;
            }

            MonitorUpperServerUserInfo& record = packet.users[user_count];
            record.account_id = static_cast<uint32_t>(user->get_acc_id());
            record.charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
            record.user_uid = static_cast<uint32_t>(user->GetUID());
            record.charac_guildkey =
                static_cast<uint32_t>(user->get_charac_guildkey());
            record.job = static_cast<uint8_t>(user->get_charac_job());
            record.grow_type =
                static_cast<uint8_t>(user->getCurCharacGrowType());
            record.level = static_cast<uint16_t>(user->get_charac_level());
            std::strcpy(record.charac_name, user->getCurCharacName());
            record.charac_memberkey =
                static_cast<uint32_t>(user->get_charac_memberkey());
            user->network_.GetPeerIP2(record.peer_ip, sizeof(record.peer_ip));
            record.sex = static_cast<uint8_t>(user->getSex());
            std::memcpy(record.ssn, user->GetSsnString(), sizeof(record.ssn));

            last_user = user;
            ++user_count;
            if (user_count > 74) {
                packet.size = 0x16e6u;
                SendUserInfoToUpperServer(
                    packet, target, last_user,
                    static_cast<ENUM_SERVER_GROUP>(server_group));
                user_count = 0;
            }
        }

        if (user_count) {
            packet.size = static_cast<uint16_t>(
                sizeof(PacketHeader) + sizeof(packet.result) +
                sizeof(packet.channel_no) +
                static_cast<size_t>(user_count) * sizeof(packet.users[0]));
            SendUserInfoToUpperServer(packet, target, last_user,
                                      static_cast<ENUM_SERVER_GROUP>(server_group));
        }
    }
    return empty;
}
