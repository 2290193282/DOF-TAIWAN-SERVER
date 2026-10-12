#include "old_data/PVF/map/datamanager/map_runtime.h"

#include <algorithm>
#include <vector>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

// IDA unit: 09_dungeon_pvp_party/CDungeon.
// These are read-only script fields.  CDungeon's canonical B-owned layout
// now contains the exact physical containers/scalars; no accessor reaches
// into a byte facade.

// IDA_EA: 0x86B9672 call-site evidence
// IDA_NAME: _ZNK8CDungeon12getDropItemsEv
const std::vector<std::pair<int, int> >& CDungeon::getDropItems() const {
    return drop_items_;
}

// IDA_EA: 0x822B4B4
// IDA_NAME: _ZNK8CDungeon21get_limit_party_countEv
// DEMANGLED: CDungeon::get_limit_party_count(void)const
int CDungeon::get_limit_party_count() const {
    return limit_party_count_;
}

// IDA_EA: 0x826B950
// IDA_NAME: _ZNK8CDungeon18getQuestNpcDungeonEv
// DEMANGLED: CDungeon::getQuestNpcDungeon(void)const
int CDungeon::getQuestNpcDungeon() const {
    return quest_npc_dungeon_;
}

// IDA_EA: 0x822B4FA
// IDA_NAME: _ZNK8CDungeon23isTowerOfDespairDungeonEv
// DEMANGLED: CDungeon::isTowerOfDespairDungeon(void)const
bool CDungeon::isTowerOfDespairDungeon() const {
    return dungeon_inheritance_type_ == 1;
}

// IDA_EA: 0x826B944
// IDA_NAME: _ZNK8CDungeon26get_dimension_min_partymemEv
// DEMANGLED: CDungeon::get_dimension_min_partymem(void)const
int CDungeon::get_dimension_min_partymem() const {
    return dimension_min_party_members_;
}

// IDA_EA: 0x834D220
// IDA_NAME: _ZNK8CDungeon26get_dimension_member_countER19DimensionPartyCount
// DEMANGLED: CDungeon::get_dimension_member_count(DimensionPartyCount &)const
int CDungeon::get_dimension_member_count(DimensionPartyCount& count) const {
    const std::vector<DimensionPartyCount>::const_iterator it =
        std::find(dimension_party_counts_.begin(),
                  dimension_party_counts_.end(),
                  count);
    if (it == dimension_party_counts_.end()) {
        return 0;
    }
    count.third_count = it->third_count;
    return 1;
}

// IDA_EA: 0x836520A
// IDA_NAME: _ZNK8CDungeon19isTournamentDungeonEv
// DEMANGLED: CDungeon::isTournamentDungeon(void)const
int CDungeon::isTournamentDungeon() const {
    return tournament_dungeon_state_ != 0 && tournament_dungeon_state_ <= 1;
}

// IDA_EA: 0x8365238
// IDA_NAME: _ZNK8CDungeon25getTournamentRoundFatigueEv
// DEMANGLED: CDungeon::getTournamentRoundFatigue(void)const
int CDungeon::getTournamentRoundFatigue() const {
    return script_value_8a4_ ? script_value_8a4_ : 1;
}

// IDA_EA: 0x836525A
// IDA_NAME: _ZNK8CDungeon13isRiskDungeonEv
// DEMANGLED: CDungeon::isRiskDungeon(void)const
int CDungeon::isRiskDungeon() const {
    return dimension_party_count_enabled_;
}

// IDA_EA: 0x836526A
// IDA_NAME: _ZNK8CDungeon23GetPartyMemberCoinLimitEv
// DEMANGLED: CDungeon::GetPartyMemberCoinLimit(void)const
int CDungeon::GetPartyMemberCoinLimit() const {
    return runtime_value_6cc_;
}

// IDA_EA: 0x8365278
// IDA_NAME: _ZNK8CDungeon14IsEnterEachMapEv
// DEMANGLED: CDungeon::IsEnterEachMap(void)const
int CDungeon::IsEnterEachMap() const {
    return enter_each_map_;
}

// IDA_EA: 0x830E5F6
// IDA_NAME: _ZNK8CDungeon24getAICharacterAppearRateEv
// DEMANGLED: CDungeon::getAICharacterAppearRate(void)const
int CDungeon::getAICharacterAppearRate() const {
    return ai_character_appear_rate_;
}

// IDA_EA: 0x834C95C
// IDA_NAME: _ZNK8CDungeon14get_clear_itemEi
// DEMANGLED: CDungeon::get_clear_item(int)const
int CDungeon::get_clear_item(int random_number) const {
    // Its keys are the cumulative drop thresholds, so the original body uses
    // lower_bound(random_number + 1), returning -2 beyond the final key.
    const std::map<int, int>::const_iterator it =
        clear_item_rates_.lower_bound(random_number + 1);
    return it == clear_item_rates_.end() ? -2 : it->second;
}

// IDA_EA: 0x85BED84
// IDA_NAME: _ZNK8CDungeon21get_limit_Inout_countEv
// DEMANGLED: CDungeon::get_limit_Inout_count(void)const
int CDungeon::get_limit_Inout_count() const {
    return limit_inout_count_;
}
