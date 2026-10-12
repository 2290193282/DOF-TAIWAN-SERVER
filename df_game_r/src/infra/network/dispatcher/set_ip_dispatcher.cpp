#include "infra/network/dispatcher/set_ip_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/transport/network_identity_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/auth_security/db_select_allow_proxy_user.h"
#include "old_data/db/auth_security/save_proxy_log_payload.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/transport/network_endpoint.h"
#include "infra/security/policy/checksum.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdint.h>

namespace {

const char kDispatchSignature[] =
    "virtual int DisPatcher_SetIP::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8595564
// IDA_NAME: _ZN16DisPatcher_SetIPC2Ev
// DEMANGLED: DisPatcher_SetIP::DisPatcher_SetIP(void)
DisPatcher_SetIP::DisPatcher_SetIP() {}

// IDA_EA: 0x8232FDC
// IDA_NAME: _ZN16DisPatcher_SetIPD2Ev
// DEMANGLED: DisPatcher_SetIP::~DisPatcher_SetIP()
// IDA_EA: 0x823300C
// IDA_NAME: _ZN16DisPatcher_SetIPD0Ev
// DEMANGLED: DisPatcher_SetIP::~DisPatcher_SetIP()
DisPatcher_SetIP::~DisPatcher_SetIP() {}

// IDA_EA: 0x81F468C
// IDA_NAME: _ZN16DisPatcher_SetIP12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SetIP::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SetIP::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() > 3) {
        return 0;
    }

    char nat_type = 0;
    unsigned int inner_ip = 0;
    unsigned int nat_ip = 0;
    unsigned short nat_port = 0;
    unsigned int mtu = 0;
    if (packet.get_byte(nat_type) != 1) {
        return LineFunc(32996, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(inner_ip) != 1) {
        return LineFunc(32997, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(nat_ip) != 1) {
        return LineFunc(32998, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(nat_port) != 1) {
        return LineFunc(32999, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(mtu) != 1) {
        return LineFunc(33000, kDispatchSignature, 0, 0);
    }

    // The 0828 client serializes the detected NAT IPv4 value in host order,
    // while CNetwork stores in_addr::s_addr network order.  Without this
    // conversion 192.168.200.1 becomes 1.200.168.192 and is rejected as a
    // proxy after the UDP-identification timeout.
    const unsigned int network_nat_ip = htonl(nat_ip);

    // The original CMD 2 body directly owns this named embedded CNetwork
    // child for set_nat, GetPeerIP2, and get_str_ip.  No synthetic CUser
    // accessor is introduced.
    CNetwork4096_450000* network = &user->network_;
    network->set_nat(nat_type, inner_ip, network_nat_ip, nat_port, mtu);

    unsigned int mac_length = 0;
    char mac_input[65];
    if (packet.get_int(mac_length) != 1) {
        return LineFunc(33024, kDispatchSignature, 0, 0);
    }
    if (packet.get_str(mac_input, 65, static_cast<int>(mac_length)) != 1) {
        return LineFunc(33026, kDispatchSignature, 0, 0);
    }

    char mac_sha256[65];
    std::memset(mac_sha256, 0, sizeof(mac_sha256));
    getSha256UsingSalt(reinterpret_cast<const unsigned char*>(mac_input),
                       65,
                       mac_sha256);

    cMyTrace trace(kDispatchSignature, 33035, 0);
    trace("(acc_id: %s) (nat type: %d) (inner_ip: %d) (nat_ip: %d) (nat_port: %d) (mac_addr: %s) (mtu: %d)",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          static_cast<int>(nat_type),
          static_cast<int>(inner_ip),
          static_cast<int>(network_nat_ip),
          static_cast<int>(nat_port),
          mac_sha256,
          static_cast<int>(mtu));

    network->set_nat(
        nat_type, inner_ip, network_nat_ip, nat_port, mac_sha256);

    char peer_ip[20];
    std::memset(peer_ip, 0, sizeof(peer_ip));
    char* peer_ip_result = network->GetPeerIP2(peer_ip, 20);
    const char* str_ip = network->get_str_ip();
    if (checkDenyIP(str_ip, peer_ip_result)) {
        Stream* stream = GlobalData::stream_pool()->Acquire(
            "PacketDispatcher_Impl_1.cpp", 33053);
        CStreamGuard stream_guard(stream, true);
        stream_guard << 158;
        stream_guard << -1;

        SIG_SAVE_PROXY_LOG* payload =
            stream_guard.GetInBuffer<SIG_SAVE_PROXY_LOG>();
        payload->m_id = static_cast<uint32_t>(user->get_acc_id());
        std::strncpy(payload->first_ip, network->get_str_ip(), 0x10u);
        std::strncpy(payload->second_ip, peer_ip, 0x10u);
        payload->proxy_flag = static_cast<uint8_t>(nat_type);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, stream_guard);
        DB_SelectAllowProxyUser::makeRequest(
            user->GetUID(), static_cast<unsigned int>(user->get_acc_id()));
    } else {
        PacketGuard reply;
        reply.put_header(1, 2);
        reply.put_byte(1);
        reply.finalize(true);
        user->Send(reply);
    }
    return 0;
}

// IDA_EA: 0x8232FC8
// IDA_NAME: _ZN16DisPatcher_SetIP4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetIP::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetIP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232FD2
// IDA_NAME: _ZN16DisPatcher_SetIP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetIP::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetIP::process(CUser* user,
                              MSG_BASE& message,
                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
