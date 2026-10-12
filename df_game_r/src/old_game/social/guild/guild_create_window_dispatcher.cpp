#include "old_game/social/guild/guild_create_window_dispatcher.h"

#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x85963F0
// IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindowC2Ev
// DEMANGLED: Dispatcher_OpenGuildCreateWindow::Dispatcher_OpenGuildCreateWindow(void)
Dispatcher_OpenGuildCreateWindow::Dispatcher_OpenGuildCreateWindow() {}

// IDA_EA: 0x82423A6
// IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindowD2Ev
// DEMANGLED: Dispatcher_OpenGuildCreateWindow::~Dispatcher_OpenGuildCreateWindow()
// IDA_EA: 0x82423D6
// IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindowD0Ev
// DEMANGLED: Dispatcher_OpenGuildCreateWindow::~Dispatcher_OpenGuildCreateWindow()
Dispatcher_OpenGuildCreateWindow::~Dispatcher_OpenGuildCreateWindow() {}

// IDA_EA: 0x8208A26
// IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindow12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_OpenGuildCreateWindow::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_OpenGuildCreateWindow::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            41955,
            "virtual int Dispatcher_OpenGuildCreateWindow::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    user->resetGuildCreateFlag();
    user->resetTempGuildNameAddr();
    return 0;
}

// IDA_EA: 0x8233A06
// IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindow4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_OpenGuildCreateWindow::read(PacketBuf &,MSG_BASE &)
int Dispatcher_OpenGuildCreateWindow::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A10
// IDA_NAME: _ZN32Dispatcher_OpenGuildCreateWindow7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_OpenGuildCreateWindow::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_OpenGuildCreateWindow::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
