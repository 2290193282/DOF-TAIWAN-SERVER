#ifndef OLDSRV_GLOBAL_STATISTIC_PROXY_ABI_H_
#define OLDSRV_GLOBAL_STATISTIC_PROXY_ABI_H_

#include <stddef.h>

#include <string>

#include "old_data/db/core/db_types.h"
#include "infra/network/transport/udp_handler.h"

template <typename T>
class CServerProxyMgr;

class CStatisticServerProxy {
public:
    // IDA_EA: 0x82A6B72 / 0x8471F84 / 0x82B3282.
    CStatisticServerProxy();
    CStatisticServerProxy(std::string server_ip, int udp_port);
    ~CStatisticServerProxy();

    // IDA_EA: 0x8471FDA / 0x82A6BC0.
    int Init();
    int Init(std::string server_ip, int udp_port, int unused_tcp_port);

    int SendPacket(char* packet, int size);

    std::string server_ip_;                         // +0x00000
    int udp_port_;                                  // +0x00004
    CUdpHandler udp_handler_;                       // +0x00008
};

template <>
class CServerProxyMgr<CStatisticServerProxy> {
public:
    // IDA_EA: 0x82B3296 / 0x82B3B8E.
    CServerProxyMgr();
    ~CServerProxyMgr();

    CStatisticServerProxy* GetServerProxy(ENUM_SERVER_GROUP server_group);
    // IDA_EA: 0x82B426E / 0x82B44BA / 0x82B44E6 / 0x82B44F2.
    int createServerProxy(int server_count,
                          int index,
                          ENUM_SERVER_GROUP server_group,
                          char* server_ip,
                          int udp_port,
                          int unused_tcp_port);
    int GetStartIndex();
    int GetEndIndex();
    int GetNextIndex(int& index);

    ENUM_SERVER_GROUP* server_groups_;              // +0x00000
    CStatisticServerProxy* proxies_;                // +0x00004
    int proxy_count_;                               // +0x00008
    uint8_t has_proxy_array_;                       // +0x0000C
    uint8_t reserved_0d_[3];                        // +0x0000D
    CStatisticServerProxy default_proxy_;           // +0x00010
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CStatisticServerProxy) == 0x10u,
              "statistic proxy must retain the old 16-byte ABI");
static_assert(offsetof(CStatisticServerProxy, udp_handler_) == 0x08u,
              "statistic proxy UDP handler offset must stay 0x08");
static_assert(sizeof(CServerProxyMgr<CStatisticServerProxy>) == 0x20u,
              "statistic proxy manager must retain the old 32-byte ABI");
static_assert(offsetof(CServerProxyMgr<CStatisticServerProxy>, default_proxy_) ==
                  0x10u,
              "statistic proxy default offset must stay 0x10");
#endif

#endif  // OLDSRV_GLOBAL_STATISTIC_PROXY_ABI_H_
