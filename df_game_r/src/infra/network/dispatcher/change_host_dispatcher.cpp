#include "infra/network/dispatcher/change_host_dispatcher.h"

#include "common/manager/global_tools.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x85960E0
// IDA_NAME: _ZN21Dispatcher_ChangeHostC2Ev
// DEMANGLED: Dispatcher_ChangeHost::Dispatcher_ChangeHost(void)
Dispatcher_ChangeHost::Dispatcher_ChangeHost() {}

// IDA_EA: 0x8242AF6
// IDA_NAME: _ZN21Dispatcher_ChangeHostD2Ev
// DEMANGLED: Dispatcher_ChangeHost::~Dispatcher_ChangeHost()
// IDA_EA: 0x8242B26
// IDA_NAME: _ZN21Dispatcher_ChangeHostD0Ev
// DEMANGLED: Dispatcher_ChangeHost::~Dispatcher_ChangeHost()
Dispatcher_ChangeHost::~Dispatcher_ChangeHost() {}

// IDA_EA: 0x8204C1E
// IDA_NAME: _ZN21Dispatcher_ChangeHost12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ChangeHost::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ChangeHost::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return LineFunc(
            40127,
            "virtual int Dispatcher_ChangeHost::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    char requested_seat = 0;
    if (packet.get_byte(requested_seat) != 1) {
        return LineFunc(
            40130,
            "virtual int Dispatcher_ChangeHost::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CParty* const party = user->GetParty();
    if (!party) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_HOST, 19);
        return 0;
    }

    CPartyTelePort* const party_teleport = party->GetPartyTelePort();
    if ((static_cast<unsigned int>(party_teleport->get_teleport_state()) &
         0x80u) == 0u &&
        static_cast<char>(party->GetPartyTelePort()->get_teleport_state()) <=
            3) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_HOST, 158);
        return 0;
    }

    const int result = party->change_manager(user,
                                             static_cast<int>(requested_seat));
    if (result > 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_HOST,
                                 static_cast<unsigned char>(result));
    }
    return 0;
}

// IDA_EA: 0x8233826
// IDA_NAME: _ZN21Dispatcher_ChangeHost4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeHost::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeHost::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233830
// IDA_NAME: _ZN21Dispatcher_ChangeHost7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeHost::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeHost::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
