#include "old_game/appearance/pet/creature.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

namespace user_creature {

bool CCreatureMgr::GetActiveCreatureSkillContext(int& creature_uid,
                                                  int& item_id,
                                                  int& level,
                                                  int& state) const {
    creature_uid = 0;
    item_id = 0;
    level = 0;
    state = 0;

    CCreature* const creature = active_creature_;
    if (!owner_ || !creature) {
        return false;
    }

    creature_uid = creature->GetUid();
    item_id = creature->GetItemId();
    level = creature->GetLevel();
    state = creature->GetState();
    return creature_uid > 0 && item_id > 0 && level > 0;
}

int CCreature::IncrementStomachValue(int value) {
    if (value <= 0) {
        return GetStomach();
    }
    return static_cast<int>(stomach_.IncrementValue(
        static_cast<float>(value)));
}

int CCreatureMgr::ApplyFillPoint(int expected_uid,
                                 int fill_point,
                                 int& absolute_stomach) {
    absolute_stomach = 0;

    CUser* const owner = owner_;
    CCreature* const creature = active_creature_;
    if (!owner || !creature || expected_uid <= 0 || fill_point <= 0 ||
        fill_point > 100 || owner->get_state() != 5 ||
        owner->is_fighting() == 0 || creature->GetUid() != expected_uid ||
        creature->GetState() != CREATURE_STATE_ABI_1 ||
        creature->IsDieCreature()) {
        return 0;
    }

    const int current_stomach = creature->GetStomach();
    if (current_stomach < 1 || current_stomach > 100) {
        return 0;
    }
    const int predicted_stomach =
        current_stomach > 100 - fill_point
            ? 100
            : current_stomach + fill_point;

    // Build the DB update before mutating runtime state.  UseFeedCreature can
    // fail only when the stream pool did not provide a packet; in that case
    // CMD549 must not change stomach or start its server cooldown.
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner);
    if (sender.UseFeedCreature(expected_uid, predicted_stomach) != 1) {
        cMyTrace trace(
            "int user_creature::CCreatureMgr::ApplyFillPoint(int,int,int&)",
            0,
            5);
        trace("CMD549 cannot prepare creature stomach DB update uid(%d)\n",
              owner->get_unique_id());
        return 0;
    }

    absolute_stomach = creature->IncrementStomachValue(fill_point);
    if (absolute_stomach != predicted_stomach) {
        // GetStomach() and IncrementValue() normally produce the same integer
        // projection.  Refresh the already-owned packet if an unusual
        // fractional/timer boundary made the projection differ.
        if (sender.UseFeedCreature(expected_uid, absolute_stomach) != 1) {
            return 0;
        }
    }
    if (sender.Send() != 1) {
        return 0;
    }

    CPacketRespondent respondent(owner, 0, 0);
    respondent.MakeNotipacketCreatureState(expected_uid, absolute_stomach);
    respondent.Send();
    return 1;
}

}  // namespace user_creature
