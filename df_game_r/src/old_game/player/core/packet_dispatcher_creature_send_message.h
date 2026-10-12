#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_CREATURE_SEND_MESSAGE_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_CREATURE_SEND_MESSAGE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_CreatureSendMessage.
// This is a state-0 MSG_BASE/ParamBase specialization whose base generic is
// canonically owned by src/protocol/packet_dispatcher_msg_base.cpp.
class Dispatcher_CreatureSendMessage
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859601C
    // IDA_NAME: _ZN30Dispatcher_CreatureSendMessageC2Ev
    // DEMANGLED: Dispatcher_CreatureSendMessage::Dispatcher_CreatureSendMessage(void)
    Dispatcher_CreatureSendMessage();

    // IDA_EA: 0x8242B92 / 0x8242BC2
    // IDA_NAME: _ZN30Dispatcher_CreatureSendMessageD2Ev / D0Ev
    // DEMANGLED: Dispatcher_CreatureSendMessage::~Dispatcher_CreatureSendMessage()
    virtual ~Dispatcher_CreatureSendMessage();

    // IDA_EA: 0x8204710
    // IDA_NAME: _ZN30Dispatcher_CreatureSendMessage12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CreatureSendMessage::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82337FE
    // IDA_NAME: _ZN30Dispatcher_CreatureSendMessage4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CreatureSendMessage::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233808
    // IDA_NAME: _ZN30Dispatcher_CreatureSendMessage7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CreatureSendMessage::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CreatureSendMessage) == 0x04u,
              "CMD117 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_CREATURE_SEND_MESSAGE_H_
