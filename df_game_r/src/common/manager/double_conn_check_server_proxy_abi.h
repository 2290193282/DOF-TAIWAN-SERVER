#ifndef OLDSRV_GLOBAL_DOUBLE_CONN_CHECK_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_DOUBLE_CONN_CHECK_SERVER_PROXY_ABI_H_

#include <stdint.h>

#include <string>

#include "infra/network/transport/udp_handler.h"


// IDA unit: 99_misc_global/CDoubleConnCheckServerProxy.  The old object is
// exactly std::string (+0x00), UDP port (+0x04), and CUdpHandler (+0x08).
class CDoubleConnCheckServerProxy {
public:
    // IDA_EA: 0x846D300
    // IDA_NAME: _ZN27CDoubleConnCheckServerProxyC2ESsi
    // DEMANGLED: CDoubleConnCheckServerProxy::CDoubleConnCheckServerProxy(std::string,int)
    CDoubleConnCheckServerProxy(std::string server_ip, int udp_port);

    // IDA_EA: 0x846D356
    // IDA_NAME: _ZN27CDoubleConnCheckServerProxy4InitEv
    // DEMANGLED: CDoubleConnCheckServerProxy::Init(void)
    bool Init();

    // IDA_EA: 0x846D412
    // IDA_NAME: _ZN27CDoubleConnCheckServerProxy9SendLoginEjii
    // DEMANGLED: CDoubleConnCheckServerProxy::SendLogin(unsigned int,int,int)
    int SendLogin(uint32_t account_id, int channel_no, int server_group);

    // IDA_EA: 0x846D47C
    // IDA_NAME: _ZN27CDoubleConnCheckServerProxy10SendLogoutEjii
    // DEMANGLED: CDoubleConnCheckServerProxy::SendLogout(unsigned int,int,int)
    void SendLogout(uint32_t account_id, int channel_no, int server_group);

    // IDA_EA: 0x846D4EE
    // IDA_NAME: _ZN27CDoubleConnCheckServerProxy10SendPacketEPci
    // DEMANGLED: CDoubleConnCheckServerProxy::SendPacket(char *,int)
    int SendPacket(char* packet, int size);

private:
    std::string server_ip_;
    int udp_port_;
    CUdpHandler udp_handler_;
};

#if defined(__i386__)
static_assert(sizeof(CDoubleConnCheckServerProxy) == 0x10u,
              "CDoubleConnCheckServerProxy must retain the old 16-byte ABI");
#endif

#endif  // OLDSRV_GLOBAL_DOUBLE_CONN_CHECK_SERVER_PROXY_ABI_H_
