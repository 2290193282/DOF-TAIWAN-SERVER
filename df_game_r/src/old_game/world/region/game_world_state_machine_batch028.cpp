#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/town/town_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x86C8008
// IDA_NAME: _ZN9GameWorld25send_power_war_hp_of_villEi
// DEMANGLED: GameWorld::send_power_war_hp_of_vill(int)
void GameWorld::send_power_war_hp_of_vill(int village_index) {
    if (village_index == 0 || village_index <= 0 ||
        village_index >= ::MAX_VILLAGE_NUM ||
        g_townScriptFileList.size() <
            static_cast<std::size_t>(village_index)) {
        return;
    }

    Village* village = GetVillage(village_index);
    if (!village) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 227);
    int user_count_index = packet.get_index();
    packet.put_short(0);
    int user_count = 0;

    for (int area_index = 0; area_index < village->area_count; ++area_index) {
        Area* area = village->getArea(area_index);
        if (area && area->get_area_index() != -1) {
            area->make_power_war_hp_of_vill(packet, user_count);
        }
    }
    packet.put_short(user_count_index, user_count);
}

// IDA_EA: 0x86C8CC4
// IDA_NAME: _ZN9GameWorld30send_all_dungeon_inout_messageEi
// DEMANGLED: GameWorld::send_all_dungeon_inout_message(int)
void GameWorld::send_all_dungeon_inout_message(int dungeon_index) {
    PacketGuard packet;
    packet.put_header(0, 295);
    packet.put_int(dungeon_index);
    packet.put_byte(
        G_CDataManager()->get_server_parameter_script()->isDungeonOpen(
            dungeon_index));
    packet.finalize(true);

    const int minimum_level =
        getDungeonMinimumRequiredLevel(dungeon_index);
    if (minimum_level == -1) {
        return;
    }

    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* user = it->second;
        if (user->get_state() > 2 &&
            user->get_charac_level() >= minimum_level) {
            user->Send(packet);
        }
    }
}

// IDA_EA: 0x86C8E4E
// IDA_NAME: _ZN9GameWorld30send_all_dungeon_inout_messageEib
// DEMANGLED: GameWorld::send_all_dungeon_inout_message(int,bool)
void GameWorld::send_all_dungeon_inout_message(int dungeon_index, bool open) {
    PacketGuard packet;
    packet.put_header(0, 295);
    packet.put_int(dungeon_index);
    packet.put_byte(open);
    packet.finalize(true);

    const int minimum_level =
        getDungeonMinimumRequiredLevel(dungeon_index);
    if (minimum_level == -1) {
        return;
    }

    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* user = it->second;
        if (user->get_state() > 2 &&
            user->get_charac_level() >= minimum_level) {
            user->Send(packet);
        }
    }
}

// IDA_EA: 0x86C8FC8
// IDA_NAME: _ZN9GameWorld31send_user_dungeon_inout_messageEP5CUserib
// DEMANGLED: GameWorld::send_user_dungeon_inout_message(CUser *,int,bool)
void GameWorld::send_user_dungeon_inout_message(CUser* user,
                                                int dungeon_index,
                                                bool open) {
    PacketGuard packet;
    packet.put_header(0, 295);
    packet.put_int(dungeon_index);
    packet.put_byte(open);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x86C9076
// IDA_NAME: _ZN9GameWorld30getDungeonMinimumRequiredLevelEi
// DEMANGLED: GameWorld::getDungeonMinimumRequiredLevel(int)
int GameWorld::getDungeonMinimumRequiredLevel(int dungeon_index) {
    CDungeon* dungeon = G_CDataManager()->find_dungeon(dungeon_index);
    return dungeon ? dungeon->get_min_level() : -1;
}

// IDA_EA: 0x86CC340
// IDA_NAME: _ZN9GameWorld13ResetMidnightEv
// DEMANGLED: GameWorld::ResetMidnight(void)
bool GameWorld::ResetMidnight() {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* user = it->second;
        if (user->get_state() > 2) {
            user->ResetDailyCharacExpandDataMidnight();
        }
    }
    // IDA returns the terminal iterator comparison after traversal.
    return false;
}

// IDA_EA: 0x86CCC78
// IDA_NAME: _ZN9GameWorld17ResetAllEventDataEv
// DEMANGLED: GameWorld::ResetAllEventData(void)
bool GameWorld::ResetAllEventData() {
    return GlobalData::s_event_manager->dailyresetData();
}

// IDA_EA: 0x86CEB08
// IDA_NAME: _ZN9GameWorld25ResetEntireReliablePersonEv
// DEMANGLED: GameWorld::ResetEntireReliablePerson(void)
bool GameWorld::ResetEntireReliablePerson() {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* user = it->second;
        if (user->get_state() > 2 && user->getCurCharacR()) {
            user->setCurCharacTradeGoldDaily(0);
            user->SetOverTradeGoldCount(0);
        }
    }
    // IDA returns the terminal iterator comparison after traversal.
    return false;
}

// IDA_EA: 0x86CF684
// IDA_NAME: _ZN9GameWorld25ResetCacheCharactorMemoryEv
// DEMANGLED: GameWorld::ResetCacheCharactorMemory(void)
bool GameWorld::ResetCacheCharactorMemory() {
    for (GameWorldLoginUserMap::iterator it = login_users_.begin();
         it != login_users_.end(); ++it) {
        CUser* user = it->second;
        user->SetValidLastLoginCharac(false);
        user->SetLastLoginCharacNo(0);
        user->SetLastLoginChannelNo(0);
    }
    // IDA returns the terminal iterator comparison after traversal.
    return false;
}

// IDA_EA: 0x86CF746
// IDA_NAME: _ZN9GameWorld25ResetCacheCharactorMemoryEj
// DEMANGLED: GameWorld::ResetCacheCharactorMemory(unsigned int)
int GameWorld::ResetCacheCharactorMemory(unsigned int account_id) {
    GameWorldLoginUserMap::iterator found = login_users_.find(account_id);
    if (found == login_users_.end()) {
        return 0;
    }

    CUser* user = found->second;
    user->SetValidLastLoginCharac(false);
    user->SetLastLoginCharacNo(0);
    user->SetLastLoginChannelNo(0);
    return 1;
}
