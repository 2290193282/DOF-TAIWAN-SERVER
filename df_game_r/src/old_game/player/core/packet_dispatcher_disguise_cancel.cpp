#include "old_game/player/core/packet_dispatcher_disguise_cancel.h"

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x85969BC
// IDA_NAME: _ZN25Dispatcher_DisguiseCancelC2Ev
// DEMANGLED: Dispatcher_DisguiseCancel::Dispatcher_DisguiseCancel(void)
Dispatcher_DisguiseCancel::Dispatcher_DisguiseCancel() {}

// IDA_EA: 0x824163E
// IDA_NAME: _ZN25Dispatcher_DisguiseCancelD2Ev
// DEMANGLED: Dispatcher_DisguiseCancel::~Dispatcher_DisguiseCancel()
// IDA_EA: 0x824166E
// IDA_NAME: _ZN25Dispatcher_DisguiseCancelD0Ev
// DEMANGLED: Dispatcher_DisguiseCancel::~Dispatcher_DisguiseCancel()
Dispatcher_DisguiseCancel::~Dispatcher_DisguiseCancel() {}

// IDA_EA: 0x821B496
// IDA_NAME: _ZN25Dispatcher_DisguiseCancel12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DisguiseCancel::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DisguiseCancel::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;

    if (!user || user->get_state() != 3) {
        return 0;
    }

    if (static_cast<unsigned char>(user->isDisguiseCharac()) != 1u) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DISGUISE_CANCEL, 3);
        user->setDisguiseCharac(false, 0, 0);
    }

    user->setDisguiseCharac(false, 0, 0);

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_USERINFO);
    response.put_byte(0);
    response.put_short(1);
    user->make_basic_info(response, 0);
    response.finalize(true);
    G_GameWorld()->send_all(response, user);
    return 0;
}

// IDA_EA: 0x8233D9E
// IDA_NAME: _ZN25Dispatcher_DisguiseCancel4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DisguiseCancel::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DisguiseCancel::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233DA8
// IDA_NAME: _ZN25Dispatcher_DisguiseCancel7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DisguiseCancel::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DisguiseCancel::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
