#include "old_game/player/core/packet_dispatcher_link_character.h"

#include "common/manager/global_tools.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "old_game/player/core/cuser.h"


namespace {

const uint8_t kLinkOperationFailed = 1u;

bool IsCharacterListAvailable(CUser* user) {
    return user->get_state() == 2 &&
           !static_cast<unsigned char>(user->isLocked4DataLoad()) &&
           !static_cast<unsigned char>(user->isLockedCreateCharac()) &&
           !static_cast<unsigned char>(user->isLockedDeleteCharac());
}

}  // namespace

Dispatcher_ConnectLinkCharacter::Dispatcher_ConnectLinkCharacter()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_ConnectLinkCharacter::~Dispatcher_ConnectLinkCharacter() {}

int Dispatcher_ConnectLinkCharacter::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    uint8_t first_slot = 0;
    uint8_t second_slot = 0;
    uint8_t first_grow_type = 0;
    uint8_t second_grow_type = 0;
    if (packet.get_byte(first_slot) != 1 ||
        packet.get_byte(second_slot) != 1 ||
        packet.get_byte(first_grow_type) != 1 ||
        packet.get_byte(second_grow_type) != 1) {
        return LineFunc(
            __LINE__,
            "Dispatcher_ConnectLinkCharacter::dispatch_sig(CUser*,PacketBuf&)",
            0,
            0);
    }

    // CLIENT_IDA_EVIDENCE: sender sub_1388340 writes selected slot A/B then
    // the two selected grow-type vector entries.  CMD309 handler sub_55BAE0
    // consumes only the common result byte; success has no command tail.
    if (!IsCharacterListAvailable(user) ||
        !user->ConnectLinkCharacters(first_slot,
                                     second_slot,
                                     first_grow_type,
                                     second_grow_type)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CONNECT_LINK_CHARAC,
                                 kLinkOperationFailed);
        return 0;
    }

    // The success packet is emitted only by signal-824's Inter owner after
    // the dedicated DB worker reply passes session and slot-identity checks.
    return 0;
}

int Dispatcher_ConnectLinkCharacter::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_ConnectLinkCharacter::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_DisconnectLinkCharacter::Dispatcher_DisconnectLinkCharacter()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_DisconnectLinkCharacter::~Dispatcher_DisconnectLinkCharacter() {}

int Dispatcher_DisconnectLinkCharacter::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    uint8_t first_slot = 0;
    uint8_t second_slot = 0;
    if (packet.get_byte(first_slot) != 1 ||
        packet.get_byte(second_slot) != 1) {
        return LineFunc(
            __LINE__,
            "Dispatcher_DisconnectLinkCharacter::dispatch_sig(CUser*,PacketBuf&)",
            0,
            0);
    }

    // CLIENT_IDA_EVIDENCE: sender sub_1388E30 writes exactly the two linked
    // character slots.  Handler sub_55BB40 reads no success tail.
    if (!IsCharacterListAvailable(user) ||
        !user->DisconnectLinkCharacters(first_slot, second_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DISCONNECT_LINK_CHARAC,
                                 kLinkOperationFailed);
        return 0;
    }

    // See CMD309 above: a queued request is not yet a successful operation.
    return 0;
}

int Dispatcher_DisconnectLinkCharacter::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DisconnectLinkCharacter::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
