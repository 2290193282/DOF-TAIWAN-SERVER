#ifndef OLDSRV_OLD_GAME_INVENTORY_USE_JEWEL_VERIFY_GOLD_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_INVENTORY_USE_JEWEL_VERIFY_GOLD_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;


// 140828 CMD201 / USE_EMBLEM. The object remains a state-0 direct dispatcher;
// its native dynamic request is consumed by dispatch_sig().
class Dispatcher_UseJewel
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85968A4
    // IDA_NAME: _ZN19Dispatcher_UseJewelC2Ev
    // DEMANGLED: Dispatcher_UseJewel::Dispatcher_UseJewel(void)
    Dispatcher_UseJewel();

    // IDA_EA: 0x82418AE
    // IDA_NAME: _ZN19Dispatcher_UseJewelD2Ev
    // DEMANGLED: Dispatcher_UseJewel::~Dispatcher_UseJewel()
    // IDA_EA: 0x82418DE
    // IDA_NAME: _ZN19Dispatcher_UseJewelD0Ev
    // DEMANGLED: Dispatcher_UseJewel::~Dispatcher_UseJewel()
    virtual ~Dispatcher_UseJewel();

    // IDA_EA: 0x8217BD6
    // IDA_NAME: _ZN19Dispatcher_UseJewel12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_UseJewel::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233D12
    // IDA_NAME: _ZN19Dispatcher_UseJewel4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_UseJewel::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233D1C
    // IDA_NAME: _ZN19Dispatcher_UseJewel7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_UseJewel::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};


// 140828 CMD283 / VERIFY_GOLD. The object remains a state-0 direct dispatcher.
class Dispatcher_VerifyGold
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596F18
    // IDA_NAME: _ZN21Dispatcher_VerifyGoldC2Ev
    // DEMANGLED: Dispatcher_VerifyGold::Dispatcher_VerifyGold(void)
    Dispatcher_VerifyGold();

    // IDA_EA: 0x8240B94
    // IDA_NAME: _ZN21Dispatcher_VerifyGoldD2Ev
    // DEMANGLED: Dispatcher_VerifyGold::~Dispatcher_VerifyGold()
    // IDA_EA: 0x8240BC4
    // IDA_NAME: _ZN21Dispatcher_VerifyGoldD0Ev
    // DEMANGLED: Dispatcher_VerifyGold::~Dispatcher_VerifyGold()
    virtual ~Dispatcher_VerifyGold();

    // IDA_EA: 0x8223BC0
    // IDA_NAME: _ZN21Dispatcher_VerifyGold12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_VerifyGold::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823405A
    // IDA_NAME: _ZN21Dispatcher_VerifyGold4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_VerifyGold::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234064
    // IDA_NAME: _ZN21Dispatcher_VerifyGold7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_VerifyGold::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};


#if defined(__i386__)
static_assert(sizeof(Dispatcher_UseJewel) == 0x04u,
              "CMD201 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_VerifyGold) == 0x04u,
              "CMD283 dispatcher must retain its i386 base-only ABI");
#endif


#endif  // OLDSRV_OLD_GAME_INVENTORY_USE_JEWEL_VERIFY_GOLD_DISPATCHERS_H_
