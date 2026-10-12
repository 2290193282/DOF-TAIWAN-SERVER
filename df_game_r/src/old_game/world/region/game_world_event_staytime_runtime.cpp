#include "old_game/world/region/game_world.h"

#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x863D3BC
// IDA_NAME: _ZN9GameWorld15ProcessAllUsersIN14CEventStayTime17RewardGoGoFighterEEEvT_
// DEMANGLED: void GameWorld::ProcessAllUsers<CEventStayTime::RewardGoGoFighter>(CEventStayTime::RewardGoGoFighter)
template <>
void GameWorld::ProcessAllUsers<CEventStayTime::RewardGoGoFighter>(
    CEventStayTime::RewardGoGoFighter reward) {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() > 2) {
            reward(user);
        }
    }
}
