#include "common/manager/user.h"
#include "common/manager/global_data.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/town/town_script.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/equipment/upgrade_room.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <map>
#include <stdint.h>
#include <utility>
#include <vector>

namespace {

// IDA_EA: CUserCharacInfo narrow accessors consumed by Area movement.


}  // namespace

// IDA_EA: 0x82343AA
// IDA_NAME: _ZN4Area18IsExistItemShopNPCEv
// DEMANGLED: Area::IsExistItemShopNPC(void)
int Area::IsExistItemShopNPC() {
    return has_item_shop_npc;
}

// IDA_EA: 0x86C214C
// IDA_NAME: _ZNK4Area6hasNPCEj
// DEMANGLED: Area::hasNPC(unsigned int)const
int Area::hasNPC(uint32_t npc_id) const {
    return std::find(npc_ids.begin(), npc_ids.end(),
                     static_cast<int>(npc_id)) != npc_ids.end();
}

// IDA_EA: 0x86C3162
// IDA_NAME: _ZNK4Area13IsMovalbeAreaEii
// DEMANGLED: Area::IsMovalbeArea(int,int)const
int Area::IsMovalbeArea(int target_village, int before_area) const {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             movable_links.begin();
         it != movable_links.end(); ++it) {
        if (it->first == target_village && it->second == before_area) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x86C31F2
// IDA_NAME: _ZNK4Area12GetUserCountEv
// DEMANGLED: Area::GetUserCount(void)const
int Area::GetUserCount() const {
    int count = 0;
    for (std::map<uint16_t, CUser*>::const_iterator it = users.begin();
         it != users.end(); ++it) {
        CUser* user = it->second;
        if (user && user->get_state() > 2) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x86C22F2
// IDA_NAME: Area_send_area_users_Noti24
// DEMANGLED: Area_send_area_users_Noti24
extern "C" {
void Area_send_area_users_Noti24(Area* area, CUser* user) {
    if (static_cast<unsigned char>(user->getCurCharacVill()) !=
            area->village_index || user->get_area(false) != area->area_index) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 24);
    packet.put_byte(area->village_index);
    packet.put_byte(user->get_area(true));
    int count_index = packet.get_index();
    int visible_count = 0;
    packet.put_short(static_cast<int>(area->users.size()));

    for (std::map<uint16_t, CUser*>::iterator it = area->users.begin();
         it != area->users.end();
         ++it) {
        CUser* area_user = it->second;
        if (!area_user) {
            continue;
        }
        if (area_user->IsCurCharacVisible() ||
            area_user == user) {
            packet.put_short(area_user->get_unique_id());
            packet.put_short(area_user->get_posX());
            packet.put_short(area_user->get_posY());
            packet.put_byte(area_user->get_direction());
            packet.put_byte(
                area_user->get_charac_visible_values());
            ++visible_count;
        }
    }

    packet.put_short(count_index, visible_count);
    packet.finalize(true);
    user->Send(packet);
}
}  // extern "C"

// IDA_EA: 0x86C25A6
// IDA_NAME: _ZN4Area11insert_userEP5CUser
// DEMANGLED: Area::insert_user(CUser *)
void Area::insert_user(CUser* user) {
    PacketGuard packet;

    if (town_area_type == TOWN_AREA_GATE) {
        user->set_state(3);
        packet.put_header(0, 24);
        packet.put_byte(village_index);
        packet.put_byte(user->get_area(true));
        packet.put_short(1);
        packet.put_short(user->get_unique_id());
        packet.put_short(user->get_posX());
        packet.put_short(user->get_posY());
        packet.put_byte(user->get_direction());
        packet.put_byte(user->get_charac_visible_values());
        packet.finalize(true);
        user->Send(packet);
        return;
    }

    if (town_area_type == TOWN_AREA_FISHING_PLACE) {
        user->set_state(4);
    } else {
        user->set_state(3);
    }

    const uint16_t unique_id =
        static_cast<uint16_t>(user->get_unique_id());
    if (users.find(unique_id) != users.end()) {
        LogManager::logFormat(
            1,
            "world.cpp",
            "void Area::insert_user(CUser*)",
            0x1A7,
            "m_UsersInArea.find(%d)",
            static_cast<int>(unique_id));
        return;
    }

    users.insert(std::make_pair(unique_id, user));
    packet.put_header(0, 23);
    packet.put_short(unique_id);
    packet.put_byte(village_index);
    packet.put_byte(user->get_area(true));
    packet.put_short(user->get_posX());
    packet.put_short(user->get_posY());
    packet.put_byte(user->get_direction());
    packet.put_byte(user->get_charac_visible_values());
    packet.finalize(true);

    if (user->IsCurCharacVisible()) {
        send_to_all(packet);
    } else {
        user->Send(packet);
    }
    Area_send_area_users_Noti24(this, user);

    pvp_assault::GetInstanceAssaultMgr()->OnMoveArea(user);
    GlobalData::s_villageObjectMgr->on_move_area(user);
    GlobalData::s_villageMonsterMgr->OnMoveArea(user);
}

// IDA_EA: 0x86C2A38
// IDA_NAME: _ZN4Area11delete_userEP5CUser
// DEMANGLED: Area::delete_user(CUser *)
void Area::delete_user(CUser* user) {
    if (town_area_type == TOWN_AREA_GATE) {
        return;
    }

    const uint16_t unique_id =
        static_cast<uint16_t>(user->get_unique_id());
    users.erase(unique_id);

    PacketGuard packet;
    packet.put_header(0, 23);
    packet.put_short(unique_id);
    packet.put_byte(user->getCurCharacVill());
    packet.put_byte(user->get_area(true));
    packet.put_short(user->get_posX());
    packet.put_short(user->get_posY());
    packet.put_byte(user->get_direction());
    packet.put_byte(user->get_charac_visible_values());
    packet.finalize(true);
    if (user->IsCurCharacVisible()) {
        send_to_all(packet);
    } else {
        user->Send(packet);
    }
}

// IDA_EA: 0x86C2BE4
// IDA_NAME: _ZN4Area10erase_userEP5CUser
// DEMANGLED: Area::erase_user(CUser *)
int Area::erase_user(CUser* user) {
    if (town_area_type == TOWN_AREA_GATE) {
        return static_cast<int>(town_area_type);
    }

    // Every caller of erase_user is an authoritative departure from the
    // current town-area container (dungeon/PvP/WarRoom/altar/world leave).
    // Upgrade Room sessions cannot survive that boundary.
    upgrade_room::OnUserDisconnect(user);

    const uint16_t unique_id =
        static_cast<uint16_t>(user->get_unique_id());
    return static_cast<int>(users.erase(unique_id));
}

// IDA_EA: 0x86C2D2A
// IDA_NAME: _ZN4Area11send_to_allER11PacketGuard
// DEMANGLED: Area::send_to_all(PacketGuard &)
bool Area::send_to_all(PacketGuard& packet) {
    bool sent = false;
    for (std::map<uint16_t, CUser*>::iterator it = users.begin();
         it != users.end();
         ++it) {
        CUser* user = it->second;
        if (user && user->get_state() > 2) {
            user->Send(packet);
            sent = true;
        }
    }
    return sent;
}

// IDA_EA: 0x86C2E1E
// IDA_NAME: _ZN4Area10send_groupER11PacketGuardP7CBelong
// DEMANGLED: Area::send_group(PacketGuard &,CBelong *)
bool Area::send_group(PacketGuard& packet, CBelong* group) {
    for (std::map<uint16_t, CUser*>::iterator it = users.begin();
         it != users.end();
         ++it) {
        CUser* user = it->second;
        if (user && user->get_state() > 2 && group->IsRight(user)) {
            user->Send(packet);
        }
    }

    // The original returns the final iterator comparison, which is false once
    // the complete recipient set has been traversed.
    return false;
}

// IDA_EA: 0x86C305E
// IDA_NAME: _ZN4Area16get_user_id_listERSt6vectorItSaItEE
// DEMANGLED: Area::get_user_id_list(std::vector<unsigned short,std::allocator<unsigned short>> &)
bool Area::get_user_id_list(std::vector<uint16_t>& user_ids) {
    bool inserted = false;
    for (std::map<uint16_t, CUser*>::iterator it = users.begin();
         it != users.end();
         ++it) {
        CUser* user = it->second;
        if (user && user->get_state() > 2) {
            user_ids.push_back(static_cast<uint16_t>(user->get_unique_id()));
            inserted = true;
        }
    }
    return inserted;
}

// IDA_EA: 0x86D238E
// IDA_NAME: _ZN4Area14get_area_indexEv
// DEMANGLED: Area::get_area_index(void)
int Area::get_area_index() {
    return area_index;
}

// IDA_EA: 0x86D2398
// IDA_NAME: _ZN4Area19get_world_map_indexEv
// DEMANGLED: Area::get_world_map_index(void)
int Area::get_world_map_index() {
    return world_map_index;
}
