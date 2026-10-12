#ifndef OLDSRV_GLOBAL_BASE_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_BASE_SERVER_PROXY_ABI_H_

#include <stdint.h>

#include <string>

#include "infra/network/transport/network_endpoint.h"

// IDA unit: 99_misc_global/BaseServerProxy.
// IDA_EA: ctor/dtor/init/tcp/send/accessors
// @0x82FD350/@0x82AA92A/@0x82FD460/@0x82FD47C/@0x82FD830/
// @0x82F7ED6/@0x82FDD6C/@0x847091A/@0x847092C/@0x847093A.
// The old class lives in the global namespace and is inherited by the auction
// proxy classes used by GlobalData.
class BaseServerProxy {
public:
    enum ENUM_SOCKET_STATE {
        SOCKET_STATE_DISCONNECTED = 0,
        SOCKET_STATE_CONNECTING = 1,
        SOCKET_STATE_CONNECTED = 2,
    };

    BaseServerProxy(char* server_ip, int server_port, char* server_name);
    virtual ~BaseServerProxy();

    CNetwork100000_100000* GetTcpHandler();
    int Init();
    int InitTcp(bool nonblock);
    virtual int Disconnect();
    virtual BaseServerProxy* OnDisconnect();
    virtual int Parsing(int received_bytes) = 0;
    bool SendTcpPacket(char* packet, int size);
    virtual int OnTcpRecv();
    virtual int OnTcpSend();
    virtual int OnTcpError();
    std::string GetServerName();
    BaseServerProxy* SetSocketState(ENUM_SOCKET_STATE state);
    ENUM_SOCKET_STATE GetSocketState() const;
    std::string GetServerIP();

protected:
    CNetwork100000_100000 tcp_handler_;
    ENUM_SOCKET_STATE socket_state_;
    std::string server_ip_;
    int server_port_;
    std::string server_name_;

private:
    BaseServerProxy(const BaseServerProxy&);
    BaseServerProxy& operator=(const BaseServerProxy&);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(BaseServerProxy) == 0x30E0Cu,
              "BaseServerProxy IDA ABI size must stay 0x30E0C");
#endif

#endif  // OLDSRV_GLOBAL_BASE_SERVER_PROXY_ABI_H_
