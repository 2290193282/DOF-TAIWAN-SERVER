#ifndef OLDSRV_OLD_GAME_GROUP_GUILD_DOUBLE_CHECK_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_GROUP_GUILD_DOUBLE_CHECK_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


// IDA unit: 13_database_dispatch/Dispatcher_CheckGuildNameDouble.
class Dispatcher_CheckGuildNameDouble
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85963B8
    // IDA_NAME: _ZN31Dispatcher_CheckGuildNameDoubleC2Ev
    // DEMANGLED: Dispatcher_CheckGuildNameDouble::Dispatcher_CheckGuildNameDouble(void)
    Dispatcher_CheckGuildNameDouble();

    // IDA_EA: 0x8242442
    // IDA_NAME: _ZN31Dispatcher_CheckGuildNameDoubleD2Ev
    // DEMANGLED: Dispatcher_CheckGuildNameDouble::~Dispatcher_CheckGuildNameDouble()
    // IDA_EA: 0x8242472
    // IDA_NAME: _ZN31Dispatcher_CheckGuildNameDoubleD0Ev
    // DEMANGLED: Dispatcher_CheckGuildNameDouble::~Dispatcher_CheckGuildNameDouble()
    virtual ~Dispatcher_CheckGuildNameDouble();

    // IDA_EA: 0x82084E8
    // IDA_NAME: _ZN31Dispatcher_CheckGuildNameDouble12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CheckGuildNameDouble::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82339DE
    // IDA_NAME: _ZN31Dispatcher_CheckGuildNameDouble4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CheckGuildNameDouble::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82339E8
    // IDA_NAME: _ZN31Dispatcher_CheckGuildNameDouble7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CheckGuildNameDouble::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_CheckGuildAddressDouble.
class Dispatcher_CheckGuildAddressDouble
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85963D4
    // IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDoubleC2Ev
    // DEMANGLED: Dispatcher_CheckGuildAddressDouble::Dispatcher_CheckGuildAddressDouble(void)
    Dispatcher_CheckGuildAddressDouble();

    // IDA_EA: 0x82423F4
    // IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDoubleD2Ev
    // DEMANGLED: Dispatcher_CheckGuildAddressDouble::~Dispatcher_CheckGuildAddressDouble()
    // IDA_EA: 0x8242424
    // IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDoubleD0Ev
    // DEMANGLED: Dispatcher_CheckGuildAddressDouble::~Dispatcher_CheckGuildAddressDouble()
    virtual ~Dispatcher_CheckGuildAddressDouble();

    // IDA_EA: 0x82087A4
    // IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDouble12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CheckGuildAddressDouble::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82339F2
    // IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDouble4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CheckGuildAddressDouble::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82339FC
    // IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDouble7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CheckGuildAddressDouble::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CheckGuildNameDouble) == 0x04u,
              "CMD159 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_CheckGuildAddressDouble) == 0x04u,
              "CMD160 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_GUILD_DOUBLE_CHECK_DISPATCHERS_H_
