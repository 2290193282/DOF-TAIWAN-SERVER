#include "old_game/social/guild/guild_double_check_inter_receivers.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x84EDAB8
// IDA_NAME: _ZN28Inter_DBCheckDoubleGuildNameC2Ev
// DEMANGLED: Inter_DBCheckDoubleGuildName::Inter_DBCheckDoubleGuildName(void)
Inter_DBCheckDoubleGuildName::Inter_DBCheckDoubleGuildName() {
}

// IDA_EA: 0x84F9174
// IDA_NAME: _ZN28Inter_DBCheckDoubleGuildNameD2Ev
// DEMANGLED: Inter_DBCheckDoubleGuildName::~Inter_DBCheckDoubleGuildName()
// IDA_EA: 0x84F91A4
// IDA_NAME: _ZN28Inter_DBCheckDoubleGuildNameD0Ev
// DEMANGLED: Inter_DBCheckDoubleGuildName::~Inter_DBCheckDoubleGuildName()
Inter_DBCheckDoubleGuildName::~Inter_DBCheckDoubleGuildName() {
}

// IDA_EA: 0x84D347E
// IDA_NAME: _ZN28Inter_DBCheckDoubleGuildName12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_DBCheckDoubleGuildName::dispatch_sig(CUser *,char *,int)
int Inter_DBCheckDoubleGuildName::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)payload_size;
    if (user->get_state() <= 2) {
        return 0;
    }

    if (*payload == 2) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHECK_GUILD_NAME_DOUBLE, 107);
        return 0;
    }

    user->setGuildCreateFlag(2);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_CHECK_GUILD_NAME_DOUBLE);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    user->setGuildCreateFlag(4);
    return 0;
}

// IDA_EA: 0x84EDAD4
// IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddressC2Ev
// DEMANGLED: Inter_DBCheckDoubleGuildAddress::Inter_DBCheckDoubleGuildAddress(void)
Inter_DBCheckDoubleGuildAddress::Inter_DBCheckDoubleGuildAddress() {
}

// IDA_EA: 0x84F9126
// IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddressD2Ev
// DEMANGLED: Inter_DBCheckDoubleGuildAddress::~Inter_DBCheckDoubleGuildAddress()
// IDA_EA: 0x84F9156
// IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddressD0Ev
// DEMANGLED: Inter_DBCheckDoubleGuildAddress::~Inter_DBCheckDoubleGuildAddress()
Inter_DBCheckDoubleGuildAddress::~Inter_DBCheckDoubleGuildAddress() {
}

// IDA_EA: 0x84D358E
// IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddress12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_DBCheckDoubleGuildAddress::dispatch_sig(CUser *,char *,int)
int Inter_DBCheckDoubleGuildAddress::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    (void)payload_size;
    if (user->get_state() <= 2) {
        return 0;
    }

    if (*payload == 2) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHECK_GUILD_ADDREASS_DOUBLE, 111);
        return 0;
    }

    user->setGuildCreateFlag(4);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_CHECK_GUILD_ADDREASS_DOUBLE);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}
