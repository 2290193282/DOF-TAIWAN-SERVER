#include "old_game/world/region/game_world.h"

#include "common/config.h"
#include "old_data/PVF/town/town_script.h"
#include "old_data/PVF/data/sync_script.h"
#include "old_game/pvp/battlefield/statistics_pvp_runtime.h"
#include "old_game/world/region/world_area_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/channel/channel_script_t.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "infra/concurrent/thread/mutex.h"

#include <cstdio>
#include <utility>
#include <vector>


namespace {

typedef char LotteryInfo_Size_Must_Be_0x20[
    (sizeof(LotteryInfo) == 0x20u) ? 1 : -1];

}  // namespace

// IDA_EA: 0x86D2108
// IDA_NAME: _ZN10SimpleInfoC2Ev
// DEMANGLED: SimpleInfo::SimpleInfo(void)
SimpleInfo::SimpleInfo()
    : account_id(0),
      forbid_chat(0),
      forbid_move(0),
      reserved(0),
      forbid_chat_until(0),
      forbid_move_until(0) {}

// IDA_EA: 0x86D214C
// IDA_NAME: _ZN4AreaC2Ev
// DEMANGLED: Area::Area(void)
Area::Area() {
    // The subobjects are constructed by C++ in the exact C2 declaration
    // order.  Only these three scalar writes occur in the original body.
    area_index = -1;
    has_item_shop_npc = 0;
    town_area_type = TOWN_AREA_GATE;
}

// IDA_EA: 0x86D2270
// IDA_NAME: _ZN4AreaD2Ev
// DEMANGLED: Area::~Area(void)
Area::~Area() {
    // The generated D2 then destroys movable_links, map_areas,
    // npc_positions, npc_ids, users, lottery_info and mutex in reverse
    // declaration order.
    area_index = -1;
}

// IDA_EA: 0x86C32E0
// IDA_NAME: _ZN7VillageC2Ev
// DEMANGLED: Village::Village(void)
Village::Village() {
    // The users map is a real C++ subobject at +0x08.  The original body only
    // clears the separately allocated Area array pointer.
    areas = 0;
}

// IDA_EA: 0x86C3300
// IDA_NAME: _ZN7VillageD2Ev
// DEMANGLED: Village::~Village(void)
Village::~Village() {
    destroy();
}

// IDA_EA: 0x86C3348
// IDA_NAME: _ZN7Village7destroyEv
// DEMANGLED: Village::destroy(void)
Village* Village::destroy() {
    if (areas) {
        delete[] areas;
        areas = 0;
    }
    return this;
}

// IDA_EA: 0x86C3D4A
// IDA_NAME: _ZN9GameWorldC2Ev
// DEMANGLED: GameWorld::GameWorld(void)
GameWorld::GameWorld() {
    // Mutex/map/statistics members are constructed automatically in the C2
    // declaration order recovered from IDA.  These are the scalar writes in
    // the original constructor body.
    crash_observation_count_ = 0;
    for (size_t index = 0;
         index < static_cast<size_t>(oldsrv::config::kCharacterJobCount);
         ++index) {
        occupation_counts_[index] = 0;
    }
    other_user_count_ = 0;
    hangame_user_count_ = 0;
}

// IDA_EA: 0x86C3F2C
// IDA_NAME: _ZN9GameWorldD2Ev
// DEMANGLED: GameWorld::~GameWorld(void)
GameWorld::~GameWorld() {
    destroy();
    for (GameWorldNatTypeStatisticsMap::iterator it =
             nat_type_statistics_.begin();
         it != nat_type_statistics_.end(); ++it) {
        delete it->second;
    }
}

// IDA_EA: 0x86C40CA
// IDA_NAME: _ZN9GameWorld7destroyEv
// DEMANGLED: GameWorld::destroy(void)
GameWorld* GameWorld::destroy() {
    if (villages_) {
        delete[] villages_;
        villages_ = 0;
    }
    return this;
}

// IDA_EA: 0x82A6898
// IDA_NAME: _ZN9GameWorld15InitChannelTypeEv
// DEMANGLED: GameWorld::InitChannelType(void)
bool GameWorld::InitChannelType() {
    channel_type_ = _GetChannelType();
    return channel_type_ != 21;
}

// IDA_EA: 0x86CD272
// IDA_NAME: _ZN9GameWorld15_GetChannelTypeEv
// DEMANGLED: GameWorld::_GetChannelType(void)
int GameWorld::_GetChannelType() {
    CEnvironment* environment = G_CEnvironment();
    const int configured_type = environment->get_channel_type();
    // The 0828 channel table adds Zombie (19) and Tournament (20).  The
    // invalid/unresolved value moved from 19 to 21 accordingly.
    if (configured_type <= 20) {
        return configured_type;
    }

    const channel_script_t* channel_script =
        G_CDataManager()->GetChannelScript();
    // Original _GetChannelType @ 0x86CD272 passes CEnvironment+0x378
    // (+888) as getChannelInfo's first key.  That physical field is
    // Server_Envir::server_group, not Server_Envir::server_type at +0x1A8.
    const channel_info_t* channel_info = channel_script->getChannelInfo(
        static_cast<uint8_t>(environment->get_server_group()),
        static_cast<uint32_t>(environment->get_channel_no()));
    return channel_info ? channel_info->channel_type : 21;
}

// IDA_EA: 0x86C4134
// IDA_NAME: _ZN9GameWorld4initEv
// DEMANGLED: GameWorld::init(void)
int GameWorld::init() {
    // The old server does not start accepting users until the PVF town list
    // has been synchronized and every Village has been built.  In particular,
    // keep the truncate failure, list ordering, and per-town DB insertion as
    // separate fail-closed steps; replacing this with an empty world makes the
    // later login/restriction startup appear to work while leaving map state
    // invalid.
    sync_script::CSyncScript* const sync =
        sync_script::GetInstanceSyncScript();
    if (sync->truncate_town_info_table() != true) {
        std::puts("Fail truncate_town_info_tablef");
        return 0;
    }

    next_unique_id_ = 0;
    initTownScript(const_cast<char*>("Town/"),
                   const_cast<char*>("Town.lst"));

    const int max_village_index = g_townScriptFileList.empty()
                                      ? 0
                                      : g_townScriptFileList.rbegin()->first;
    const int village_count = max_village_index + 1;
    MAX_VILLAGE_NUM = village_count;
    Village* const villages = new Village[village_count];
    villages_ = villages;
    village_count_ = max_village_index;

    if (IsIntegratedPvPBaseChannel()) {
        int iteration = 0;
        CServerProxyMgr<CStatisticServerProxy>* const manager =
            GlobalData::s_statistic_proxy_mgr;
        for (int server_group = manager->GetStartIndex();
             iteration < manager->GetEndIndex();
             server_group = manager->GetNextIndex(iteration)) {
            InsertChannel(G_CEnvironment()->get_gc_no_hardcode(server_group),
                          static_cast<ENUM_SERVER_GROUP>(server_group));
        }
    } else {
        InsertChannel(G_CEnvironment()->get_gc_no(), SERVER_GROUP_DEFAULT);
    }

    TownScript town;
    for (STScriptFileList::const_iterator it = g_townScriptFileList.begin();
         it != g_townScriptFileList.end(); ++it) {
        const char* const filename = it->second.c_str();
        if (!filename) {
            break;
        }
        if (importTownScript(&town, filename) != 1) {
            LogManager::logFormat(1, "world.cpp", "bool GameWorld::init()",
                                  0x3D4,
                                  "importTownScript() fail - filename(%s)",
                                  filename);
            return 0;
        }
        if (sync->insert_town_info_to_db(town) != true) {
            std::printf("insert_town_category Error: %s\n", filename);
            return 0;
        }
        if (town.town_index > village_count_) {
            LogManager::logFormat(
                1, "world.cpp", "bool GameWorld::init()", 0x3E3,
                "tempTown.townIndex_(%d) > m_iVillCount(%d) - file(%s)",
                town.town_index, village_count_, filename);
            return 0;
        }
        if (villages[town.town_index].set_village(town) != 1) {
            LogManager::logFormat(
                1, "world.cpp", "bool GameWorld::init()", 0x3E9,
                "m_pVill[tempTown.townIndex_].set_village() fail - file(%s)",
                filename);
            return 0;
        }
    }

    G_CDataManager()->GetPvPChannelGrade();
    return 1;
}
