#include "old_game/player/core/packet_dispatcher_redeem.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x85972BA
// IDA_NAME: _ZN17Dispatcher_RedeemC2Ev
// DEMANGLED: Dispatcher_Redeem::Dispatcher_Redeem(void)
Dispatcher_Redeem::Dispatcher_Redeem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82404E0
// IDA_NAME: _ZN17Dispatcher_RedeemD2Ev
// DEMANGLED: Dispatcher_Redeem::~Dispatcher_Redeem()
// IDA_EA: 0x8240510
// IDA_NAME: _ZN17Dispatcher_RedeemD0Ev
// DEMANGLED: Dispatcher_Redeem::~Dispatcher_Redeem()
Dispatcher_Redeem::~Dispatcher_Redeem() {}

// IDA_EA: 0x8226250
// IDA_NAME: _ZN17Dispatcher_Redeem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Redeem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Redeem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() > 2) {
        char index;
        packet.get_byte(index);
        user->Redeem_Item(index);
    }
    return 0;
}

// IDA_EA: 0x8234212
// IDA_NAME: _ZN17Dispatcher_Redeem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Redeem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Redeem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823421C
// IDA_NAME: _ZN17Dispatcher_Redeem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Redeem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Redeem::process(CUser* user,
                               MSG_BASE& message,
                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
