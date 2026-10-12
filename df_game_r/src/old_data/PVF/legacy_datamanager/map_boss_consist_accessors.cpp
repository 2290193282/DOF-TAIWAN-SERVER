#include "old_data/PVF/map/datamanager/map_runtime.h"

#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "common/manager/global_data.h"
#include "old_data/PVF/data/data_manager.h"

// IDA_EA: 0x8134796
// IDA_NAME: _ZNK4CMap16get_monster_listEv
// DEMANGLED: CMap::get_monster_list(void) const
std::list<_mapMonster> CMap::get_monster_list() const {
    return monsters_;
}

// IDA_EA: 0x81508C6
// IDA_NAME: _ZNK4CMap13get_item_listEv
// DEMANGLED: CMap::get_item_list(void) const
std::list<_mapItem> CMap::get_item_list() const {
    return items_;
}

// IDA_EA: 0x8373F08
// IDA_NAME: _ZNK4CMap15getDungeonListsEv
// DEMANGLED: CMap::getDungeonLists(void) const
const std::list<int>& CMap::getDungeonLists() const {
    return dungeon_lists_;
}

// IDA_EA: 0x81508F0
// IDA_NAME: _ZNK4CMap17getAPCRandomPointEv
// DEMANGLED: CMap::getAPCRandomPoint(void) const
int CMap::getAPCRandomPoint() const {
    return apc_random_point_;
}

// IDA_EA: 0x81508FC
// IDA_NAME: _ZNK4CMap15getAICharactersEv
// DEMANGLED: CMap::getAICharacters(void) const
const std::vector<STAICharacterMapArrangeData>&
CMap::getAICharacters() const {
    return ai_characters_;
}

// IDA_EA: 0x8150908
// IDA_NAME: _ZNK8CDungeon18getObjectDropItemsEv
// DEMANGLED: CDungeon::getObjectDropItems(void) const
const std::vector<std::pair<int, int> >&
CDungeon::getObjectDropItems() const {
    return object_drop_items_;
}

// IDA_EA: 0x8150914
// IDA_NAME: _ZNK8CDungeon28getSpecialPassiveObjectItemsEv
// DEMANGLED: CDungeon::getSpecialPassiveObjectItems(void) const
const std::vector<stDungeonAssignItem_t>&
CDungeon::getSpecialPassiveObjectItems() const {
    return special_passive_object_items_;
}

// IDA_EA: 0x8150920
// IDA_NAME: _ZNK8CDungeon24getRandomTowerMapIndexesEv
// DEMANGLED: CDungeon::getRandomTowerMapIndexes(void) const
const RandomTowerMapIndexTable&
CDungeon::getRandomTowerMapIndexes() const {
    return random_tower_map_indexes_;
}

// IDA_EA: 0x81455BC
// IDA_NAME: _ZNK8CDungeon22getRandomTowerMaxStageEv
// DEMANGLED: CDungeon::getRandomTowerMaxStage(void)const
int CDungeon::getRandomTowerMaxStage() const {
    return random_tower_max_stage_;
}

// IDA_EA: 0x834D2D4
// IDA_NAME: _ZNK8CDungeon21IsOpenScheduleDungeonEv
// DEMANGLED: CDungeon::IsOpenScheduleDungeon(void)const
int CDungeon::IsOpenScheduleDungeon() const {
    const int state = GlobalData::s_DailyScheduleManager->GetScheduleState(
        DAILY_SCHEDULE_DUNGEON,
        static_cast<unsigned int>(dungeon_index_));
    return state == 1 || state == 2;
}

// Typed reconstruction boundary for the IDA-proven +0x08 manager slot used
// by CBossStage::consistMap @0x814BEF4.  This is deliberately not labelled as
// an old exported identity.
CMonsterManager* CDataManager::get_boss_consist_monster_manager() {
    return monster_manager_;
}
