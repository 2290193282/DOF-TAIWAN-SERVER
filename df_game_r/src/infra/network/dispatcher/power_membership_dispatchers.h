#ifndef OLDSRV_PROTOCOL_POWER_MEMBERSHIP_DISPATCHERS_H_
#define OLDSRV_PROTOCOL_POWER_MEMBERSHIP_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 180 / JOIN_POWER.  The old registration table allocates only the
// four-byte i386 IPacketDispatcher base for this class.
class Dispatcher_Join_Power
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596604
    // IDA_NAME: _ZN21Dispatcher_Join_PowerC2Ev
    // DEMANGLED: Dispatcher_Join_Power::Dispatcher_Join_Power(void)
    Dispatcher_Join_Power();

    // IDA_EA: 0x8241EC6
    // IDA_NAME: _ZN21Dispatcher_Join_PowerD2Ev
    // DEMANGLED: Dispatcher_Join_Power::~Dispatcher_Join_Power()
    // IDA_EA: 0x8241EF6
    // IDA_NAME: _ZN21Dispatcher_Join_PowerD0Ev
    // DEMANGLED: Dispatcher_Join_Power::~Dispatcher_Join_Power()
    virtual ~Dispatcher_Join_Power();

    // IDA_EA: 0x820BA38
    // IDA_NAME: _ZN21Dispatcher_Join_Power12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_Join_Power::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233AF6
    // IDA_NAME: _ZN21Dispatcher_Join_Power4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Join_Power::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233B00
    // IDA_NAME: _ZN21Dispatcher_Join_Power7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Join_Power::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD 181 / SECEDE_POWER.  It has the same state-0 base-only object layout
// as CMD 180, but preserves its own native symbols and LineFunc site.
class Dispatcher_Secede_Power
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596620
    // IDA_NAME: _ZN23Dispatcher_Secede_PowerC2Ev
    // DEMANGLED: Dispatcher_Secede_Power::Dispatcher_Secede_Power(void)
    Dispatcher_Secede_Power();

    // IDA_EA: 0x8241E78
    // IDA_NAME: _ZN23Dispatcher_Secede_PowerD2Ev
    // DEMANGLED: Dispatcher_Secede_Power::~Dispatcher_Secede_Power()
    // IDA_EA: 0x8241EA8
    // IDA_NAME: _ZN23Dispatcher_Secede_PowerD0Ev
    // DEMANGLED: Dispatcher_Secede_Power::~Dispatcher_Secede_Power()
    virtual ~Dispatcher_Secede_Power();

    // IDA_EA: 0x820BA64
    // IDA_NAME: _ZN23Dispatcher_Secede_Power12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_Secede_Power::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233B0A
    // IDA_NAME: _ZN23Dispatcher_Secede_Power4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Secede_Power::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233B14
    // IDA_NAME: _ZN23Dispatcher_Secede_Power7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Secede_Power::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Join_Power) == 0x04u,
              "CMD180 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_Secede_Power) == 0x04u,
              "CMD181 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_PROTOCOL_POWER_MEMBERSHIP_DISPATCHERS_H_
