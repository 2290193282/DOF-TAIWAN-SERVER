#include "old_game/appearance/pet/packet_dispatcher_creature_fillpoint.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/appearance/pet/creature_fillpoint_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace {

const unsigned char kCreatureFillPointGenericError = 1;

int RejectFillPoint(CUser* user) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATURE_FILLPOINT,
                                 kCreatureFillPointGenericError);
    }
    return 0;
}

}  // namespace

Dispatcher_CreatureFillPoint::Dispatcher_CreatureFillPoint()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(5)>() {}

Dispatcher_CreatureFillPoint::~Dispatcher_CreatureFillPoint() {}

int Dispatcher_CreatureFillPoint::check_state(CUser* user,
                                               PacketBuf& packet) {
    (void)packet;
    if (!user || user->get_state() != 5 || user->is_fighting() == 0) {
        RejectFillPoint(user);
        return 1;
    }
    return 0;
}

int Dispatcher_CreatureFillPoint::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    unsigned int requested_fill_point = 0;
    if (!user || packet.get_int(requested_fill_point) != 1 ||
        user->get_state() != 5 || user->is_fighting() == 0) {
        return RejectFillPoint(user);
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    user_creature::CCreatureMgr* const creatures =
        inventory ? inventory->GetCreatureMgrW() : 0;
    int creature_uid = 0;
    int item_id = 0;
    int creature_level = 0;
    int creature_state = 0;
    // RECONSTRUCTED_AUTHORIZATION_RULE: CMD549 carries neither skill ID nor
    // the client actor's state-5/state-6 selector.  Enforce every condition
    // observable by the server: exact dungeon state, active live creature,
    // equipped creature runtime state, learned PVF record/value and cooldown.
    if (!creatures ||
        !creatures->GetActiveCreatureSkillContext(creature_uid,
                                                   item_id,
                                                   creature_level,
                                                   creature_state) ||
        creature_state != user_creature::CREATURE_STATE_ABI_1 ||
        creatures->IsDieCreature() != 0) {
        return RejectFillPoint(user);
    }

    CreatureFillPoint::SkillConfig config;
    if (!CreatureFillPoint::ResolveGeneralSkill(item_id,
                                                 requested_fill_point,
                                                 creature_level,
                                                 config)) {
        return RejectFillPoint(user);
    }

    CSystemTime* const system_time = GlobalData::system_time();
    const int charac_no = user->getCurCharacNo();
    if (!system_time || charac_no <= 0) {
        return RejectFillPoint(user);
    }
    const uint32_t now_tick =
        static_cast<uint32_t>(system_time->getCurTickCount());

    // RECONSTRUCTED_COOLDOWN: enforce the authoritative general-skill PVF
    // recovery exactly.  The 0828 client applies additional modifier calls
    // before displaying its timer; no matching server-side modifier owner is
    // proven yet, so this conservative boundary may reject an early client
    // retry but never permits extra stomach gain.
    if (!CreatureFillPoint::BeginCooldown(user,
                                           charac_no,
                                           creature_uid,
                                           config.recovery_time_ms,
                                           now_tick)) {
        return RejectFillPoint(user);
    }

    int absolute_stomach = 0;
    if (creatures->ApplyFillPoint(creature_uid,
                                  config.fill_point,
                                  absolute_stomach) != 1) {
        CreatureFillPoint::CancelCooldown(user,
                                          charac_no,
                                          creature_uid,
                                          now_tick);
        return RejectFillPoint(user);
    }

    CreatureFillPoint::CommitCooldown(user,
                                      charac_no,
                                      creature_uid,
                                      now_tick);
    user->SendCmdOkPacket(ENUM_CMDPACKET_CREATURE_FILLPOINT);
    return 0;
}

int Dispatcher_CreatureFillPoint::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_CreatureFillPoint::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

template class IPacketDispatcher<MSG_BASE,
                                 ParamBase,
                                 static_cast<ch_state>(5)>;
