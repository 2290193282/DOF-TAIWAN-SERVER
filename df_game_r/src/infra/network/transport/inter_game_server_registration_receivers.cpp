#include "infra/network/transport/inter_game_server_registration_receivers.h"

#include <stdint.h>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/world/region/game_world.h"
#include "core/lifecycle/shutdown/shutdown_manager.h"

namespace {

void SendGameServerInfo(bool guild_server, uint8_t requested_group) {
    CEnvironment* const environment = G_CEnvironment();
    const unsigned short udp_port = static_cast<unsigned short>(
        environment->get_udp_port());
    char* const server_ip = const_cast<char*>(environment->get_server_ip());
    const unsigned char server_group = static_cast<unsigned char>(
        environment->get_server_group());
    const unsigned char channel_no = static_cast<unsigned char>(
        environment->get_channel_no());
    const ENUM_SERVER_GROUP target_group =
        static_cast<ENUM_SERVER_GROUP>(requested_group);

    if (guild_server) {
        if (GlobalData::s_guild_proxy_mgr) {
            CGuildServerProxy* const proxy =
                GlobalData::s_guild_proxy_mgr->GetServerProxy(target_group);
            if (proxy) {
                proxy->SendGameServerInfo(server_ip, udp_port, server_group,
                                          channel_no);
            }
        }
    } else if (GlobalData::s_monitor_proxy_mgr) {
        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(target_group);
        if (proxy) {
            proxy->SendGameServerInfo(server_ip, udp_port, server_group,
                                      channel_no);
        }
    }
}

}  // namespace

// IDA_EA: 0x84E2B94
// IDA_NAME: _ZN23Inter_ReqGameServerInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReqGameServerInfo::dispatch_sig(CUser *,char *,int)
int Inter_ReqGameServerInfo::dispatch_sig(CUser*, char* payload,
                                          int payload_size) {
    if (!payload || payload_size < 12 || G_GameWorld()->GetChannelType() == 7) {
        return 0;
    }

    const bool integrated_pvp = G_GameWorld()->IsIntegratedPvPBaseChannel();
    const uint8_t requested_group = integrated_pvp
        ? static_cast<uint8_t>(payload[11])
        : 0u;
    if (static_cast<uint8_t>(payload[10]) == 1u) {
        SendGameServerInfo(true, requested_group);
    } else if (payload[10] == 0) {
        SendGameServerInfo(false, requested_group);
    }
    return 0;
}

// IDA_EA: 0x84EE8B8 / 0x84F6A74 / 0x84F6AA4
Inter_ReqGameServerInfo::Inter_ReqGameServerInfo() : ISigDispatcher() {}
Inter_ReqGameServerInfo::~Inter_ReqGameServerInfo() {}

// IDA_EA: 0x84E2DEE
// IDA_NAME: _ZN32Inter_MonitorRegistGameServerAck12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorRegistGameServerAck::dispatch_sig(CUser *,char *,int)
int Inter_MonitorRegistGameServerAck::dispatch_sig(CUser*, char* payload,
                                                   int payload_size) {
    if (payload && payload_size >= 12 && payload[11] == 1) {
        CShutdowManager* const shutdown_manager = GetInstanceShutdowManager();
        if (shutdown_manager) {
            shutdown_manager->SendShutdownSignal();
        }
    }
    return 0;
}

// IDA_EA: 0x84EE8D4 / 0x84F6A26 / 0x84F6A56
Inter_MonitorRegistGameServerAck::Inter_MonitorRegistGameServerAck()
    : ISigDispatcher() {}
Inter_MonitorRegistGameServerAck::~Inter_MonitorRegistGameServerAck() {}
