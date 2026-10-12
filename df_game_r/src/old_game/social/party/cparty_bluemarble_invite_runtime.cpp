#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x859D4B8
// IDA_NAME: _ZN6CParty22send_invite_bluemarbleEi
// DEMANGLED: CParty::send_invite_bluemarble(int)
void CParty::send_invite_bluemarble(int unique_id) {
    PacketGuard packet;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 0u) {
            packet.clear();
            packet.put_header(0, 7);
            packet.put_short(static_cast<short>(unique_id));
            packet.put_byte(9);
            packet.put_int(static_cast<unsigned short>(
                members_[seat].user->get_unique_id()));
            packet.finalize(1);
            members_[seat].user->Send(packet);
        }
    }
}
