#include "common/manager/base_server_proxy_abi.h"

#include <errno.h>
#include <string.h>

#include "infra/log/format/trace.h"

// IDA_EA: 0x82FD350
// IDA_NAME: _ZN15BaseServerProxyC2EPciS0_
// DEMANGLED: BaseServerProxy::BaseServerProxy(char *,int,char *)
BaseServerProxy::BaseServerProxy(char* server_ip,
                                 int server_port,
                                 char* server_name)
    : tcp_handler_(),
      socket_state_(SOCKET_STATE_DISCONNECTED),
      server_ip_(server_ip ? server_ip : ""),
      server_port_(server_port),
      server_name_(server_name ? server_name : "") {}

// IDA_EA: 0x82AA92A
// IDA_NAME: _ZN15BaseServerProxyD2Ev
// DEMANGLED: BaseServerProxy::~BaseServerProxy()
BaseServerProxy::~BaseServerProxy() {}

// IDA_EA: 0x82F7ED6
// IDA_NAME: _ZN15BaseServerProxy13GetTcpHandlerEv
// DEMANGLED: BaseServerProxy::GetTcpHandler(void)
CNetwork100000_100000* BaseServerProxy::GetTcpHandler() {
    return &tcp_handler_;
}

// IDA_EA: 0x82FD460
// IDA_NAME: _ZN15BaseServerProxy4InitEv
// DEMANGLED: BaseServerProxy::Init(void)
int BaseServerProxy::Init() {
    return InitTcp(true);
}

// IDA_EA: 0x82FD47C
// IDA_NAME: _ZN15BaseServerProxy7InitTcpEb
// DEMANGLED: BaseServerProxy::InitTcp(bool)
int BaseServerProxy::InitTcp(bool nonblock) {
    if (server_ip_.empty() || server_port_ == 0) {
        return 0;
    }

    if (tcp_handler_.get_socket() < 0) {
        // IDA passes the CNetwork object head as the output fd slot.
        int& fd = *reinterpret_cast<int*>(&tcp_handler_);
        const int result = nonblock
                               ? CNetwork100000_100000::
                                     CreateConnectionSocket(
                                         fd, server_ip_.c_str(), server_port_)
                               : CNetwork100000_100000::
                                     CreateConnectionBlockSocket(
                                         fd, server_ip_.c_str(), server_port_);
        if (result != 0) {
            const int error_no = errno;
            cMyTrace trace(
                "bool BaseServerProxy::InitTcp(bool)", 308, 0);
            trace("%s CreateConnectionSocket() error(%d:%d:%s)",
                  server_name_.c_str(),
                  result,
                  error_no,
                  strerror(error_no));
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x82FD6E4
// IDA_NAME: _ZN15BaseServerProxy10DisconnectEv
// DEMANGLED: BaseServerProxy::Disconnect(void)
int BaseServerProxy::Disconnect() {
    cMyTrace trace(
        "virtual void BaseServerProxy::Disconnect()", 328, 0);
    trace("BaseServerProxy::Disconnect(), ServerName:%s,SocketState:%d,IP:%s,Port:%d,Socket:%d",
          server_name_.c_str(),
          static_cast<int>(socket_state_),
          server_ip_.c_str(),
          server_port_,
          tcp_handler_.get_socket());
    if (server_ip_.empty() || server_port_ == 0) {
        return 1;
    }
    tcp_handler_.disconnect();
    socket_state_ = SOCKET_STATE_DISCONNECTED;
    OnDisconnect();
    return 0;
}

// IDA_EA: 0x82FD82A
// IDA_NAME: _ZN15BaseServerProxy12OnDisconnectEv
// DEMANGLED: BaseServerProxy::OnDisconnect(void)
BaseServerProxy* BaseServerProxy::OnDisconnect() {
    return this;
}

// IDA_EA: 0x82FD830
// IDA_NAME: _ZN15BaseServerProxy13SendTcpPacketEPci
// DEMANGLED: BaseServerProxy::SendTcpPacket(char *,int)
bool BaseServerProxy::SendTcpPacket(char* packet, int size) {
    if (socket_state_ == SOCKET_STATE_CONNECTED) {
        return tcp_handler_.send_packet(packet, size) >= 0;
    }
    cMyTrace trace(
        "bool BaseServerProxy::SendTcpPacket(char*, int)", 349, 0);
    trace("cannot send packet because %s is disconnected",
          server_name_.c_str());
    return false;
}

// IDA_EA: 0x82FD90E
// IDA_NAME: _ZN15BaseServerProxy9OnTcpRecvEv
// DEMANGLED: BaseServerProxy::OnTcpRecv(void)
int BaseServerProxy::OnTcpRecv() {
    const int received = tcp_handler_.recv_packet();
    if (received <= 0) {
        if (received >= 0) {
            return 1;
        }
        LogManager::logFormat(
            1,
            "BaseServerProxy.cpp",
            "bool BaseServerProxy::OnTcpRecv()",
            0x17A,
            "%s maybe Die!!!",
            server_name_.c_str());
        // IDA 0x82FDA81..0x82FDA91 calls vtable +0x04, resolved by the
        // BaseServerProxy vtable to Disconnect(), not OnDisconnect().
        Disconnect();
        return 0;
    }
    if (Parsing(received) == 1) {
        return 1;
    }
    cMyTrace trace("bool BaseServerProxy::OnTcpRecv()", 367, 0);
    trace("%s occur Error at parsing receive packet", server_name_.c_str());
    // IDA 0x82FD9D9..0x82FD9E9 likewise dispatches vtable +0x04.
    Disconnect();
    return 0;
}

// IDA_EA: 0x82FDAAA
// IDA_NAME: _ZN15BaseServerProxy9OnTcpSendEv
// DEMANGLED: BaseServerProxy::OnTcpSend(void)
int BaseServerProxy::OnTcpSend() {
    const int result = tcp_handler_.send_packet();
    if (result >= 0) {
        return static_cast<int>(socket_state_);
    }
    cMyTrace error_trace(
        "int BaseServerProxy::OnTcpSend()", 399, 0);
    error_trace("%s OnSend() Socket[%d] Error(%s)",
                server_name_.c_str(),
                tcp_handler_.get_socket(),
                strerror(errno));
    cMyTrace disconnect_trace(
        "int BaseServerProxy::OnTcpSend()", 402, 0);
    disconnect_trace("Disconnect to %s (send_packet() returns minus value)",
                     server_name_.c_str());
    // IDA 0x82FDC04..0x82FDC14 dispatches vtable +0x04 -> Disconnect().
    Disconnect();
    return -1;
}

// IDA_EA: 0x82FDC2E
// IDA_NAME: _ZN15BaseServerProxy10OnTcpErrorEv
// DEMANGLED: BaseServerProxy::OnTcpError(void)
int BaseServerProxy::OnTcpError() {
    if (errno == 0) {
        return 1;
    }
    cMyTrace trace("bool BaseServerProxy::OnTcpError()", 415, 0);
    trace("%s OnError() Socket[%d] Error(%s)",
          server_name_.c_str(),
          tcp_handler_.get_socket(),
          strerror(errno));
    return 0;
}

// IDA_EA: 0x82FDD6C
// IDA_NAME: _ZN15BaseServerProxy13GetServerNameEv
// DEMANGLED: BaseServerProxy::GetServerName(void)
std::string BaseServerProxy::GetServerName() {
    return server_name_;
}

// IDA_EA: 0x847091A
// IDA_NAME: _ZN15BaseServerProxy14SetSocketStateE17ENUM_SOCKET_STATE
// DEMANGLED: BaseServerProxy::SetSocketState(ENUM_SOCKET_STATE)
BaseServerProxy* BaseServerProxy::SetSocketState(ENUM_SOCKET_STATE state) {
    socket_state_ = state;
    return this;
}

// IDA_EA: 0x847092C
// IDA_NAME: _ZN15BaseServerProxy14GetSocketStateEv
// DEMANGLED: BaseServerProxy::GetSocketState(void)
BaseServerProxy::ENUM_SOCKET_STATE BaseServerProxy::GetSocketState() const {
    return socket_state_;
}

// IDA_EA: 0x847093A
// IDA_NAME: _ZN15BaseServerProxy11GetServerIPEv
// DEMANGLED: BaseServerProxy::GetServerIP(void)
std::string BaseServerProxy::GetServerIP() {
    return server_ip_;
}
