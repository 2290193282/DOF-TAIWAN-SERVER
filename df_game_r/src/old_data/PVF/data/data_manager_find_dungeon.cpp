#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"

// IDA_EA: 0x835F9F8
// IDA_NAME: _ZNK12CDataManager12find_dungeonEi
// DEMANGLED: CDataManager::find_dungeon(int) const
CDungeon* CDataManager::find_dungeon(int dungeon_index) const {
    // CDataManager +0x00 is the canonical CDungeonList pointer.
    return dungeon_index_list_->find_dungeon(dungeon_index);
}

// IDA_EA: 0x86D18C8
// IDA_NAME: _ZN12CDataManager11get_dungeonEi
// DEMANGLED: CDataManager::get_dungeon(int)
CDungeon* CDataManager::get_dungeon(int dungeon_index) {
    return find_dungeon(dungeon_index);
}
