#ifndef OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_RESPONSE_CREATURE_H_
#define OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_RESPONSE_CREATURE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// 140828 CMD101 / RESPONSE_CREATURE.
//
// The 130627 dispatcher used the same state-0 MSG_BASE base but accepted an
// empty request.  The 140828 client writes the requesting scene object's u16
// unique_id.  This concrete owner consumes and validates that native field;
// successful completion is the existing area-scoped NOTI104, not a same-CMD
// response.
class Dispatcher_ResponseCreature
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595F3C (130627 constructor identity)
    // IDA_NAME: _ZN27Dispatcher_ResponseCreatureC2Ev
    // DEMANGLED: Dispatcher_ResponseCreature::Dispatcher_ResponseCreature(void)
    Dispatcher_ResponseCreature();

    // IDA_EA: 0x8242DB4 / 0x8242DE4 (130627 destructor identities)
    virtual ~Dispatcher_ResponseCreature();

    // IDA_EA: 0x82037FA (130627 owner; native 140828 read added here)
    // IDA_NAME: _ZN27Dispatcher_ResponseCreature12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ResponseCreature::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233710
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823371A
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ResponseCreature) == 0x04u,
              "CMD101 dispatcher must retain its vptr-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_RESPONSE_CREATURE_H_
