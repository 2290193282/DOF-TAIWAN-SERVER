#ifndef OLDSRV_CHANNEL_CHANNEL_PROXY_H_
#define OLDSRV_CHANNEL_CHANNEL_PROXY_H_

#include "infra/network/transport/udp_handler.h"

#include <string>

// IDA unit: 03_channel_select/CmmChannelProxy. Old i386 size: 0x80020.
class CmmChannelProxy {
public:
    CmmChannelProxy(std::string bridge_ip, int udp_port_channel);

    int Init();
    int SendChannelInfo();
    int ConnectBridgeServer();
    int SendPacket(char* buffer, int bytes);
    int RecvPacket();
    int SendCheckChannelScript();

private:
    std::string bridge_ip_;
    int udp_port_channel_;
    char reserved_08_[8];
    CUdpHandler udp_handler_;
    int bridge_socket_;
    int recv_size_;
    char recv_buffer_[0x80000];
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CmmChannelProxy) == 0x80020u,
              "CmmChannelProxy IDA ABI size must stay 0x80020");
#endif

#endif  // OLDSRV_CHANNEL_CHANNEL_PROXY_H_
