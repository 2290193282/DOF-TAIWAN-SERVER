#include "old_game/appearance/pet/creature.h"

#include <map>

namespace WongWork {
namespace {

typedef std::map<int, Avatar_Item*> AvatarItemMap;
typedef std::map<int, int> AvatarEnduranceMap;

}  // namespace

// IDA_EA: 0x82F9A42
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr15EraseAvatarItemEi
// DEMANGLED: WongWork::CAvatarItemMgr::EraseAvatarItem(int)
Avatar_Item* CAvatarItemMgr::EraseAvatarItem(int key) {
    AvatarItemMap::iterator found = assigned_items_.find(key);
    if (found == assigned_items_.end()) return 0;

    Avatar_Item* const item = found->second;
    assigned_items_.erase(found);

    AvatarEnduranceMap::iterator endurance_found =
        emblem_reduce_rates_.find(key);
    if (endurance_found != emblem_reduce_rates_.end()) {
        emblem_reduce_rates_.erase(endurance_found);
    }
    return item;
}

}  // namespace WongWork
