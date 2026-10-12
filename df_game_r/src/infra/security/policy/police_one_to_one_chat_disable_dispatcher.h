#ifndef OLDSRV_SECURITY_POLICE_ONE_TO_ONE_CHAT_DISABLE_DISPATCHER_H_
#define OLDSRV_SECURITY_POLICE_ONE_TO_ONE_CHAT_DISABLE_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 330 / police one-to-one chat disable. The concrete old provider remains
// unregistered until PacketDispatcher's complete original table transaction is
// restored.
class Dispatcer_polic_one_to_one_chat_disable
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859739A
    // IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disableC2Ev
    // DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::Dispatcer_polic_one_to_one_chat_disable(void)
    Dispatcer_polic_one_to_one_chat_disable();

    // IDA_EA: 0x82402BE
    // IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disableD2Ev
    // DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::~Dispatcer_polic_one_to_one_chat_disable()
    // IDA_EA: 0x82402EE
    // IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disableD0Ev
    // DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::~Dispatcer_polic_one_to_one_chat_disable()
    virtual ~Dispatcer_polic_one_to_one_chat_disable();

    // IDA_EA: 0x82272CE
    // IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disable12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823429E
    // IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disable4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82342A8
    // IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disable7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcer_polic_one_to_one_chat_disable) == 0x04u,
              "CMD330 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_SECURITY_POLICE_ONE_TO_ONE_CHAT_DISABLE_DISPATCHER_H_
