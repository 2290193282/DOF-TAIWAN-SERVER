#include "infra/security/policy/hades_server_proxy_abi.h"

#include <string>

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/packet/udp_packets.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8470510
// IDA_NAME: _ZN17CHadesServerProxyC2EPcii
// DEMANGLED: CHadesServerProxy::CHadesServerProxy(char *,int,int)
CHadesServerProxy::CHadesServerProxy(char* udp_ip_hades,
                                     int tcp_port_hades,
                                     int udp_port_hades)
    : BaseServerProxy(udp_ip_hades,
                      tcp_port_hades,
                      const_cast<char*>("HadesSvr")),
      udp_port_hades_(udp_port_hades),
      udp_handler_(),
      socket_conn_sec_(0) {
    HadesSvrInit();
}

// IDA_EA: 0x847059A
// IDA_NAME: _ZN17CHadesServerProxy12HadesSvrInitEv
// DEMANGLED: CHadesServerProxy::HadesSvrInit(void)
int CHadesServerProxy::HadesSvrInit() {
    ConnTcpHadesSvr();
    InitUdp();
    return 1;
}

// IDA_EA: 0x84705BE
// IDA_NAME: _ZN17CHadesServerProxy15ConnTcpHadesSvrEv
// DEMANGLED: CHadesServerProxy::ConnTcpHadesSvr(void)
int CHadesServerProxy::ConnTcpHadesSvr() {
    BaseServerProxy::Init();
    SetSocketState(SOCKET_STATE_CONNECTED);
    socket_conn_sec_ = GlobalData::system_time()->getCurSec();
    return 1;
}

// IDA_EA: 0x84705FE
// IDA_NAME: _ZN17CHadesServerProxy9IsConnectEv
// DEMANGLED: CHadesServerProxy::IsConnect(void)
bool CHadesServerProxy::IsConnect() {
    return GetSocketState() == SOCKET_STATE_CONNECTED;
}

// IDA_EA: 0x8470628
// IDA_NAME: _ZN17CHadesServerProxy7InitUdpEv
// DEMANGLED: CHadesServerProxy::InitUdp(void)
int CHadesServerProxy::InitUdp() {
    if (udp_handler_.InitClientSocket() != -1) {
        return 1;
    }
    LogManager::logFormat(1,
                          "DF_HadesServerProxy.cpp",
                          "bool CHadesServerProxy::InitUdp()",
                          0xD5,
                          "[CHadesServerProxy::Init] Can't init udp client socket");
    return 0;
}

// IDA_EA: 0x8470682
// IDA_NAME: _ZN17CHadesServerProxy19SendHeartBeatPacketEti
// DEMANGLED: CHadesServerProxy::SendHeartBeatPacket(unsigned short,int)
void CHadesServerProxy::SendHeartBeatPacket(uint16_t udp_port,
                                            int channel_no) {
    Packet_Hades_Udp_HeartBeat packet;
    packet.server_id = udp_port;
    packet.state = static_cast<uint16_t>(channel_no);
    SendHadesUdpPacket(reinterpret_cast<char*>(&packet));
}

// IDA_EA: 0x84706BE
// IDA_NAME: _ZN17CHadesServerProxy18SendHadesUdpPacketEPc
// DEMANGLED: CHadesServerProxy::SendHadesUdpPacket(char *)
void CHadesServerProxy::SendHadesUdpPacket(char* packet) {
    if (!packet) {
        return;
    }
    std::string server_ip = GetServerIP();
    udp_handler_.SendToServer(
        packet,
        reinterpret_cast<PacketHeader*>(packet)->size,
        static_cast<uint16_t>(udp_port_hades_),
        server_ip.c_str());
}

// IDA_EA: 0x8470752
// IDA_NAME: _ZN17CHadesServerProxy18SendHadesTcpPacketEPc
// DEMANGLED: CHadesServerProxy::SendHadesTcpPacket(char *)
int CHadesServerProxy::SendHadesTcpPacket(char* packet) {
    if (packet && GetSocketState() == SOCKET_STATE_CONNECTED) {
        SendTcpPacket(packet, reinterpret_cast<PacketHeader*>(packet)->size);
    }
    return 1;
}

// IDA_EA: 0x8470794
// IDA_NAME: _ZN17CHadesServerProxy7ParsingEi
// DEMANGLED: CHadesServerProxy::Parsing(int)
int CHadesServerProxy::Parsing(int received_bytes) {
    return GetTcpHandler()->Parsing_ServerPacket(received_bytes);
}

// IDA_EA: 0x84707B6
// IDA_NAME: _ZN17CHadesServerProxy12OnDisconnectEv
// DEMANGLED: CHadesServerProxy::OnDisconnect(void)
BaseServerProxy* CHadesServerProxy::OnDisconnect() {
    RunConnTimer();
    return this;
}

// IDA_EA: 0x84707CA
// IDA_NAME: _ZN17CHadesServerProxy12RunConnTimerEv
// DEMANGLED: CHadesServerProxy::RunConnTimer(void)
void CHadesServerProxy::RunConnTimer() {
    CStreamGuard stream_guard(
        GlobalData::stream_pool()->Acquire("DF_HadesServerProxy.cpp", 281),
        true);
    // IDA 0x8470811 stores the full immediate 0x23A.  Treating its low
    // byte (0x3A / 58) as the Inter signal routed disconnects into an empty
    // table slot; the original message is Inter_HadesDisConnect at slot 570.
    (*stream_guard) << 570;
    (*stream_guard) << -1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
}

// IDA_EA: 0x84B9AB2
// IDA_NAME: _ZN17CHadesServerProxy21GetHadesSocketConnSecEv
// DEMANGLED: CHadesServerProxy::GetHadesSocketConnSec(void)
int CHadesServerProxy::GetHadesSocketConnSec() {
    return socket_conn_sec_;
}
