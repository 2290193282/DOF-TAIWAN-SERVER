#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_COMPOUND_AVATAR_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_COMPOUND_AVATAR_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD102 is a direct state-0 dispatcher.  Its original dispatch_sig owns the
// full compact request read and immediately hands it to CCompoundAvatar; the
// template read/process slots are intentionally no-ops.
class Dispatcher_Compound_Avatar
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595F04
    Dispatcher_Compound_Avatar();
    // IDA_EA: 0x82438AC / 0x82438DC
    virtual ~Dispatcher_Compound_Avatar();

    // IDA_EA: 0x81EAD40
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x82336FC
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x8233706
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Compound_Avatar) == 0x04u,
              "CMD102 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_COMPOUND_AVATAR_H_
