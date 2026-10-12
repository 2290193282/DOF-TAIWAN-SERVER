#ifndef OLDSRV_GLOBAL_SCHOOL_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_SCHOOL_SERVER_PROXY_ABI_H_

#include <string>

#include "infra/network/transport/udp_handler.h"

// IDA unit: 99_misc_global/CSchoolServerProxy.  The old object consists of
// std::string (+0x00), the UDP port (+0x04), and CUdpHandler (+0x08).
class CSchoolServerProxy {
public:
    // IDA_EA: 0x8471E24
    // IDA_NAME: _ZN18CSchoolServerProxyC2ESsi
    // DEMANGLED: CSchoolServerProxy::CSchoolServerProxy(std::string,int)
    CSchoolServerProxy(std::string school_server_ip, int school_server_port);

    // IDA_EA: 0x8471E7A
    // IDA_NAME: _ZN18CSchoolServerProxy4InitEv
    // DEMANGLED: CSchoolServerProxy::Init(void)
    bool Init();

    // IDA_EA: 0x8471ED2
    // IDA_NAME: _ZN18CSchoolServerProxy10SendPacketEPci
    // DEMANGLED: CSchoolServerProxy::SendPacket(char *,int)
    int SendPacket(char* buffer, int bytes);

private:
    std::string school_server_ip_;  // IDA +0x00
    int school_server_port_;        // IDA +0x04
    CUdpHandler udp_handler_;       // IDA +0x08
};

#if defined(__i386__)
static_assert(sizeof(CSchoolServerProxy) == 0x10u,
              "CSchoolServerProxy must retain the old 16-byte ABI");
#endif

#endif  // OLDSRV_GLOBAL_SCHOOL_SERVER_PROXY_ABI_H_
