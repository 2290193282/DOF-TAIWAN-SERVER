#ifndef OLDSRV_GLOBAL_HADES_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_HADES_SERVER_PROXY_ABI_H_

#include <stdint.h>

#include "common/manager/base_server_proxy_abi.h"
#include "infra/network/transport/udp_handler.h"

// IDA unit: 99_misc_global/CHadesServerProxy.
// The old proxy extends the shared BaseServerProxy with a UDP heartbeat
// sender and the second at which its current TCP session was established.
class CHadesServerProxy : public BaseServerProxy {
public:
    // IDA_EA: 0x8470510
    // IDA_NAME: _ZN17CHadesServerProxyC2EPcii
    CHadesServerProxy(char* udp_ip_hades,
                      int tcp_port_hades,
                      int udp_port_hades);

    // IDA_EA: 0x847059A
    // IDA_NAME: _ZN17CHadesServerProxy12HadesSvrInitEv
    int HadesSvrInit();

    // IDA_EA: 0x84705BE
    // IDA_NAME: _ZN17CHadesServerProxy15ConnTcpHadesSvrEv
    int ConnTcpHadesSvr();

    // IDA_EA: 0x84705FE
    // IDA_NAME: _ZN17CHadesServerProxy9IsConnectEv
    bool IsConnect();

    // IDA_EA: 0x8470628
    // IDA_NAME: _ZN17CHadesServerProxy7InitUdpEv
    int InitUdp();

    // IDA_EA: 0x8470682
    // IDA_NAME: _ZN17CHadesServerProxy19SendHeartBeatPacketEti
    void SendHeartBeatPacket(uint16_t udp_port, int channel_no);

    // IDA_EA: 0x84706BE
    // IDA_NAME: _ZN17CHadesServerProxy18SendHadesUdpPacketEPc
    void SendHadesUdpPacket(char* packet);

    // IDA_EA: 0x8470752
    // IDA_NAME: _ZN17CHadesServerProxy18SendHadesTcpPacketEPc
    int SendHadesTcpPacket(char* packet);

    // IDA_EA: 0x8470794
    // IDA_NAME: _ZN17CHadesServerProxy7ParsingEi
    virtual int Parsing(int received_bytes);

    // IDA_EA: 0x84707B6
    // The reconstructed BaseServerProxy ABI exposes this virtual with its
    // covariant base return type.  The old body only schedules reconnection.
    virtual BaseServerProxy* OnDisconnect();

    // IDA_EA: 0x84707CA
    // IDA_NAME: _ZN17CHadesServerProxy12RunConnTimerEv
    static void RunConnTimer();

    // IDA_EA: 0x84B9AB2
    // IDA_NAME: _ZN17CHadesServerProxy21GetHadesSocketConnSecEv
    int GetHadesSocketConnSec();

private:
    int udp_port_hades_;          // +0x30e0c (i386)
    CUdpHandler udp_handler_;     // +0x30e10
    int socket_conn_sec_;         // +0x30e18
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CHadesServerProxy) == 0x30e1cu,
              "CHadesServerProxy IDA ABI size must stay 0x30e1c");
#endif

// IDA_DATA: GlobalData::s_hades_proxy
// IDA_NAME: _ZN10GlobalData13s_hades_proxyE

#endif  // OLDSRV_GLOBAL_HADES_SERVER_PROXY_ABI_H_
