#include "old_data/PVF/data/datamanager/data_manager.h"

#include <map>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "old_data/PVF/npc/datamanager/npc_script.h"
#include "old_data/PVF/legacy_datamanager/login_state_shard.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"

// IDA_EA: 0x835FD84
// IDA_NAME: _ZNK12CDataManager12find_monsterEi
// DEMANGLED: CDataManager::find_monster(int)const
CMonster* CDataManager::find_monster(int monster_index) const {
    return monster_manager_->find_monster(monster_index);
}

// IDA_EA: 0x836379A
// IDA_NAME: _ZN12CDataManager28get_hellparty_groupOrder_mapEi
// DEMANGLED: CDataManager::get_hellparty_groupOrder_map(int)
std::map<uint16_t, STHellPartyGroupOrder>*
CDataManager::get_hellparty_groupOrder_map(int group_index) {
    HellPartyGroupOrderMapPool* const pool = Get_instance_groupOrder_map_pool();
    HellPartyGroupOrderMapPool::iterator found = pool->find(group_index);
    return found == pool->end() ? 0 : &found->second;
}

// IDA_EA: 0x8363818
// IDA_NAME: _ZNK12CDataManager8find_npcEj
// DEMANGLED: CDataManager::find_npc(unsigned int)const
CNPCScript* CDataManager::find_npc(unsigned int npc_index) const {
    return npc_script_list_->find(npc_index);
}

// IDA_EA: 0x8360306
// IDA_NAME: _ZN12CDataManager14InitLoginStateE17ENUM_SERVER_GROUPi
// DEMANGLED: CDataManager::InitLoginState(ENUM_SERVER_GROUP,int)
int CDataManager::InitLoginState(ENUM_SERVER_GROUP login_table_no,
                                 int channel_gc_no) {
    MySQL* const db_handle =
        GlobalData::s_db_mgr->GetDBHandle(
            static_cast<ENUM_DB_HANDLE_IDX>(6), SERVER_GROUP_DEFAULT);
    const eStartArgumentType_t operation_type =
        G_CEnvironment()->get_operation_type();
    const int login_table_suffix = ResolveLoginTableSuffix(
        static_cast<int>(login_table_no), channel_gc_no);
    if (operation_type == START_ARGUMENT_START ||
        operation_type == START_ARGUMENT_NOFORK) {
        db_handle->set_query(
            "upDate login_account_%d set login_status=0 where m_channel_no=%d",
            login_table_suffix,
            channel_gc_no);
        return db_handle->exec(true) ? 1 : 0;
    }

    if (operation_type == START_ARGUMENT_START_ALL) {
        if (G_CEnvironment()->get_channel_no() == 1) {
            db_handle->set_query("upDate login_account_%d set login_status=0",
                                 login_table_suffix);
            return db_handle->exec(true) ? 1 : 0;
        }
        return 1;
    }

    return 0;
}
