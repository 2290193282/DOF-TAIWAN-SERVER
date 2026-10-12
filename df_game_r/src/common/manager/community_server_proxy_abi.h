#ifndef OLDSRV_GLOBAL_COMMUNITY_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_COMMUNITY_SERVER_PROXY_ABI_H_

#include <stdint.h>

#include <string>

#include "infra/network/transport/network_endpoint.h"

// IDA unit: 99_misc_global/CCommunityServerProxy.  The original socket-state
// enum is global (not nested in BaseServerProxy); the three observed values
// correspond to disconnected, connecting, and connected respectively.
enum ENUM_SOCKET_STATE {
    SOCKET_STATE_DISCONNECTED = 0,
    SOCKET_STATE_CONNECTING = 1,
    SOCKET_STATE_CONNECTED = 2,
};

// IDA unit: 99_misc_global/CCommunityServerProxy.  The class starts with the
// exact CNetwork<100000,100000> object.  Its socket state, host string, and
// port follow at +0x30DF8, +0x30DFC, and +0x30E00 in the old i386 ABI.
class CCommunityServerProxy {
public:
    // IDA_EA: 0x846C5C8
    // IDA_NAME: _ZN21CCommunityServerProxyC2EPci
    // DEMANGLED: CCommunityServerProxy::CCommunityServerProxy(char *,int)
    CCommunityServerProxy(char* community_server_ip, int community_server_port);

    // IDA_EA: 0x846C770
    // IDA_NAME: _ZN21CCommunityServerProxy10DisconnectEv
    // DEMANGLED: CCommunityServerProxy::Disconnect(void)
    CCommunityServerProxy* Disconnect();

    // IDA_EA: 0x846C7C4
    // IDA_NAME: _ZN21CCommunityServerProxy13SendTcpPacketEPci
    // DEMANGLED: CCommunityServerProxy::SendTcpPacket(char *,int)
    int SendTcpPacket(char* packet, int size);

    // IDA_EA: 0x846C82C
    // IDA_NAME: _ZN21CCommunityServerProxy9OnTcpRecvEv
    // DEMANGLED: CCommunityServerProxy::OnTcpRecv(void)
    int OnTcpRecv();

    // IDA_EA: 0x846C8C4
    // IDA_NAME: _ZN21CCommunityServerProxy9OnTcpSendEv
    // DEMANGLED: CCommunityServerProxy::OnTcpSend(void)
    int OnTcpSend();

    // IDA_EA: 0x846C962
    // IDA_NAME: _ZN21CCommunityServerProxy10OnTcpErrorEv
    // DEMANGLED: CCommunityServerProxy::OnTcpError(void)
    int OnTcpError();

    // IDA_EA: 0x846C9EC
    // IDA_NAME: _ZN21CCommunityServerProxy7ParsingEi
    // DEMANGLED: CCommunityServerProxy::Parsing(int)
    int Parsing(int received_bytes);

    // IDA_EA: 0x846CB80
    // IDA_NAME: _ZN21CCommunityServerProxy16SendPvPBuddyListEjj
    // DEMANGLED: CCommunityServerProxy::SendPvPBuddyList(unsigned int,unsigned int)
    int SendPvPBuddyList(uint32_t acc_id, uint32_t charac_no);

    // IDA_EA: 0x846CBBC
    // IDA_NAME: _ZN21CCommunityServerProxy18SendReqAddPvPBuddyEjjcPci
    // DEMANGLED: CCommunityServerProxy::SendReqAddPvPBuddy(unsigned int,unsigned int,char,char *,int)
    int SendReqAddPvPBuddy(uint32_t acc_id,
                           uint32_t charac_no,
                           char request_kind,
                           char* target_name,
                           int target_name_size);

    // IDA_EA: 0x846CC22
    // IDA_NAME: _ZN21CCommunityServerProxy18SendResAddPvPBuddyEjjcPci
    // DEMANGLED: CCommunityServerProxy::SendResAddPvPBuddy(unsigned int,unsigned int,char,char *,int)
    int SendResAddPvPBuddy(uint32_t acc_id,
                           uint32_t charac_no,
                           char response_kind,
                           char* target_name,
                           int target_name_size);

    // IDA_EA: 0x846CC88
    // IDA_NAME: _ZN21CCommunityServerProxy18SendRemovePvPBuddyEjjcPci
    // DEMANGLED: CCommunityServerProxy::SendRemovePvPBuddy(unsigned int,unsigned int,char,char *,int)
    int SendRemovePvPBuddy(uint32_t acc_id,
                           uint32_t charac_no,
                           char remove_kind,
                           char* target_name,
                           int target_name_size);

    // IDA_EA: 0x846D048
    // IDA_NAME: _ZN21CCommunityServerProxy13GetTcpHandlerEv
    // DEMANGLED: CCommunityServerProxy::GetTcpHandler(void)
    CNetwork100000_100000* GetTcpHandler();

    // IDA_EA: 0x857F464
    // IDA_NAME: _ZN21CCommunityServerProxy14SetSocketStateE17ENUM_SOCKET_STATE
    // DEMANGLED: CCommunityServerProxy::SetSocketState(ENUM_SOCKET_STATE)
    CCommunityServerProxy* SetSocketState(ENUM_SOCKET_STATE state);

    // IDA_EA: 0x857F476
    // IDA_NAME: _ZN21CCommunityServerProxy14GetSocketStateEv
    // DEMANGLED: CCommunityServerProxy::GetSocketState(void)
    ENUM_SOCKET_STATE GetSocketState();

private:
    CNetwork100000_100000 tcp_handler_;
    ENUM_SOCKET_STATE socket_state_;
    std::string server_ip_;
    int server_port_;
};

#if defined(__i386__)
static_assert(sizeof(CNetwork100000_100000) == 0x30DF8u,
              "Community proxy TCP endpoint must retain the old i386 extent");
static_assert(sizeof(CCommunityServerProxy) == 0x30E04u,
              "CCommunityServerProxy must retain the old i386 extent");
#endif

#endif  // OLDSRV_GLOBAL_COMMUNITY_SERVER_PROXY_ABI_H_
