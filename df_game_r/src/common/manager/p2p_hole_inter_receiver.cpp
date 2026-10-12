#include "common/manager/p2p_hole_inter_receiver.h"

#include <cstring>
#include <string>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/restrict_geolocation_abi.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/dispatcher/p2p_hole_punching_statistics_dispatcher.h"
#include "infra/network/transport/network_endpoint.h"


// IDA_EA: 0x8161DAE
// IDA_NAME: _ZN38Packet_GameServer2Statisctics2DBServerC2Ev
// DEMANGLED: Packet_GameServer2Statisctics2DBServer::Packet_GameServer2Statisctics2DBServer(void)
Packet_GameServer2Statisctics2DBServer::
    Packet_GameServer2Statisctics2DBServer()
    : PacketHeader(0x27FDu, 0x35u),
      server_group(0),
      p2p_state(0xFFu),
      p2p_value_0e(0),
      p2p_value_12(0) {
    std::memset(country_code, 0, sizeof(country_code));
    std::memset(peer_ip, 0, sizeof(peer_ip));
}

// IDA_EA: 0x81621E2
// IDA_NAME: _ZN31InterUpdateP2pHolePunchingStateC2Ev
// DEMANGLED: InterUpdateP2pHolePunchingState::InterUpdateP2pHolePunchingState(void)
InterUpdateP2pHolePunchingState::InterUpdateP2pHolePunchingState()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::P2P_HOLE_PUNCHING_STATE) {}

// IDA_EA: 0x8162206
// IDA_NAME: _ZN31InterUpdateP2pHolePunchingStateD2Ev
// DEMANGLED: InterUpdateP2pHolePunchingState::~InterUpdateP2pHolePunchingState()
// IDA_EA: 0x8162236
// IDA_NAME: _ZN31InterUpdateP2pHolePunchingStateD0Ev
// DEMANGLED: InterUpdateP2pHolePunchingState::~InterUpdateP2pHolePunchingState()
InterUpdateP2pHolePunchingState::~InterUpdateP2pHolePunchingState() {}

// IDA_EA: 0x8161952
// IDA_NAME: _ZN31InterUpdateP2pHolePunchingState12dispatch_sigEP5CUserPci
// DEMANGLED: InterUpdateP2pHolePunchingState::dispatch_sig(CUser *,char *,int)
int InterUpdateP2pHolePunchingState::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    (void)payload_size;

    const MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE* const input =
        reinterpret_cast<const MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE*>(payload);
    Packet_GameServer2Statisctics2DBServer packet;
    packet.server_group = static_cast<uint16_t>(
        G_CEnvironment()->get_server_group());
    packet.p2p_state = input->raw_0d;
    packet.p2p_value_0e = input->raw_0e;
    packet.p2p_value_12 = input->raw_12;

    // The old receiver owns a direct CUser CNetwork child access.  Friendship
    // is intentionally limited to this receiver; no CUser network bridge is
    // introduced.
    user->network_.GetPeerIP2(packet.peer_ip, sizeof(packet.peer_ip));

    const std::string peer_ip(packet.peer_ip);
    ARAD::Singleton<RestrictGeolocation>::Get()->getCountryCode(
        packet.country_code, sizeof(packet.country_code), peer_ip);

    CStatisticServerProxy* const proxy =
        GlobalData::s_statistic_proxy_mgr->GetServerProxy(
            SERVER_GROUP_DEFAULT);
    proxy->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    return 0;
}
