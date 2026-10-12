#include "old_data/Login/inter_returns/inter_update_logout.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/network/packet/logout_payload.h"
#include "infra/network/packet/udp_packets.h"

// IDA_EA: 0x84CE548
// IDA_NAME: _ZN18Inter_UpdateLogout12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UpdateLogout::dispatch_sig(CUser *,char *,int)
int Inter_UpdateLogout::dispatch_sig(CUser* /*user*/,
                                     char* payload,
                                     int /*payload_size*/) {
    SIG_UPDATE_LOGOUT& logout = *reinterpret_cast<SIG_UPDATE_LOGOUT*>(payload);
    if (logout.m_id != 0u) {
        Packet_Monitor_UDP_Logout_Complete packet;
        packet.account_id = logout.m_id;
        packet.channel_no = static_cast<uint8_t>(G_CEnvironment()->get_channel_no());

        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(logout.server_id));
        proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    }
    return 0;
}

// IDA_EA: 0x84EB02C
// IDA_NAME: _ZN18Inter_UpdateLogoutC2Ev
// DEMANGLED: Inter_UpdateLogout::Inter_UpdateLogout(void)
Inter_UpdateLogout::Inter_UpdateLogout() : ISigDispatcher() {}

// IDA_EA: 0x84EB048 / 0x84EB078
// IDA_NAME: _ZN18Inter_UpdateLogoutD2Ev / D0Ev
// DEMANGLED: Inter_UpdateLogout::~Inter_UpdateLogout()
Inter_UpdateLogout::~Inter_UpdateLogout() {}
