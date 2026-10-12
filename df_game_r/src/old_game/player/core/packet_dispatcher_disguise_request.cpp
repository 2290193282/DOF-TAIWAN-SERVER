#include "old_game/player/core/packet_dispatcher_disguise_request.h"

#include <stdint.h>

#include "common/manager/global_tools.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_DisguiseRequest::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

Dispatcher_DisguiseRequest::Dispatcher_DisguiseRequest() {}

Dispatcher_DisguiseRequest::~Dispatcher_DisguiseRequest() {}

int Dispatcher_DisguiseRequest::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() != 3) {
        return 0;
    }
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user) ||
        static_cast<unsigned char>(user->IsCurCharacGhost()) == 1u ||
        user->GetAssaultPlace() != 0) {
        return 0;
    }

    uint8_t kind = 0;
    uint16_t disguise_id = 0;
    if (packet.get_byte(kind) != 1) {
        return LineFunc(52992, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(disguise_id) != 1) {
        return LineFunc(52994, kDispatchSignature, 0, 0);
    }

    // Preserve the original error contract once a structurally valid request
    // reaches an already-disguised character.
    if (static_cast<unsigned char>(user->isDisguiseCharac()) == 1u) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DISGUISE_REQUEST, 18);
        return 0;
    }

    user->setDisguiseCharac(true, kind, disguise_id);

    // The u32 template is deliberately absent from USERINFO: the 0828 sender
    // stores it in the current actor before sending CMD211, and the local
    // model lifecycle consumes it to add/remove PVF effects.  Remote actors
    // receive the established kind/id appearance fields only.
    PacketGuard notification;
    notification.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                            ENUM_NOTIPACKET_USERINFO);
    notification.put_byte(0);
    notification.put_short(1);
    user->make_basic_info(notification, 0);
    notification.finalize(true);
    G_GameWorld()->send_all(notification, user);
    return 0;
}

int Dispatcher_DisguiseRequest::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DisguiseRequest::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
