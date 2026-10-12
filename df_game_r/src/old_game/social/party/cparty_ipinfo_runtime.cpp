#include "old_game/social/party/party.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x859CEA2
// IDA_NAME: _ZN6CParty17send_party_ipinfoEv
// DEMANGLED: CParty::send_party_ipinfo(void)
void CParty::send_party_ipinfo() {
    PacketGuard packet;
    packet.put_header(0, 11);
    packet.put_byte(get_member_count());

    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat)) {
            CUser* const user = members_[seat].user;
            CNetwork4096_450000& network = user->network_;

            packet.put_short(static_cast<short>(user->get_unique_id()));
            packet.put_int(static_cast<int>(network.get_inner_ip()));
            packet.put_int(static_cast<int>(network.get_ip()));
            packet.put_short(static_cast<short>(network.get_port()));
            packet.put_int(user->get_acc_id());
            packet.put_byte(network.get_nat_type());
            packet.put_int(static_cast<int>(network.get_mtu()));

            if (user->getCurCharacR()) {
                packet.put_byte(user->getCurCharacStamina());
            } else {
                packet.put_byte(0);
            }
        }
    }

    packet.finalize(true);
    send_to_party(packet);
}
