#include "old_game/pvp/battlefield/battle_field.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8146994
// IDA_NAME: _ZN6Global14CBossTowerFunc7GetItemEP5CUseribc
// DEMANGLED: Global::CBossTowerFunc::GetItem(CUser *,int,bool,char)
int Global::CBossTowerFunc::GetItem(CUser* user,
                                    int unique_id,
                                    bool client_flag,
                                    char client_rarity) {
    WongWork::CBossTower* const tower = user->getBossTower();
    if (!tower) return 1;

    if (tower->handlePickupItem(user, unique_id, client_flag, client_rarity)) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
        packet.put_byte(1);
        packet.finalize(true);
        user->Send(packet);
    }
    return 1;
}
