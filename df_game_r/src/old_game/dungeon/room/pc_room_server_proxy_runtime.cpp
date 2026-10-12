#include "old_game/dungeon/room/pc_room_server_proxy_runtime.h"

#include <cerrno>
#include <cstring>

#include "infra/log/format/trace.h"

// IDA_EA: 0x84717DC
// IDA_NAME: _ZN18CPCRoomServerProxyC2EPci
// DEMANGLED: CPCRoomServerProxy::CPCRoomServerProxy(char *,int)
CPCRoomServerProxy::CPCRoomServerProxy(char* pcroom_server_ip,
                                       int pcroom_server_port)
    : tcp_handler_(),
      server_ip_(pcroom_server_ip),
      server_port_(pcroom_server_port),
      socket_state_(SOCKET_STATE_DISCONNECTED) {}

// IDA_EA: 0x8471874
// IDA_NAME: _ZN18CPCRoomServerProxyD2Ev
// DEMANGLED: CPCRoomServerProxy::~CPCRoomServerProxy()
CPCRoomServerProxy::~CPCRoomServerProxy() {}

// IDA_EA: 0x84718BC
// IDA_NAME: _ZN18CPCRoomServerProxy4InitEv
// DEMANGLED: CPCRoomServerProxy::Init(void)
bool CPCRoomServerProxy::Init() { return InitTcp() == 1; }

// IDA_EA: 0x84718E2
// IDA_NAME: _ZN18CPCRoomServerProxy7InitTcpEv
// DEMANGLED: CPCRoomServerProxy::InitTcp(void)
int CPCRoomServerProxy::InitTcp() {
    if (server_ip_.empty() || !server_port_) {
        return 0;
    }
    if (tcp_handler_.get_socket() >= 0) {
        return 1;
    }

    int& socket = *reinterpret_cast<int*>(&tcp_handler_);
    const int result = CNetwork100000_100000::CreateConnectionSocket(
        socket, server_ip_.c_str(), static_cast<int>(server_port_));
    if (result == 0) {
        return 1;
    }

    cMyTrace trace("InitTcp", 278, 0);
    trace("CPCRoomServerProxy CreateConnectionSocket() error(%d:%d:%s)",
          result,
          errno,
          std::strerror(errno));
    return 0;
}

// IDA_EA: 0x84719DE
// IDA_NAME: _ZN18CPCRoomServerProxy10DisconnectEv
// DEMANGLED: CPCRoomServerProxy::Disconnect(void)
CPCRoomServerProxy* CPCRoomServerProxy::Disconnect() {
    if (!server_ip_.empty() && server_port_) {
        tcp_handler_.disconnect();
        socket_state_ = SOCKET_STATE_DISCONNECTED;
    }
    return this;
}

// IDA_EA: 0x8471A32
// IDA_NAME: _ZN18CPCRoomServerProxy13SendTcpPacketEPci
// DEMANGLED: CPCRoomServerProxy::SendTcpPacket(char *,int)
int CPCRoomServerProxy::SendTcpPacket(char* packet, int size) {
    if (socket_state_ == SOCKET_STATE_CONNECTED) {
        return tcp_handler_.send_packet(packet, size);
    }

    cMyTrace trace("SendTcpPacket", 312, 0);
    return trace("cannot send packet because PCRoom Server is disconnected");
}

// IDA_EA: 0x8471A9A
// IDA_NAME: _ZN18CPCRoomServerProxy9OnTcpRecvEv
// DEMANGLED: CPCRoomServerProxy::OnTcpRecv(void)
int CPCRoomServerProxy::OnTcpRecv() {
    const int received_bytes = tcp_handler_.recv_packet();
    if (received_bytes <= 0) {
        if (received_bytes >= 0) {
            return 1;
        }
        LogManager::logFormat(1,
                              "DF_PCRoomServerProxy.cpp",
                              "OnTcpRecv",
                              0x155,
                              "PCRoom Server maybe Die!!!");
        Disconnect();
        return 0;
    }
    if (Parsing(received_bytes) == 1) {
        return 1;
    }
    Disconnect();
    return 0;
}

// IDA_EA: 0x8471B32
// IDA_NAME: _ZN18CPCRoomServerProxy9OnTcpSendEv
// DEMANGLED: CPCRoomServerProxy::OnTcpSend(void)
int CPCRoomServerProxy::OnTcpSend() {
    if (tcp_handler_.send_packet() >= 0) {
        return static_cast<int>(socket_state_);
    }

    cMyTrace trace("OnTcpSend", 365, 0);
    trace("PCRoom OnSend() Socket[%d] Error(%s)",
          tcp_handler_.get_socket(),
          std::strerror(errno));
    Disconnect();
    return -1;
}

// IDA_EA: 0x8471BD0
// IDA_NAME: _ZN18CPCRoomServerProxy10OnTcpErrorEv
// DEMANGLED: CPCRoomServerProxy::OnTcpError(void)
int CPCRoomServerProxy::OnTcpError() {
    if (errno == 0) {
        return 1;
    }

    cMyTrace trace("OnTcpError", 383, 0);
    trace("PCRoomServer OnError() Socket[%d] Error(%s)",
          tcp_handler_.get_socket(),
          std::strerror(errno));
    return 0;
}

// IDA_EA: 0x8471C5A
// IDA_NAME: _ZN18CPCRoomServerProxy7ParsingEi
// DEMANGLED: CPCRoomServerProxy::Parsing(int)
int CPCRoomServerProxy::Parsing(int received_bytes) {
    return tcp_handler_.Parsing_ServerPacket(received_bytes);
}

// IDA_EA: 0x8471E08
// IDA_NAME: _ZN18CPCRoomServerProxy13GetTcpHandlerEv
// DEMANGLED: CPCRoomServerProxy::GetTcpHandler(void)
CNetwork100000_100000* CPCRoomServerProxy::GetTcpHandler() {
    return &tcp_handler_;
}

// IDA_EA: 0x857F444
// IDA_NAME: _ZN18CPCRoomServerProxy14SetSocketStateE17ENUM_SOCKET_STATE
// DEMANGLED: CPCRoomServerProxy::SetSocketState(ENUM_SOCKET_STATE)
CPCRoomServerProxy* CPCRoomServerProxy::SetSocketState(
    ENUM_SOCKET_STATE state) {
    socket_state_ = state;
    return this;
}

// IDA_EA: 0x857F456
// IDA_NAME: _ZN18CPCRoomServerProxy14GetSocketStateEv
// DEMANGLED: CPCRoomServerProxy::GetSocketState(void)
ENUM_SOCKET_STATE CPCRoomServerProxy::GetSocketState() {
    return socket_state_;
}
