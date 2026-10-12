#ifndef OLDSRV_OLD_GAME_GROUP_GUILD_CREATE_WINDOW_DISPATCHER_H_
#define OLDSRV_OLD_GAME_GROUP_GUILD_CREATE_WINDOW_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_OpenGuildCreateWindow.
// The class has no state after the state-0 packet-dispatch base.
class Dispatcher_OpenGuildCreateWindow
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85963F0
    // IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindowC2Ev
    // DEMANGLED: Dispatcher_OpenGuildCreateWindow::Dispatcher_OpenGuildCreateWindow(void)
    Dispatcher_OpenGuildCreateWindow();

    // IDA_EA: 0x82423A6
    // IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindowD2Ev
    // DEMANGLED: Dispatcher_OpenGuildCreateWindow::~Dispatcher_OpenGuildCreateWindow()
    // IDA_EA: 0x82423D6
    // IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindowD0Ev
    // DEMANGLED: Dispatcher_OpenGuildCreateWindow::~Dispatcher_OpenGuildCreateWindow()
    virtual ~Dispatcher_OpenGuildCreateWindow();

    // IDA_EA: 0x8208A26
    // IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindow12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_OpenGuildCreateWindow::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233A06
    // IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindow4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_OpenGuildCreateWindow::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233A10
    // IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindow7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_OpenGuildCreateWindow::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_OpenGuildCreateWindow) == 0x04u,
              "CMD161 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_GUILD_CREATE_WINDOW_DISPATCHER_H_
