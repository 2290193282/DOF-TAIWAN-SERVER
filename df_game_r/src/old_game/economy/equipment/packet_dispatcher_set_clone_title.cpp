#include "old_game/economy/equipment/packet_dispatcher_set_clone_title.h"

#include <stdint.h>

#include "old_game/economy/equipment/clone_title_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"


namespace {

const uint8_t kCloneTitleOperationFailed = 1u;

}  // namespace


Dispatcher_SetCloneTitle::Dispatcher_SetCloneTitle()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_SetCloneTitle::~Dispatcher_SetCloneTitle() {}

int Dispatcher_SetCloneTitle::dispatch_sig(CUser* user,
                                            PacketBuf& packet) {
    uint32_t item_id = 0;
    if (packet.get_int(item_id) != 1) {
        return 0;
    }

    // CLIENT_IDA_EVIDENCE: sub_128B360/sub_128BE80 send exactly one u32;
    // zero clears the clone and a nonzero value is the selected title's
    // protected Equipment.lst id.  sub_128BC70 consumes the same u32 only on
    // success and stores it through the clone-title manager vtable +0x14.
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        !clone_title::SetForCurrentCharacter(user, item_id)) {
        if (user) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SET_CLONE_TITLE,
                                     kCloneTitleOperationFailed);
        }
        return 0;
    }

    // Signal 822 sends the canonical success frame and full USERINFO only
    // after the dedicated DB_Q owner commits.  Queue failure above is an
    // immediate command error; SQL failure is an asynchronous command error.
    return 0;
}

int Dispatcher_SetCloneTitle::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SetCloneTitle::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
