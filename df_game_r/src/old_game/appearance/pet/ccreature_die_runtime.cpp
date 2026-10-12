#include "old_game/appearance/pet/creature.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace user_creature {

// IDA_EA: 0x83406FE
// IDA_NAME: _ZN13user_creature17CPacketRespondent26MakeNotipacketDiedCreatureEv
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketDiedCreature(void)
int CPacketRespondent::MakeNotipacketDiedCreature() {
    clear();
    put_header(0, 100);
    put_short(user_->get_unique_id());
    return finalize(true);
}

// IDA_EA: 0x8338536
// IDA_NAME: _ZN13user_creature9CCreature3DieEP5CUser
// DEMANGLED: user_creature::CCreature::Die(CUser *)
int CCreature::Die(CUser* user) {
    CPacketRespondent packet(user, 0, 0);
    packet.MakeNotipacketDiedCreature();
    packet.SendAll();
    return 1;
}

}  // namespace user_creature
