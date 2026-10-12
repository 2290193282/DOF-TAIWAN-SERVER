#include "old_game/world/region/game_world.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/concurrent/timer/time_gate.h"
#include "old_data/PVF/npc/npc_script.h"
#include "old_data/PVF/town/town_script.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <map>
#include <new>
#include <utility>
#include <vector>

namespace {


}  // namespace

namespace village_object {

// IDA_EA: 0x82845AA
// IDA_NAME: _ZN14village_object7IObjectC2Ev
// DEMANGLED: village_object::IObject::IObject(void)
IObject::IObject() {}

// IDA_EA: 0x85CCADC
// IDA_NAME: _ZN14village_object7IObjectaSERKS0_
// DEMANGLED: village_object::IObject::operator=(village_object::IObject const&)
IObject& IObject::operator=(const IObject&) {
    return *this;
}

}  // namespace village_object

// IDA_EA: 0x86C1D14
// IDA_NAME: _ZN4Area8set_areaEiR14TownAreaScriptR11STMapScript
// DEMANGLED: Area::set_area(int,TownAreaScript &,STMapScript &)
void Area::set_area(int village_index,
                    TownAreaScript& area_script,
                    STMapScript& map_script) {
    npc_ids.clear();

    this->village_index = village_index;
    area_index = area_script.area_index;
    town_area_type = area_script.area_type;
    private_store_enabled = area_script.private_store_enabled;

    if (area_script.area_type == TOWN_AREA_FISHING_PLACE) {
        createLotteryInfo(&lottery_info, &area_script.linked_area_indices);
    }

    if (area_script.area_type == TOWN_AREA_DUNGEON_GATE) {
        world_map_index = area_script.linked_area_indices[0];
    } else {
        world_map_index = -1;
    }

    npc_positions.clear();
    has_item_shop_npc = 0;

    CDataManager* data_manager = G_CDataManager();
    for (std::vector<AssignNpc>::const_iterator it = map_script.npcs.begin();
         it != map_script.npcs.end(); ++it) {
        npc_positions.push_back(std::make_pair(it->x, it->y));
        npc_ids.push_back(it->npc_id);

        if (has_item_shop_npc != 1) {
            CNPCScript* npc = data_manager->find_npc(it->npc_id);
            if (npc && npc->script.roles.find(0) != npc->script.roles.end()) {
                has_item_shop_npc = 1;
            }
        }

        if (it->npc_id == 127 && GlobalData::s_timeGate_ &&
            GlobalData::s_villageObjectMgr) {
            GlobalData::s_timeGate_->SetTimeGate(
                village_index, area_script.area_index, it->npc_id,
                4490, 4540);
            village_object::Zone zone(
                static_cast<unsigned char>(village_index),
                static_cast<unsigned char>(area_script.area_index));
            GlobalData::s_villageObjectMgr->register_object(
                zone,
                GlobalData::s_timeGate_);
        }
    }

    map_areas = map_script.map_areas;

    movable_links.clear();
    for (std::vector<TownPath>::const_iterator it =
             map_script.town_paths.begin();
         it != map_script.town_paths.end(); ++it) {
        movable_links.push_back(std::make_pair(it->start_area, it->end_area));
    }

    const stVillageAttacked_t& attacked = data_manager->village_attacked_;
    const stAttackedMapInfo_t* attacked_row = 0;
    for (stVillageAttacked_t::const_iterator it = attacked.begin();
         it != attacked.end(); ++it) {
        if (it->village == village_index &&
            it->area == area_script.area_index) {
            attacked_row = &*it;
        }
    }
    if (!map_script.attacked_monsters.empty() && attacked_row &&
        GlobalData::s_villageMonsterMgr) {
        GlobalData::s_villageMonsterMgr->InsertVillageMonster(
            village_index,
            area_script.area_index,
            attacked_row->monster_group,
            attacked_row->reward_group,
            &map_script.attacked_monsters,
            &map_areas);
    }
}

// IDA_EA: 0x86C2C1E
// IDA_NAME: _ZN4Area25make_power_war_hp_of_villER11PacketGuardRi
// DEMANGLED: Area::make_power_war_hp_of_vill(PacketGuard &,int &)
bool Area::make_power_war_hp_of_vill(PacketGuard& packet, int& user_count) {
    for (std::map<uint16_t, CUser*>::iterator it = users.begin();
         it != users.end(); ++it) {
        CUser* user = it->second;
        if (user && user->get_state() > 2) {
            packet.put_short(user->getPowerWarHP());
            ++user_count;
        }
    }
    return false;
}

// IDA_EA: 0x86C2F2C
// IDA_NAME: _ZN4Area11send_to_allER11PacketGuardRSt6vectorIP5CUserSaIS4_EE
// DEMANGLED: Area::send_to_all(PacketGuard &,std::vector<CUser *,std::allocator<CUser *>> &)
bool Area::send_to_all(PacketGuard& packet, std::vector<CUser*>& touched_users) {
    for (std::map<uint16_t, CUser*>::iterator it = users.begin();
         it != users.end(); ++it) {
        CUser* user = it->second;
        for (unsigned int index = 0; index < touched_users.size(); ++index) {
            (void)touched_users[index];
        }
        if (user && user->get_state() > 2) {
            user->Send(packet);
        }
    }
    return false;
}

// IDA_EA: 0x86C33B8
// IDA_NAME: _ZN7Village11set_villageER10TownScript
// DEMANGLED: Village::set_village(TownScript &)
int Village::set_village(TownScript& town_script) {
    village_index = town_script.town_index;
    if (town_script.server_dungeon_what_must_be_cleared == -1) {
        requires_cleared_dungeon = 0;
    } else {
        requires_cleared_dungeon = 1;
        CDungeon* dungeon = G_CDataManager()->get_dungeon(
            town_script.server_dungeon_what_must_be_cleared);
        if (dungeon) {
            dungeon->village_indices_.push_back(town_script.town_index);
        }
    }

    const int source_area_count = static_cast<int>(town_script.areas.size());
    area_count = source_area_count;
    if (source_area_count == 0) {
        return 0;
    }

    int allocation_count = source_area_count;
    if (G_GameWorld()->IsSchoolPvPChannel() && town_script.town_index == 2) {
        allocation_count += 600;
        g_schoolMgr.SetSchoolArea(source_area_count, source_area_count + 600);
    } else if (town_script.town_index == 8) {
        allocation_count += 2400;
        g_guildAgitMgr.SetGuildAgitAreaDomain(source_area_count,
                                              allocation_count);
    }

    areas = new Area[allocation_count];
    required_level = town_script.limit_level;
    required_quest_id = town_script.need_quest;

    for (std::map<int, TownAreaScript>::const_iterator it =
             town_script.areas.begin();
         it != town_script.areas.end(); ++it) {
        TownAreaScript area_script(it->second);
        if (area_script.area_index < 0 ||
            area_script.area_index >= source_area_count) {
            continue;
        }

        STMapScript map_script;
        if (ImportMapScript(&map_script, area_script.area_name.c_str()) != 1) {
            return 0;
        }
        areas[area_script.area_index].set_area(
            town_script.town_index, area_script, map_script);

        if (G_GameWorld()->IsSchoolPvPChannel() &&
            town_script.town_index == 2 && area_script.area_index == 4) {
            for (int area = source_area_count;
                 area < source_area_count + 600; ++area) {
                area_script.area_index = area;
                areas[area].set_area(town_script.town_index,
                                     area_script, map_script);
            }
            area_count += 600;
        }

        if (town_script.town_index == 8 && area_script.area_index <= 3) {
            const int end_area = source_area_count + 2400;
            int begin_area = source_area_count + area_script.area_index;
            if (area_script.area_index == 3) {
                area_count += 2400;
            }
            for (int area = begin_area; area < end_area; area += 4) {
                area_script.area_index = area;
                areas[area].set_area(town_script.town_index,
                                     area_script, map_script);
            }
        }
    }
    return 1;
}
