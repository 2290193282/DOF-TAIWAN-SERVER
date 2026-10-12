#include "common/manager/statistic_proxy_abi.h"

#include "infra/log/format/trace.h"

#include <new>

// IDA_EA: 0x82A6B72
// IDA_NAME: _ZN21CStatisticServerProxyC2Ev
// DEMANGLED: CStatisticServerProxy::CStatisticServerProxy(void)
CStatisticServerProxy::CStatisticServerProxy()
    : server_ip_(), udp_port_(0), udp_handler_() {}

// IDA_EA: 0x8471F84
// IDA_NAME: _ZN21CStatisticServerProxyC2ESsi
// DEMANGLED: CStatisticServerProxy::CStatisticServerProxy(std::string,int)
CStatisticServerProxy::CStatisticServerProxy(std::string server_ip,
                                               int udp_port)
    : server_ip_(server_ip), udp_port_(udp_port), udp_handler_() {}

// IDA_EA: 0x82B3282
// IDA_NAME: _ZN21CStatisticServerProxyD2Ev
// DEMANGLED: CStatisticServerProxy::~CStatisticServerProxy()
CStatisticServerProxy::~CStatisticServerProxy() {}

// IDA_EA: 0x8471FDA
// IDA_NAME: _ZN21CStatisticServerProxy4InitEv
// DEMANGLED: CStatisticServerProxy::Init(void)
int CStatisticServerProxy::Init() {
    if (udp_port_ == 0) {
        return 1;
    }
    if (udp_handler_.InitClientSocket() != -1) {
        return 1;
    }
    LogManager::logFormat(1,
                          "DF_StatisticServerProxy.cpp",
                          "bool CStatisticServerProxy::Init()",
                          0x267,
                          "[CStatisticServerProxy::Init] Can't init udp client socket\n");
    return 0;
}

// IDA_EA: 0x82A6BC0
// IDA_NAME: _ZN21CStatisticServerProxy4InitESsii
// DEMANGLED: CStatisticServerProxy::Init(std::string,int,int)
int CStatisticServerProxy::Init(std::string server_ip,
                                int udp_port,
                                int unused_tcp_port) {
    (void)unused_tcp_port;
    server_ip_ = server_ip;
    udp_port_ = udp_port;
    return Init();
}

// IDA_EA: 0x82B3296
// IDA_NAME: _ZN15CServerProxyMgrI21CStatisticServerProxyEC2Ev
// DEMANGLED: CServerProxyMgr<CStatisticServerProxy>::CServerProxyMgr(void)
CServerProxyMgr<CStatisticServerProxy>::CServerProxyMgr()
    : server_groups_(0),
      proxies_(0),
      proxy_count_(0),
      has_proxy_array_(0),
      reserved_0d_{0, 0, 0},
      default_proxy_() {}

// IDA_EA: 0x82B3B8E
// IDA_NAME: _ZN15CServerProxyMgrI21CStatisticServerProxyED2Ev
// DEMANGLED: CServerProxyMgr<CStatisticServerProxy>::~CServerProxyMgr()
CServerProxyMgr<CStatisticServerProxy>::~CServerProxyMgr() {
    if (has_proxy_array_ == 1) {
        delete[] server_groups_;
        delete[] proxies_;
    }
    server_groups_ = 0;
    proxies_ = 0;
    proxy_count_ = 0;
    has_proxy_array_ = 0;
}

// IDA_EA: 0x82B426E
// IDA_NAME: _ZN15CServerProxyMgrI21CStatisticServerProxyE17createServerProxyEii17ENUM_SERVER_GROUPPcii
// DEMANGLED: CServerProxyMgr<CStatisticServerProxy>::createServerProxy(int,int,ENUM_SERVER_GROUP,char *,int,int)
int CServerProxyMgr<CStatisticServerProxy>::createServerProxy(
    int server_count,
    int index,
    ENUM_SERVER_GROUP server_group,
    char* server_ip,
    int udp_port,
    int unused_tcp_port) {
    const std::string endpoint(server_ip ? server_ip : "");
    if (server_group != SERVER_GROUP_DEFAULT) {
        if (has_proxy_array_ != 1) {
            if (server_count <= 0 || index < 0 || index >= server_count) {
                return 0;
            }
            server_groups_ = new (std::nothrow) ENUM_SERVER_GROUP[server_count];
            proxies_ = new (std::nothrow) CStatisticServerProxy[server_count];
            if (!server_groups_ || !proxies_) {
                delete[] server_groups_;
                delete[] proxies_;
                server_groups_ = 0;
                proxies_ = 0;
                return 0;
            }
            proxy_count_ = server_count;
            has_proxy_array_ = 1;
        }
        if (index < 0 || index >= proxy_count_) {
            return 0;
        }
        server_groups_[index] = server_group;
        return proxies_[index].Init(endpoint, udp_port, unused_tcp_port);
    }

    proxies_ = &default_proxy_;
    proxy_count_ = 1;
    return default_proxy_.Init(endpoint, udp_port, unused_tcp_port);
}

// IDA_EA: 0x82B44BA
// IDA_NAME: _ZN15CServerProxyMgrI21CStatisticServerProxyE13GetStartIndexEv
int CServerProxyMgr<CStatisticServerProxy>::GetStartIndex() {
    if (proxy_count_ > 0 && has_proxy_array_ == 1) {
        return static_cast<int>(server_groups_[0]);
    }
    return 0;
}

// IDA_EA: 0x82B44E6
// IDA_NAME: _ZN15CServerProxyMgrI21CStatisticServerProxyE11GetEndIndexEv
int CServerProxyMgr<CStatisticServerProxy>::GetEndIndex() {
    return proxy_count_;
}

// IDA_EA: 0x82B44F2
// IDA_NAME: _ZN15CServerProxyMgrI21CStatisticServerProxyE12GetNextIndexERi
int CServerProxyMgr<CStatisticServerProxy>::GetNextIndex(int& index) {
    ++index;
    if (has_proxy_array_ != 1 || index >= proxy_count_) {
        return 0;
    }
    return static_cast<int>(server_groups_[index]);
}

// IDA_EA: 0x8111CF8
// IDA_NAME: _ZN15CServerProxyMgrI21CStatisticServerProxyE14GetServerProxyE17ENUM_SERVER_GROUP
// DEMANGLED: CServerProxyMgr<CStatisticServerProxy>::GetServerProxy(ENUM_SERVER_GROUP)
CStatisticServerProxy*
CServerProxyMgr<CStatisticServerProxy>::GetServerProxy(
    ENUM_SERVER_GROUP server_group) {
    if (server_group == SERVER_GROUP_DEFAULT) {
        return &default_proxy_;
    }

    for (int index = 0; proxy_count_ > index; ++index) {
        if (server_groups_ != 0 && server_groups_[index] == server_group) {
            return &proxies_[index];
        }
    }
    return &default_proxy_;
}

// IDA_EA: 0x8472044
// IDA_NAME: _ZN21CStatisticServerProxy10SendPacketEPci
// DEMANGLED: CStatisticServerProxy::SendPacket(char *,int)
int CStatisticServerProxy::SendPacket(char* packet, int size) {
    if (udp_port_ == 0) {
        return 0;
    }

    return udp_handler_.SendToServer(packet,
                                     static_cast<size_t>(size),
                                     static_cast<uint16_t>(udp_port_),
                                     server_ip_.c_str());
}
