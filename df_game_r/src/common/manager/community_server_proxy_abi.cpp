#include "common/manager/community_server_proxy_abi.h"

#include "common/manager/global_data.h"

#include <cerrno>
#include <cstring>

#include "infra/log/format/trace.h"
#include "old_game/progression/quest/quest_skill_packets.h"

// IDA_DATA: GlobalData::s_community_proxy
// IDA_NAME: _ZN10GlobalData17s_community_proxyE
CCommunityServerProxy* GlobalData::s_community_proxy = 0;

// IDA_EA: 0x846C5C8
// IDA_NAME: _ZN21CCommunityServerProxyC2EPci
// DEMANGLED: CCommunityServerProxy::CCommunityServerProxy(char *,int)
CCommunityServerProxy::CCommunityServerProxy(char* community_server_ip,
                                             int community_server_port)
    : tcp_handler_(),
      socket_state_(SOCKET_STATE_DISCONNECTED),
      server_ip_(community_server_ip),
      server_port_(community_server_port) {}

// IDA_EA: 0x846C770
// IDA_NAME: _ZN21CCommunityServerProxy10DisconnectEv
// DEMANGLED: CCommunityServerProxy::Disconnect(void)
CCommunityServerProxy* CCommunityServerProxy::Disconnect() {
    if (!server_ip_.empty() && server_port_) {
        tcp_handler_.disconnect();
        socket_state_ = SOCKET_STATE_DISCONNECTED;
    }
    return this;
}

// IDA_EA: 0x846C7C4
// IDA_NAME: _ZN21CCommunityServerProxy13SendTcpPacketEPci
// DEMANGLED: CCommunityServerProxy::SendTcpPacket(char *,int)
int CCommunityServerProxy::SendTcpPacket(char* packet, int size) {
    if (socket_state_ == SOCKET_STATE_CONNECTED) {
        return tcp_handler_.send_packet(packet, size);
    }

    cMyTrace trace(
        "void CCommunityServerProxy::SendTcpPacket(char*, int)", 212, 0);
    return trace("cannot send packet because Community Server is disconnected");
}

// IDA_EA: 0x846C82C
// IDA_NAME: _ZN21CCommunityServerProxy9OnTcpRecvEv
// DEMANGLED: CCommunityServerProxy::OnTcpRecv(void)
int CCommunityServerProxy::OnTcpRecv() {
    const int received_bytes = tcp_handler_.recv_packet();
    if (received_bytes <= 0) {
        if (received_bytes >= 0) {
            return 1;
        }
        LogManager::logFormat(1,
                              "DF_CommunityServerProxy.cpp",
                              "bool CCommunityServerProxy::OnTcpRecv()",
                              0xED,
                              "Community Server maybe Die!!!");
        Disconnect();
        return 0;
    }
    if (Parsing(received_bytes) == 1) {
        return 1;
    }
    Disconnect();
    return 0;
}

// IDA_EA: 0x846C8C4
// IDA_NAME: _ZN21CCommunityServerProxy9OnTcpSendEv
// DEMANGLED: CCommunityServerProxy::OnTcpSend(void)
int CCommunityServerProxy::OnTcpSend() {
    if (tcp_handler_.send_packet() >= 0) {
        return static_cast<int>(socket_state_);
    }

    cMyTrace trace("int CCommunityServerProxy::OnTcpSend()", 257, 0);
    trace("CommunityServer OnSend() Socket[%d] Error(%s)",
          tcp_handler_.get_socket(),
          std::strerror(errno));
    Disconnect();
    return -1;
}

// IDA_EA: 0x846C962
// IDA_NAME: _ZN21CCommunityServerProxy10OnTcpErrorEv
// DEMANGLED: CCommunityServerProxy::OnTcpError(void)
int CCommunityServerProxy::OnTcpError() {
    if (errno == 0) {
        return 1;
    }

    cMyTrace trace("bool CCommunityServerProxy::OnTcpError()", 271, 0);
    trace("CommunityServer OnError() Socket[%d] Error(%s)",
          tcp_handler_.get_socket(),
          std::strerror(errno));
    return 0;
}

// IDA_EA: 0x846C9EC
// IDA_NAME: _ZN21CCommunityServerProxy7ParsingEi
// DEMANGLED: CCommunityServerProxy::Parsing(int)
int CCommunityServerProxy::Parsing(int received_bytes) {
    return tcp_handler_.Parsing_ServerPacket(received_bytes);
}

// IDA_EA: 0x846CB80
// IDA_NAME: _ZN21CCommunityServerProxy16SendPvPBuddyListEjj
// DEMANGLED: CCommunityServerProxy::SendPvPBuddyList(unsigned int,unsigned int)
int CCommunityServerProxy::SendPvPBuddyList(uint32_t acc_id,
                                             uint32_t charac_no) {
    Packet_Request_PvP_Buddy_Conn_List packet;
    packet.dword_0a = acc_id;
    packet.dword_0e = charac_no;

    char wire_bytes[sizeof(packet)];
    std::memcpy(wire_bytes, &packet, sizeof(wire_bytes));
    return SendTcpPacket(wire_bytes, packet.size);
}

// IDA_EA: 0x846CBBC
// IDA_NAME: _ZN21CCommunityServerProxy18SendReqAddPvPBuddyEjjcPci
// DEMANGLED: CCommunityServerProxy::SendReqAddPvPBuddy(unsigned int,unsigned int,char,char *,int)
int CCommunityServerProxy::SendReqAddPvPBuddy(uint32_t acc_id,
                                               uint32_t charac_no,
                                               char request_kind,
                                               char* target_name,
                                               int target_name_size) {
    Packet_Request_Add_PvP_Buddy packet;
    packet.acc_id = acc_id;
    packet.charac_no = charac_no;
    packet.request_kind = request_kind;
    std::memcpy(packet.target_name, target_name,
                static_cast<size_t>(target_name_size));

    char wire_bytes[sizeof(packet)];
    std::memcpy(wire_bytes, &packet, sizeof(wire_bytes));
    return SendTcpPacket(wire_bytes, packet.size);
}

// IDA_EA: 0x846CC22
// IDA_NAME: _ZN21CCommunityServerProxy18SendResAddPvPBuddyEjjcPci
// DEMANGLED: CCommunityServerProxy::SendResAddPvPBuddy(unsigned int,unsigned int,char,char *,int)
int CCommunityServerProxy::SendResAddPvPBuddy(uint32_t acc_id,
                                               uint32_t charac_no,
                                               char response_kind,
                                               char* target_name,
                                               int target_name_size) {
    Packet_Response_Add_PvP_Buddy packet;
    packet.acc_id = acc_id;
    packet.charac_no = charac_no;
    packet.response_kind = response_kind;
    std::memcpy(packet.target_name, target_name,
                static_cast<size_t>(target_name_size));

    char wire_bytes[sizeof(packet)];
    std::memcpy(wire_bytes, &packet, sizeof(wire_bytes));
    return SendTcpPacket(wire_bytes, packet.size);
}

// IDA_EA: 0x846CC88
// IDA_NAME: _ZN21CCommunityServerProxy18SendRemovePvPBuddyEjjcPci
// DEMANGLED: CCommunityServerProxy::SendRemovePvPBuddy(unsigned int,unsigned int,char,char *,int)
int CCommunityServerProxy::SendRemovePvPBuddy(uint32_t acc_id,
                                               uint32_t charac_no,
                                               char remove_kind,
                                               char* target_name,
                                               int target_name_size) {
    Packet_Request_Remove_PvP_Buddy packet;
    packet.dword_0a = acc_id;
    packet.dword_0e = charac_no;
    packet.byte_12 = static_cast<uint8_t>(remove_kind);
    std::memcpy(packet.raw_13, target_name,
                static_cast<size_t>(target_name_size));

    char wire_bytes[sizeof(packet)];
    std::memcpy(wire_bytes, &packet, sizeof(wire_bytes));
    return SendTcpPacket(wire_bytes, packet.size);
}

// IDA_EA: 0x846D048
// IDA_NAME: _ZN21CCommunityServerProxy13GetTcpHandlerEv
// DEMANGLED: CCommunityServerProxy::GetTcpHandler(void)
CNetwork100000_100000* CCommunityServerProxy::GetTcpHandler() {
    return &tcp_handler_;
}

// IDA_EA: 0x857F464
// IDA_NAME: _ZN21CCommunityServerProxy14SetSocketStateE17ENUM_SOCKET_STATE
// DEMANGLED: CCommunityServerProxy::SetSocketState(ENUM_SOCKET_STATE)
CCommunityServerProxy* CCommunityServerProxy::SetSocketState(
    ENUM_SOCKET_STATE state) {
    socket_state_ = state;
    return this;
}

// IDA_EA: 0x857F476
// IDA_NAME: _ZN21CCommunityServerProxy14GetSocketStateEv
// DEMANGLED: CCommunityServerProxy::GetSocketState(void)
ENUM_SOCKET_STATE CCommunityServerProxy::GetSocketState() {
    return socket_state_;
}
