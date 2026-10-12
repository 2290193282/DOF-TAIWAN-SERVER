#include "old_game/social/party/party.h"

#include "old_game/pvp/battlefield/pvp_assault.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x859CB00
// IDA_NAME: _ZN6CParty11SendWalkoutEi17ENUM_WALKOUT_TYPE
// DEMANGLED: CParty::SendWalkout(int, ENUM_WALKOUT_TYPE)
int CParty::SendWalkout(int seat, ENUM_WALKOUT_TYPE type) {
    PacketGuard packet;
    packet.put_header(0, 10);
    packet.put_byte(seat);
    packet.put_byte(static_cast<int>(type));
    packet.finalize(true);
    send_to_party(packet);
    return 1;
}

// IDA_EA: 0x859CA4A
// IDA_NAME: _ZN6CParty14walkout_memberEP5CUseri
// DEMANGLED: CParty::walkout_member(CUser *, int)
int CParty::walkout_member(CUser* user, int seat) {
    if (manager_ != user) {
        return 8;
    }

    CUser* const selected_user = members_[seat].user;
    if (!selected_user) {
        return 4;
    }

    const int leave_result =
        leave_user(selected_user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
    if (leave_result == 0) {
        pvp_assault::GetInstanceAssaultMgr()->ForcedMoveAtPowerWarPlace(
            selected_user);
        return 0;
    }
    return leave_result == 1 ? 1 : 4;
}
