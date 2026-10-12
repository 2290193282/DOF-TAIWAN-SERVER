#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_MERCENARY_INFO_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_MERCENARY_INFO_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD442 uses the already-restored MSG_BASE/ParamBase/state-2 template
// instance.  Its three concrete slots deliberately have no new message or
// result layout: read is a literal zero return and send emits the user-owned
// MercenaryInfo packet directly.
class Dispatcher_Mercenary_Info
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)> {
public:
    // IDA_EA: 0x85979BA
    // IDA_NAME: _ZN25Dispatcher_Mercenary_InfoC2Ev
    // DEMANGLED: Dispatcher_Mercenary_Info::Dispatcher_Mercenary_Info(void)
    Dispatcher_Mercenary_Info();

    // IDA_EA: 0x82446FE
    // IDA_NAME: _ZN25Dispatcher_Mercenary_InfoD2Ev
    // DEMANGLED: Dispatcher_Mercenary_Info::~Dispatcher_Mercenary_Info()
    // IDA_EA: 0x824472E
    // IDA_NAME: _ZN25Dispatcher_Mercenary_InfoD0Ev
    // DEMANGLED: Dispatcher_Mercenary_Info::~Dispatcher_Mercenary_Info()
    virtual ~Dispatcher_Mercenary_Info();

    // IDA_EA: 0x81DDB3C
    // IDA_NAME: _ZN25Dispatcher_Mercenary_Info4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Mercenary_Info::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DDB46
    // IDA_NAME: _ZN25Dispatcher_Mercenary_Info7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Mercenary_Info::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DDB7C
    // IDA_NAME: _ZN25Dispatcher_Mercenary_Info4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Mercenary_Info::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Mercenary_Info) == 0x04u,
              "CMD442 dispatcher must retain its old vptr-only object ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_MERCENARY_INFO_H_
