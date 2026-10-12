#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COMPOUND_EQUIPMENT_UPGRADE_CARD_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COMPOUND_EQUIPMENT_UPGRADE_CARD_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
class PacketGuard;

// CMD361 is a direct dispatcher override, not a read/process template body.
// The three unsigned-short slots are consumed without PacketBuf failure
// branches by the original handler.
class Dispatcher_Compound_Equipment_Upgrade_Card
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597068
    Dispatcher_Compound_Equipment_Upgrade_Card();
    // IDA_EA: 0x8240138 / 0x8240168
    virtual ~Dispatcher_Compound_Equipment_Upgrade_Card();
    // IDA_EA: 0x8227E00
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x82342EE / 0x82342F8
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x8228A2E
    int set_ReturnSendPacket(PacketGuard& packet, unsigned short slot,
                             int remaining_count) const;
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Compound_Equipment_Upgrade_Card) == 0x04u,
              "CMD361 dispatcher must retain its old vptr-only ABI");
#endif

#endif
