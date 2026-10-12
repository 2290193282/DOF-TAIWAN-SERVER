#include "old_game/economy/inventory/inventory.h"

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/invalid_area_move.h"

#include <algorithm>
#include <stdint.h>
#include <vector>


namespace {

// Project runtime character state into StackableAction::UsablePlace.  The two
// values are different semantic domains: official 0627 maps state 3/5 to
// village/dungeon (1/2), while 0828 keeps both ordinary village areas and the
// village's gate/Seria room in state 3.  Use the existing village/area rule to
// distinguish the 0828 Seria-room extension instead of treating every state-3
// area as [seria room].
int ResolveStackableActionUsablePlace(CUser* user) {
    if (!user) {
        return 0;
    }

    if (G_GameWorld()->IsPVPChannel()) {
        return 3;
    }

    const int user_state = user->get_state();
    if (user_state == 5) {
        return 2;
    }
    if (user_state != 3) {
        return 0;
    }

    const int village = user->getCurCharacVill();
    if (village > 0) {
        CHackLog_InvalidAreaMove* const invalid_area =
            reinterpret_cast<CHackLog_InvalidAreaMove*>(
                static_cast<uintptr_t>(village));
        if (invalid_area->checkSeriaRoom(user->get_area(false), 0)) {
            return 4;
        }
    }
    return 1;
}

}  // namespace



// IDA_EA: 0x822CA58
// IDA_NAME: _ZNK14CStackableItem21verifyStackableActionEN15StackableAction4TypeE
// DEMANGLED: CStackableItem::verifyStackableAction(StackableAction::Type)const
bool CStackableItem::verifyStackableAction(
    StackableAction::Type action) const {
    return stackable_action_type_ == static_cast<int>(action);
}

// IDA_EA: 0x850E9F4
// IDA_NAME: _ZN14CStackableItem17verifyUsablePlaceEP5CUser
// DEMANGLED: CStackableItem::verifyUsablePlace(CUser *)
int CStackableItem::verifyUsablePlace(CUser* user) {
    // `StackableAction::UsablePlace` is an IDA int enum. Its vector occupies
    // CStackableItem +940; preserve the original container rather than
    // replacing script-provided place rules with a local default.
    if (usable_places_.empty()) {
        return 1;
    }

    const int usable_place = ResolveStackableActionUsablePlace(user);
    bool place_matches =
        std::find(usable_places_.begin(), usable_places_.end(),
                  static_cast<StackableAction::UsablePlace>(usable_place)) !=
        usable_places_.end();

    // [seria room] is a narrower 0828 sub-place of the official village
    // context, not a replacement for it.  The 0627 verifier projected every
    // state-3 room (including Seria) to [village]=1.  Keep that inherited
    // permission while still requiring an exact [seria room]=4 declaration
    // outside the village-wide case: a Seria-only item remains rejected in an
    // ordinary town area, whereas a village item is also valid in Seria.
    if (!place_matches && usable_place == 4) {
        place_matches =
            std::find(usable_places_.begin(), usable_places_.end(),
                      static_cast<StackableAction::UsablePlace>(1)) !=
            usable_places_.end();
    }
    if (!place_matches) {
        return 0;
    }

    const int8_t required_first_grow_type = static_cast<int8_t>(
        arad_item_info_.values_00[1]);
    return required_first_grow_type < 0 ||
           user->getCurCharFirstGrowType() ==
               static_cast<uint8_t>(required_first_grow_type);
}
