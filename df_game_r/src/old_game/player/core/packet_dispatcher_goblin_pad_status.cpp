#include "old_game/player/core/packet_dispatcher_goblin_pad_status.h"

#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8596850
// IDA_NAME: _ZN26Dispatcher_GoblinPadStatusC2Ev
// DEMANGLED: Dispatcher_GoblinPadStatus::Dispatcher_GoblinPadStatus(void)
Dispatcher_GoblinPadStatus::Dispatcher_GoblinPadStatus() {}

// IDA_EA: 0x8241998
// IDA_NAME: _ZN26Dispatcher_GoblinPadStatusD2Ev
// DEMANGLED: Dispatcher_GoblinPadStatus::~Dispatcher_GoblinPadStatus()
// IDA_EA: 0x82419C8
// IDA_NAME: _ZN26Dispatcher_GoblinPadStatusD0Ev
// DEMANGLED: Dispatcher_GoblinPadStatus::~Dispatcher_GoblinPadStatus()
Dispatcher_GoblinPadStatus::~Dispatcher_GoblinPadStatus() {}

// IDA_EA: 0x82177AE
// IDA_NAME: _ZN26Dispatcher_GoblinPadStatus12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_GoblinPadStatus::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GoblinPadStatus::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;

    uint8_t status = 0;
    if (user->getPad()->isActivate()) {
        if (user->getPad()->getFailCnt() <= 2) {
            status = user->getPad()->isCertified() ? 2 : 3;
        } else {
            status = 1;
        }
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_GOBLIN_PAD_STATUS);
    response.put_byte(1);
    response.put_byte(status);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233CD6
// IDA_NAME: _ZN26Dispatcher_GoblinPadStatus4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GoblinPadStatus::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GoblinPadStatus::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233CE0
// IDA_NAME: _ZN26Dispatcher_GoblinPadStatus7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GoblinPadStatus::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GoblinPadStatus::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
