#include "old_game/player/core/packet_dispatcher_response_creature.h"

#include <stdint.h>

#include "infra/network/packet/packet_buf.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_ResponseCreature::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// 140828 client evidence:
//   sender sub_ADBDF0 @0xADBDF0 begins CMD101, obtains the local scene
//   object's u16 unique_id through its +0x48 virtual, writes it at 0xADBE1A,
//   and finalizes at 0xADBE20.
//
//   NOTI104 handler sub_5A7690 @0x5A7690 reads exactly one u16, resolves that
//   scene object through sub_B56180, resolves its equipped creature, and calls
//   creature virtual +0x248 with action 4.  There is no registered same-CMD
//   response handler in the 140828 client.
//
// The request value is therefore the requester scene unique_id, not a creature
// slot, item uid or PVF selector.  It is authorization data: accepting another
// user's id would let one connection trigger another character's pet action.
int Dispatcher_ResponseCreature::dispatch_sig(CUser* user, PacketBuf& packet) {
    // Preserve the recovered 130627 lifecycle boundary before consuming the
    // request.  A state-0 dispatcher may still be reached outside the town
    // character state, where this command is not meaningful.
    if (!user || user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(39631, kDispatchSignature, 0, 0);
    }

    uint16_t requester_unique_id = 0;
    if (packet.get_short(requester_unique_id) != 1) {
        return LineFunc(39632, kDispatchSignature, 0, 0);
    }
    // The native field is an actor identity, so it must match the session's
    // current scene object before any creature state or broadcast is touched.
    if (requester_unique_id !=
        static_cast<uint16_t>(user->get_unique_id())) {
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        return 0;
    }

    user_creature::CCreatureMgr* const manager =
        inventory->GetCreatureMgrW();
    if (!manager || !manager->IsEquippedCreature()) {
        return 0;
    }

    // ResponseCreature emits the exact NOTI104 {u16 unique_id} through the
    // existing village/area broadcast owner.  This is an animation request;
    // it changes neither creature inventory nor persisted creature fields.
    manager->ResponseCreature();
    return 0;
}

// The state-0 MSG_BASE virtuals stay the original no-op bodies.  CMD101 parses
// directly in dispatch_sig so no legacy empty message can bypass the u16 read.
int Dispatcher_ResponseCreature::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

int Dispatcher_ResponseCreature::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

Dispatcher_ResponseCreature::Dispatcher_ResponseCreature()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_ResponseCreature::~Dispatcher_ResponseCreature() {}
