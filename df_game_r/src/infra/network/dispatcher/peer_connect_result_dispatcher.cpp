#include "infra/network/dispatcher/peer_connect_result_dispatcher.h"

#include <stdint.h>

#include "common/manager/global_tools.h"
#include "common/types/statistics_nat_type.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/transport/network_endpoint.h"


namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_PeerConnectResult::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace


// IDA_EA: 0x8595E78
// IDA_NAME: _ZN28Dispatcher_PeerConnectResultC2Ev
// DEMANGLED: Dispatcher_PeerConnectResult::Dispatcher_PeerConnectResult(void)
Dispatcher_PeerConnectResult::Dispatcher_PeerConnectResult() {}

// IDA_EA: 0x8242E50
// IDA_NAME: _ZN28Dispatcher_PeerConnectResultD2Ev
// DEMANGLED: Dispatcher_PeerConnectResult::~Dispatcher_PeerConnectResult()
// IDA_EA: 0x8242E80
// IDA_NAME: _ZN28Dispatcher_PeerConnectResultD0Ev
// DEMANGLED: Dispatcher_PeerConnectResult::~Dispatcher_PeerConnectResult()
Dispatcher_PeerConnectResult::~Dispatcher_PeerConnectResult() {}

// IDA_EA: 0x82033E6
// IDA_NAME: _ZN28Dispatcher_PeerConnectResult12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_PeerConnectResult::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_PeerConnectResult::dispatch_sig(CUser* user, PacketBuf& packet) {
    unsigned char result = 0;
    short target_user_uid = 0;
    if (packet.get_byte(result) != 1) {
        return LineFunc(39452, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(target_user_uid) != 1) {
        return LineFunc(39455, kDispatchSignature, 0, 0);
    }

    CUser* const peer_user = G_GameWorld()->find_from_world(
        static_cast<uint16_t>(target_user_uid));
    if (peer_user) {
        // The old body addresses the named embedded CNetwork child directly
        // for both users; this matches CMD2 and the P2P receiver.
        CNetwork4096_450000* const user_network =
            &user->network_;
        CNetwork4096_450000* const peer_network =
            &peer_user->network_;
        const int user_nat_type = static_cast<char>(
            user_network->get_nat_type());
        const int peer_nat_type = static_cast<char>(
            peer_network->get_nat_type());
        StatisticsNatType* const statistics =
            reinterpret_cast<StatisticsNatType*>(
                G_GameWorld()->GetStatisticsNatType());
        statistics->IncreaseNatType(user_nat_type, peer_nat_type, result);
    }
    return 0;
}

// IDA_EA: 0x82336D4
// IDA_NAME: _ZN28Dispatcher_PeerConnectResult4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PeerConnectResult::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PeerConnectResult::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82336DE
// IDA_NAME: _ZN28Dispatcher_PeerConnectResult7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PeerConnectResult::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PeerConnectResult::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
