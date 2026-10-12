#ifndef OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/message_base.h"


class CUser;
class PacketBuf;

// The old packet registry invokes these concrete classes through the canonical
// MSG_BASE/ParamBase/state-0 dispatcher specialization.  IDA constructors at
// 0x8596E70/0x8596E8C/0x8596EA8 call that base before installing each concrete
// vtable.
class Dispatcher_RequestItemLock
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596E70
    // IDA_NAME: _ZN26Dispatcher_RequestItemLockC2Ev
    // DEMANGLED: Dispatcher_RequestItemLock::Dispatcher_RequestItemLock(void)
    Dispatcher_RequestItemLock();

    // IDA_EA: 0x8222E00
    // IDA_NAME: _ZN26Dispatcher_RequestItemLock12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RequestItemLock::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8233FF6
    // IDA_NAME: _ZN26Dispatcher_RequestItemLock4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RequestItemLock::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x8234000
    // IDA_NAME: _ZN26Dispatcher_RequestItemLock7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RequestItemLock::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x8240D1A (D2), 0x8240D4A (D0)
    // IDA_NAME: _ZN26Dispatcher_RequestItemLockD2Ev / _ZN26Dispatcher_RequestItemLockD0Ev
    // DEMANGLED: Dispatcher_RequestItemLock::~Dispatcher_RequestItemLock()
    virtual ~Dispatcher_RequestItemLock();
};

class Dispatcher_RequestItemUnlock
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596E8C
    // IDA_NAME: _ZN28Dispatcher_RequestItemUnlockC2Ev
    // DEMANGLED: Dispatcher_RequestItemUnlock::Dispatcher_RequestItemUnlock(void)
    Dispatcher_RequestItemUnlock();

    // IDA_EA: 0x8222F82
    // IDA_NAME: _ZN28Dispatcher_RequestItemUnlock12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RequestItemUnlock::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x823400A
    // IDA_NAME: _ZN28Dispatcher_RequestItemUnlock4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RequestItemUnlock::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x8234014
    // IDA_NAME: _ZN28Dispatcher_RequestItemUnlock7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RequestItemUnlock::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x8240CCC (D2), 0x8240CFC (D0)
    // IDA_NAME: _ZN28Dispatcher_RequestItemUnlockD2Ev / _ZN28Dispatcher_RequestItemUnlockD0Ev
    // DEMANGLED: Dispatcher_RequestItemUnlock::~Dispatcher_RequestItemUnlock()
    virtual ~Dispatcher_RequestItemUnlock();
};

class Dispatcher_RequestItemUnlockCancel
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596EA8
    // IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancelC2Ev
    // DEMANGLED: Dispatcher_RequestItemUnlockCancel::Dispatcher_RequestItemUnlockCancel(void)
    Dispatcher_RequestItemUnlockCancel();

    // IDA_EA: 0x82231AE
    // IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancel12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RequestItemUnlockCancel::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x823401E
    // IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancel4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RequestItemUnlockCancel::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x8234028
    // IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancel7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RequestItemUnlockCancel::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x8240C7E (D2), 0x8240CAE (D0)
    // IDA_NAME: _ZN34Dispatcher_RequestItemUnlockCancelD2Ev / _ZN34Dispatcher_RequestItemUnlockCancelD0Ev
    // DEMANGLED: Dispatcher_RequestItemUnlockCancel::~Dispatcher_RequestItemUnlockCancel()
    virtual ~Dispatcher_RequestItemUnlockCancel();
};


#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestItemLock) == 0x04u,
              "Dispatcher_RequestItemLock IDA ABI size must stay 0x04");
static_assert(sizeof(Dispatcher_RequestItemUnlock) == 0x04u,
              "Dispatcher_RequestItemUnlock IDA ABI size must stay 0x04");
static_assert(sizeof(Dispatcher_RequestItemUnlockCancel) == 0x04u,
              "Dispatcher_RequestItemUnlockCancel IDA ABI size must stay 0x04");
#endif


#endif  // OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_DISPATCHERS_H_
