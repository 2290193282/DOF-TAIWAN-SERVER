#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_CREATURE_SCRIPT_MESSAGE_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_CREATURE_SCRIPT_MESSAGE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_CreatureScriptMessage.
// The state-0 MSG_BASE/ParamBase base owns its canonical ABI in
// src/protocol/packet_dispatcher_msg_base.cpp; this class adds only the
// concrete virtual overrides present in the original table.
class Dispatcher_CreatureScriptMessage
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85960FC
    // IDA_NAME: _ZN32Dispatcher_CreatureScriptMessageC2Ev
    // DEMANGLED: Dispatcher_CreatureScriptMessage::Dispatcher_CreatureScriptMessage(void)
    Dispatcher_CreatureScriptMessage();

    // IDA_EA: 0x8242AA8 / 0x8242AD8
    // IDA_NAME: _ZN32Dispatcher_CreatureScriptMessageD2Ev / D0Ev
    // DEMANGLED: Dispatcher_CreatureScriptMessage::~Dispatcher_CreatureScriptMessage()
    virtual ~Dispatcher_CreatureScriptMessage();

    // IDA_EA: 0x8204D8E
    // IDA_NAME: _ZN32Dispatcher_CreatureScriptMessage12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CreatureScriptMessage::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823383A
    // IDA_NAME: _ZN32Dispatcher_CreatureScriptMessage4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CreatureScriptMessage::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233844
    // IDA_NAME: _ZN32Dispatcher_CreatureScriptMessage7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CreatureScriptMessage::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CreatureScriptMessage) == 0x04u,
              "CMD125 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_CREATURE_SCRIPT_MESSAGE_H_
