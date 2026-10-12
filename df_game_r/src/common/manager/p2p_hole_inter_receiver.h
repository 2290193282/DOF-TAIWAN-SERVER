#ifndef OLDSRV_GLOBAL_P2P_HOLE_INTER_RECEIVER_H_
#define OLDSRV_GLOBAL_P2P_HOLE_INTER_RECEIVER_H_

#include <stddef.h>
#include <stdint.h>

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/packet/PacketHeader.h"


// Original 0x35-byte statistic packet used only by the code-15 P2P internal
// receiver.  PacketHeader is the exact old packed 10-byte server header.
#pragma pack(push, 1)
struct Packet_GameServer2Statisctics2DBServer : public PacketHeader {
    uint16_t server_group;
    uint8_t p2p_state;
    uint32_t p2p_value_0e;
    uint32_t p2p_value_12;
    char country_code[16];
    char peer_ip[16];

    // IDA_EA: 0x8161DAE
    // IDA_NAME: _ZN38Packet_GameServer2Statisctics2DBServerC2Ev
    // DEMANGLED: Packet_GameServer2Statisctics2DBServer::Packet_GameServer2Statisctics2DBServer(void)
    Packet_GameServer2Statisctics2DBServer();
};
#pragma pack(pop)

static_assert(sizeof(Packet_GameServer2Statisctics2DBServer) == 0x35u,
              "P2P internal statistic packet must retain its 53-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(Packet_GameServer2Statisctics2DBServer, server_group) == 0x0au &&
                  offsetof(Packet_GameServer2Statisctics2DBServer, p2p_state) == 0x0cu &&
                  offsetof(Packet_GameServer2Statisctics2DBServer, p2p_value_0e) == 0x0du &&
                  offsetof(Packet_GameServer2Statisctics2DBServer, p2p_value_12) == 0x11u &&
                  offsetof(Packet_GameServer2Statisctics2DBServer, country_code) == 0x15u &&
                  offsetof(Packet_GameServer2Statisctics2DBServer, peer_ip) == 0x25u,
              "P2P internal statistic packet offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


class InterUpdateP2pHolePunchingState
    : public TaiwanInterfaceInterDispatcher {
public:
    // IDA_EA: 0x81621E2
    // IDA_NAME: _ZN31InterUpdateP2pHolePunchingStateC2Ev
    // DEMANGLED: InterUpdateP2pHolePunchingState::InterUpdateP2pHolePunchingState(void)
    InterUpdateP2pHolePunchingState();

    // IDA_EA: 0x8162206
    // IDA_NAME: _ZN31InterUpdateP2pHolePunchingStateD2Ev
    // DEMANGLED: InterUpdateP2pHolePunchingState::~InterUpdateP2pHolePunchingState()
    // IDA_EA: 0x8162236
    // IDA_NAME: _ZN31InterUpdateP2pHolePunchingStateD0Ev
    // DEMANGLED: InterUpdateP2pHolePunchingState::~InterUpdateP2pHolePunchingState()
    virtual ~InterUpdateP2pHolePunchingState();

    // IDA_EA: 0x8161952
    // IDA_NAME: _ZN31InterUpdateP2pHolePunchingState12dispatch_sigEP5CUserPci
    // DEMANGLED: InterUpdateP2pHolePunchingState::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


#if defined(__i386__)
static_assert(sizeof(InterUpdateP2pHolePunchingState) == 0x08u,
              "P2P internal receiver must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_P2P_HOLE_INTER_RECEIVER_H_
