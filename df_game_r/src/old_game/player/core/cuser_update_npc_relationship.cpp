#include "old_game/player/core/cuser.h"
#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"


// IDA_EA: 0x8652DD8
// IDA_NAME: _ZN5CUser22_updateNPCRelationshipEv
// DEMANGLED: CUser::_updateNPCRelationship(void)
void CUser::_updateNPCRelationship() {
    (void)npc_gift::Flush(this);
}

// IDA_EA: 0x8681218
// IDA_NAME: _ZN5CUser29resetNPCRelationShipDailyDataEv
// DEMANGLED: CUser::resetNPCRelationShipDailyData(void)
void CUser::resetNPCRelationShipDailyData() {
    npc_gift::ResetDaily(this);
}

// IDA_EA: 0x868121E
// IDA_NAME: _ZN5CUser24sendNPCRelationShipFavorEv
// DEMANGLED: CUser::sendNPCRelationShipFavor(void)
// The original body contains no statements.  No notification or DB side
// effect may be invented until a non-empty original body is available.
void CUser::sendNPCRelationShipFavor() {
}
