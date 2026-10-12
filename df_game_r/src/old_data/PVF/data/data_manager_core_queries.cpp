#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"

// IDA_EA: 0x8365434
// IDA_NAME: _ZN12CDataManager19getDungeonIndexListEv
// DEMANGLED: CDataManager::getDungeonIndexList(void)
CDungeonList* CDataManager::getDungeonIndexList() {
    return dungeon_index_list_->getDungeonIndexList();
}

// IDA_EA: 0x835FA14
// IDA_NAME: _ZNK12CDataManager8find_mapEi
// DEMANGLED: CDataManager::find_map(int) const
CMap* CDataManager::find_map(int map_index) const {
    return map_list_->find_map(map_index);
}

// IDA_EA: 0x835FDC6
// IDA_NAME: _ZNK12CDataManager10find_questEi
// DEMANGLED: CDataManager::find_quest(int) const
Quest* CDataManager::find_quest(int quest_index) const {
    return quest_list_->find_quest(quest_index);
}
