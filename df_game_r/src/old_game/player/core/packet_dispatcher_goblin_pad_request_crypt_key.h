#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_GOBLIN_PAD_REQUEST_CRYPT_KEY_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_GOBLIN_PAD_REQUEST_CRYPT_KEY_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 08_quest_skill_growth/Dispatcher_GoblinPadRequestCryptKey.
// The class carries no state beyond the state-0 IPacketDispatcher base.
class Dispatcher_GoblinPadRequestCryptKey
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596AD4
    // IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKeyC2Ev
    // DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::Dispatcher_GoblinPadRequestCryptKey(void)
    Dispatcher_GoblinPadRequestCryptKey();

    // IDA_EA: 0x8241296
    // IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKeyD2Ev
    // DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::~Dispatcher_GoblinPadRequestCryptKey()
    // IDA_EA: 0x82412C6
    // IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKeyD0Ev
    // DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::~Dispatcher_GoblinPadRequestCryptKey()
    virtual ~Dispatcher_GoblinPadRequestCryptKey();

    // IDA_EA: 0x821E58C
    // IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKey12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233E7A
    // IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKey4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233E84
    // IDA_NAME: _ZN35Dispatcher_GoblinPadRequestCryptKey7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GoblinPadRequestCryptKey::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GoblinPadRequestCryptKey) == 0x04u,
              "CMD226 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_GOBLIN_PAD_REQUEST_CRYPT_KEY_H_
