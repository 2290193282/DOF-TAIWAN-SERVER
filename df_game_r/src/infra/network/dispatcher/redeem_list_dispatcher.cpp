#include "infra/network/dispatcher/redeem_list_dispatcher.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x859729E
// IDA_NAME: _ZN21Dispatcher_RedeemListC2Ev
// DEMANGLED: Dispatcher_RedeemList::Dispatcher_RedeemList(void)
Dispatcher_RedeemList::Dispatcher_RedeemList() {}

// IDA_EA: 0x824052E
// IDA_NAME: _ZN21Dispatcher_RedeemListD2Ev
// DEMANGLED: Dispatcher_RedeemList::~Dispatcher_RedeemList()
// IDA_EA: 0x824055E
// IDA_NAME: _ZN21Dispatcher_RedeemListD0Ev
// DEMANGLED: Dispatcher_RedeemList::~Dispatcher_RedeemList()
Dispatcher_RedeemList::~Dispatcher_RedeemList() {}

// IDA_EA: 0x822621C
// IDA_NAME: _ZN21Dispatcher_RedeemList12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RedeemList::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RedeemList::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() > 2) {
        user->send_RedeemInfo();
    }
    return 0;
}

// IDA_EA: 0x82341FE
// IDA_NAME: _ZN21Dispatcher_RedeemList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RedeemList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RedeemList::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234208
// IDA_NAME: _ZN21Dispatcher_RedeemList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RedeemList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RedeemList::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
