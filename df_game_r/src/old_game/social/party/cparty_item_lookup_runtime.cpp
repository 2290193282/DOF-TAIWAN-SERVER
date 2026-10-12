#include "old_game/social/party/party.h"

#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/world/region/world_value_types.h"

// IDA_EA: 0x85A7786
// IDA_NAME: _ZN6CParty18isMonsterDrop_itemEi
// DEMANGLED: CParty::isMonsterDrop_item(int)
int CParty::isMonsterDrop_item(int unique_id) {
    map_item item;
    const int found = battle_field().get_field_item(unique_id, item);
    return !found || item.routing_eligible_ == 1;
}
