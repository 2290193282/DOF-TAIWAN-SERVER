#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>

// IDA_EA: 0x830E800
// IDA_NAME: _ZN7MapInfo26SelectDonsterItemDropLimitEj
// DEMANGLED: MapInfo::SelectDonsterItemDropLimit(unsigned int)
void MapInfo::SelectDonsterItemDropLimit(unsigned int limit) {
    // The original enters this path only for a nonzero limit.  It first
    // excludes every non-counted monster, then keeps randomly selected
    // entries while extending the scan for counted entries.
    if (limit == 0 ||
        monster_order_.size() - static_cast<size_t>(named_monster_count_) <=
            limit) {
        return;
    }

    std::random_shuffle(monster_order_.begin(), monster_order_.end());

    for (std::map<int, map_monster>::iterator monster = monsters_.begin();
         monster != monsters_.end();
         ++monster) {
        if (monster->second.counted_by_map_info_ != 1) {
            monster->second.drop_limit_excluded_ = 1;
        }
    }

    for (unsigned int index = 0; index < limit; ++index) {
        const int monster_unique_id = monster_order_.at(index);
        std::map<int, map_monster>::iterator monster =
            monsters_.find(monster_unique_id);
        if (monster == monsters_.end()) {
            continue;
        }

        if (monster->second.counted_by_map_info_) {
            ++limit;
        }
        monster->second.drop_limit_excluded_ = 0;
    }
}
