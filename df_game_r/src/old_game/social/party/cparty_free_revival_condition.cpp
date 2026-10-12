#include "common/manager/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

// IDA_EA: 0x85BE39A
// IDA_NAME: _ZN6CParty25checkFreeRevivalConditionEPK8CDungeon
// DEMANGLED: CParty::checkFreeRevivalCondition(CDungeon const*)
int CParty::checkFreeRevivalCondition(const CDungeon* dungeon) {
    if (battle_field_.instance_dungeon_mode() == 1) {
        return 1;
    }

    if (!dungeon) {
        const int dungeon_index =
            battle_field_.get_dungeon_index();
        dungeon = reinterpret_cast<const CDungeon*>(
            (G_CDataManager())->find_dungeon(dungeon_index));
        if (!dungeon) {
            return 0;
        }
    }

    const int dungeon_diff = battle_field_.get_dungeon_diff();
    if (reinterpret_cast<const CDungeon*>(dungeon)->checkFreeRevivalCondition(dungeon_diff)) {
        return 1;
    }

    if (static_cast<signed char>(
            reinterpret_cast<const CDungeon*>(dungeon)->get_dimension_possible()) <= 0) {
        return 0;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(94);
    if (!repeat_event) {
        return 0;
    }
    if (repeat_event->IsEventing(0) != 1) {
        return 0;
    }

    CDimensionActivationEvent* dimension_event =
        static_cast<CDimensionActivationEvent*>(repeat_event);
    return dimension_event->checkFreeRevivalCondition(dungeon_diff) ? 1 : 0;
}
