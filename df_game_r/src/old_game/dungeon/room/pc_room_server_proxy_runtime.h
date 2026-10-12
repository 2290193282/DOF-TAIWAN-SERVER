#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PC_ROOM_SERVER_PROXY_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PC_ROOM_SERVER_PROXY_RUNTIME_H_

#include <string>

#include "common/manager/community_server_proxy_abi.h"
#include "infra/network/transport/network_endpoint.h"

// IDA unit: 09_dungeon_pvp_party/CPCRoomServerProxy.  The network endpoint
// is the first subobject; the host, port and socket state follow at the
// exact old i386 offsets 0x30DF8, 0x30DFC and 0x30E00.
class CPCRoomServerProxy {
public:
    // IDA_EA: 0x84717DC
    // IDA_NAME: _ZN18CPCRoomServerProxyC2EPci
    CPCRoomServerProxy(char* pcroom_server_ip, int pcroom_server_port);

    // IDA_EA: 0x8471874
    // IDA_NAME: _ZN18CPCRoomServerProxyD2Ev
    ~CPCRoomServerProxy();

    // IDA_EA: 0x84718BC
    // IDA_NAME: _ZN18CPCRoomServerProxy4InitEv
    bool Init();

    // IDA_EA: 0x84718E2
    // IDA_NAME: _ZN18CPCRoomServerProxy7InitTcpEv
    int InitTcp();

    // IDA_EA: 0x84719DE
    // IDA_NAME: _ZN18CPCRoomServerProxy10DisconnectEv
    CPCRoomServerProxy* Disconnect();

    // IDA_EA: 0x8471A32
    // IDA_NAME: _ZN18CPCRoomServerProxy13SendTcpPacketEPci
    int SendTcpPacket(char* packet, int size);

    // IDA_EA: 0x8471A9A / 0x8471B32 / 0x8471BD0
    int OnTcpRecv();
    int OnTcpSend();
    int OnTcpError();

    // IDA_EA: 0x8471C5A / 0x8471E08
    int Parsing(int received_bytes);
    CNetwork100000_100000* GetTcpHandler();

    // IDA_EA: 0x857F444 / 0x857F456
    CPCRoomServerProxy* SetSocketState(ENUM_SOCKET_STATE state);
    ENUM_SOCKET_STATE GetSocketState();

private:
    CNetwork100000_100000 tcp_handler_;  // +0x00000
    std::string server_ip_;               // +0x30DF8
    int server_port_;                     // +0x30DFC
    ENUM_SOCKET_STATE socket_state_;      // +0x30E00
};

#if defined(__i386__)
static_assert(sizeof(CNetwork100000_100000) == 0x30DF8u,
              "PC-room proxy TCP endpoint must retain the old i386 extent");
static_assert(sizeof(CPCRoomServerProxy) == 0x30E04u,
              "CPCRoomServerProxy must retain the old i386 extent");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PC_ROOM_SERVER_PROXY_RUNTIME_H_
