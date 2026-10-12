#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "old_game/social/party/party.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/gm/world/game_master_village_attacked_commands.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "old_game/world/region/world_timer_runtime.h"
#include "old_game/world/region/world_packet_structs.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/server_utility.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <map>
#include <cstdlib>
#include <stdint.h>
#include <string>
#include <vector>



namespace village_attacked {

// IDA_EA: 0x86B4AF2
// IDA_NAME: _ZNK16village_attacked4ZoneltERKS0_
// DEMANGLED: village_attacked::Zone::operator<(village_attacked::Zone const&)const
bool Zone::operator<(const Zone& rhs) const {
    if (village < rhs.village) {
        return true;
    }
    return village == rhs.village && area < rhs.area;
}

// IDA_EA: 0x86B4B3C
// IDA_NAME: _ZN16village_attacked4ZoneC2Ehh
// DEMANGLED: village_attacked::Zone::Zone(unsigned char,unsigned char)
Zone::Zone(uint8_t village_value, uint8_t area_value)
    : village(village_value),
      area(area_value) {}

// IDA_EA: 0x86B3004
// IDA_NAME: _ZN16village_attacked15CVillageMonsterC2EtiiiPNS_19CVillageMonsterAreaEii
// DEMANGLED: village_attacked::CVillageMonster::CVillageMonster(unsigned short,int,int,int,village_attacked::CVillageMonsterArea *,int,int)
CVillageMonster::CVillageMonster(uint16_t monster_id_value,
                                 int pos_x_value,
                                 int pos_y_value,
                                 int route_value,
                                 CVillageMonsterArea* area_value,
                                 int dungeon_index_value,
                                 int dungeon_route_value)
    : uid(0),
      monster_id(monster_id_value),
      pos_x(pos_x_value),
      pos_y(pos_y_value),
      route(route_value),
      position_dirty(0),
      state(0),
      party_index(-1),
      area(area_value),
      dungeon_index(dungeon_index_value),
      dungeon_route(dungeon_route_value) {
    static uint16_t uid_seed = 0;
    const uint16_t current_seed = uid_seed;
    uid = current_seed;
    uid_seed = static_cast<uint16_t>(current_seed + 1);
    if (current_seed == 0xffffu) {
        uid_seed = 1;
    }
}

// IDA_EA: 0x86B3098
// IDA_NAME: _ZN16village_attacked15CVillageMonsterC2Ev
// DEMANGLED: village_attacked::CVillageMonster::CVillageMonster(void)
CVillageMonster::CVillageMonster()
    : uid(0),
      monster_id(0),
      pos_x(0),
      pos_y(0),
      route(0),
      position_dirty(0),
      state(0),
      party_index(-1),
      area(0),
      dungeon_index(0),
      dungeon_route(0) {}

// IDA_EA: 0x86B3106
// IDA_NAME: _ZN16village_attacked15CVillageMonsterD2Ev
// DEMANGLED: village_attacked::CVillageMonster::~CVillageMonster()
CVillageMonster::~CVillageMonster() {}

// IDA_EA: 0x86B310C
// IDA_NAME: _ZN16village_attacked15CVillageMonster19OnRunVillageMonsterEPSt6vectorI7MapAreaSaIS2_EEi
// DEMANGLED: village_attacked::CVillageMonster::OnRunVillageMonster(std::vector<MapArea,std::allocator<MapArea>> *,int)
int CVillageMonster::OnRunVillageMonster(std::vector<MapArea>* map_areas,
                                         int tick) {
    position_dirty = 0;
    int result = state;
    if (result == 0) {
        result = route;
        if (tick % 3 == result) {
            const int old_x = pos_x;
            const int old_y = pos_y;
            pos_x += get_rand_int(300) - 150;
            pos_y += get_rand_int(300) - 150;
            pvp_assault::IsIncludeVirtualMoveAreas(map_areas,
                                                   pos_x,
                                                   pos_y,
                                                   pos_x,
                                                   pos_y,
                                                   20);
            if (std::abs(pos_x - old_x) + std::abs(pos_y - old_y) > 49) {
                position_dirty = 1;
                result = reinterpret_cast<int>(this);
            } else {
                pos_x = old_x;
                pos_y = old_y;
                result = reinterpret_cast<int>(this);
            }
        }
    }
    return result;
}

// IDA_EA: 0x86B4A4C
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgrC2Ev
// DEMANGLED: village_attacked::CVillageMonsterMgr::CVillageMonsterMgr(void)
CVillageMonsterMgr::CVillageMonsterMgr()
    : raw_0(0),
      event_state(0),
      areas(),
      reward_type(0) {}

// IDA_EA: 0x86B4A80
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgrD2Ev
// DEMANGLED: village_attacked::CVillageMonsterMgr::~CVillageMonsterMgr()
CVillageMonsterMgr::~CVillageMonsterMgr() {}

// IDA_EA: 0x82F0F44
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr7OnEventEv
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnEvent(void)
bool CVillageMonsterMgr::OnEvent() {
    return this->event_state != 0;
}

// IDA_EA: 0x847DD44
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr13SetRewardTypeE28ENUM_VILLAGE_ATTACKED_REWARD
// DEMANGLED: village_attacked::CVillageMonsterMgr::SetRewardType(ENUM_VILLAGE_ATTACKED_REWARD)
CVillageMonsterMgr* CVillageMonsterMgr::SetRewardType(
    ENUM_VILLAGE_ATTACKED_REWARD reward) {
    reward_type = static_cast<int>(reward);
    return this;
}

// IDA_EA: 0x84ED3F8
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr13GetRewardTypeEv
// DEMANGLED: village_attacked::CVillageMonsterMgr::GetRewardType(void)
int CVillageMonsterMgr::GetRewardType() {
    return this->reward_type;
}

// IDA_EA: 0x86B4144
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr20InsertVillageMonsterEiiiiPSt6vectorI17STAttackedMonsterSaIS2_EEPS1_I7MapAreaSaIS6_EE
// DEMANGLED: village_attacked::CVillageMonsterMgr::InsertVillageMonster(int,int,int,int,std::vector<STAttackedMonster,std::allocator<STAttackedMonster>> *,std::vector<MapArea,std::allocator<MapArea>> *)
void CVillageMonsterMgr::InsertVillageMonster(
    int village,
    int area,
    int max_count,
    int regen_seconds,
    std::vector<STAttackedMonster>* attacked_monsters,
    std::vector<MapArea>* map_areas) {
    CVillageMonsterArea monster_area(village,
                                     area,
                                     max_count,
                                     regen_seconds,
                                     attacked_monsters,
                                     map_areas);
    areas.insert(std::make_pair(
        Zone(static_cast<uint8_t>(village), static_cast<uint8_t>(area)),
        monster_area));
}

// IDA_EA: 0x86B4280
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr22OnCreateVillageMonsterEv
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnCreateVillageMonster(void)
int CVillageMonsterMgr::OnCreateVillageMonster() {
    for (std::map<Zone, CVillageMonsterArea>::iterator it = areas.begin();
         it != areas.end();
         ++it) {
        it->second.OnCreateVillageMonster();
    }
    event_state = 2;
    return G_TimerQueue()->InsertTimer(2, 0, 133, 1, 0, 0);
}

// IDA_EA: 0x86B4338
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr21OnRegenVillageMonsterEii
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnRegenVillageMonster(int,int)
int CVillageMonsterMgr::OnRegenVillageMonster(int village, int area) {
    int result = event_state;
    if (result != 0) {
        const Zone zone(static_cast<uint8_t>(village),
                        static_cast<uint8_t>(area));
        std::map<Zone, CVillageMonsterArea>::iterator it = areas.find(zone);
        result = it != areas.end();
        if (result != 0) {
            return it->second.OnCreateVillageMonster();
        }
    }
    return result;
}

// IDA_EA: 0x86B43D4
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr23OnDestroyVillageMonsterE28ENUM_VILLAGE_ATTACKED_REWARD
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnDestroyVillageMonster(ENUM_VILLAGE_ATTACKED_REWARD)
void CVillageMonsterMgr::OnDestroyVillageMonster(
    ENUM_VILLAGE_ATTACKED_REWARD reward) {
    for (std::map<Zone, CVillageMonsterArea>::iterator it = areas.begin();
         it != areas.end();
         ++it) {
        it->second.OnDestroyVillageMonster();
    }
    event_state = 0;
    raw_0 = 0;
    reward_type = static_cast<int>(reward);

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 242);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x86B44EC
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr25OnEndRewardVillageMonsterEv
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnEndRewardVillageMonster(void)
CVillageMonsterMgr* CVillageMonsterMgr::OnEndRewardVillageMonster() {
    reward_type = 0;
    return this;
}

// IDA_EA: 0x86B44FC
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr19OnRunVillageMonsterEv
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnRunVillageMonster(void)
int CVillageMonsterMgr::OnRunVillageMonster() {
    int result = event_state;
    if (result != 0) {
        for (std::map<Zone, CVillageMonsterArea>::iterator it = areas.begin();
             it != areas.end();
             ++it) {
            it->second.OnRunVillageMonster();
        }
        return G_TimerQueue()->InsertTimer(2, 0, 133, 1, 0, 0);
    }
    return result;
}

// IDA_EA: 0x86B3666
// IDA_NAME: _ZN16village_attacked15CVillageMonster28MakeVillageMonsterListPacketER11PacketGuard
// DEMANGLED: village_attacked::CVillageMonster::MakeVillageMonsterListPacket(PacketGuard &)
int CVillageMonster::MakeVillageMonsterListPacket(PacketGuard& packet) {
    packet.put_short(this->uid);
    packet.put_short(this->monster_id);
    packet.put_byte(this->state);
    packet.put_short(this->pos_x);
    packet.put_short(this->pos_y);
    return 1;
}

// IDA_EA: 0x86B36E6
// IDA_NAME: _ZN16village_attacked15CVillageMonster32MakeVillageMonsterPositionPacketER11PacketGuard
// DEMANGLED: village_attacked::CVillageMonster::MakeVillageMonsterPositionPacket(PacketGuard &)
int CVillageMonster::MakeVillageMonsterPositionPacket(PacketGuard& packet) {
    if (position_dirty != 1) {
        return 0;
    }
    packet.put_short(uid);
    packet.put_short(pos_x);
    packet.put_short(pos_y);
    position_dirty = 0;
    return 1;
}

// IDA_EA: 0x86B4B64
// IDA_NAME: _ZNK16village_attacked15CVillageMonster6GetUIDEv
// DEMANGLED: village_attacked::CVillageMonster::GetUID(void)const
int CVillageMonster::GetUID() const {
    return uid;
}

// IDA_EA: 0x86B3240
// IDA_NAME: _ZN16village_attacked15CVillageMonster21OnFightVillageMonsterEP5CUserRiS3_
// DEMANGLED: village_attacked::CVillageMonster::OnFightVillageMonster(CUser *,int &,int &)
int CVillageMonster::OnFightVillageMonster(CUser* user,
                                          int& selected_dungeon_index,
                                          int& selected_dungeon_route) {
    if (this->state != 0) {
        return 19;
    }
    const int delta_x =
        std::abs(static_cast<unsigned short>(user->get_posX()) -
                 this->pos_x);
    const int delta_y =
        std::abs(static_cast<unsigned short>(user->get_posY()) -
                 this->pos_y);
    if (delta_x > 300 || delta_y > 300) {
        return 99;
    }

    OnChangeState(static_cast<ENUM_VILLAGE_MONSTER_STATE>(1));
    this->party_index =
        static_cast<short>(user->GetPartyIndex());
    selected_dungeon_index = dungeon_index;
    selected_dungeon_route = dungeon_route;
    return 0;
}

// IDA_EA: 0x86B330A
// IDA_NAME: _ZN16village_attacked15CVillageMonster29SendVillageMonsterFightResultEP5CUserb
// DEMANGLED: village_attacked::CVillageMonster::SendVillageMonsterFightResult(CUser *,bool)
void CVillageMonster::SendVillageMonsterFightResult(CUser* user,
                                                   bool clear) {
    if (!user) {
        return;
    }

    CParty* party = user->GetParty();
    if (!party) {
        return;
    }

    Packet_VillageMonsterFightResult monitor_packet;
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* member = party->get_user(seat);
        if (!member) {
            continue;
        }
        monitor_packet.monster_counts[seat] =
            static_cast<uint32_t>(member->GetUID());
        monitor_packet.clear_counts[seat] =
            static_cast<uint32_t>(member->get_charac_no(-1));
        if (reinterpret_cast<CVillageMonsterMgr*>(
                GlobalData::s_villageMonsterMgr)->OnEvent() &&
            clear) {
            member->IncVillageAttackCount();
        }
    }
    monitor_packet.result = clear ? 1 : 0;

    CMonitorServerProxy* proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(
                user->GetServerGroup()));
    proxy->SendTcpPacket(reinterpret_cast<char*>(&monitor_packet),
                         sizeof(monitor_packet));

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 249);
    packet.put_byte(clear ? 1 : 0);
    packet.finalize(true);
    party->send_to_party(packet);
}

// IDA_EA: 0x86B34A0
// IDA_NAME: _ZN16village_attacked15CVillageMonster20OnKillVillageMonsterEP5CUseriiib
// DEMANGLED: village_attacked::CVillageMonster::OnKillVillageMonster(CUser *,int,int,int,bool)
int CVillageMonster::OnKillVillageMonster(CUser* user,
                                         int village,
                                         int area,
                                         int regen_seconds,
                                         bool clear) {
    if (this->state != 1) {
        return 19;
    }
    if (user &&
        static_cast<short>(user->GetPartyIndex()) !=
            this->party_index) {
        return 19;
    }

    if (clear) {
        OnChangeState(static_cast<ENUM_VILLAGE_MONSTER_STATE>(2));
        G_TimerQueue()->InsertTimer(2,
                                            0,
                                            134,
                                            regen_seconds,
                                            village,
                                            area);
        SendVillageMonsterFightResult(user, true);
    } else {
        OnChangeState(static_cast<ENUM_VILLAGE_MONSTER_STATE>(0));
        SendVillageMonsterFightResult(user, clear);
    }
    return 0;
}

// IDA_EA: 0x86B35A0
// IDA_NAME: _ZN16village_attacked15CVillageMonster13OnChangeStateENS_26ENUM_VILLAGE_MONSTER_STATEE
// DEMANGLED: village_attacked::CVillageMonster::OnChangeState(village_attacked::ENUM_VILLAGE_MONSTER_STATE)
void CVillageMonster::OnChangeState(ENUM_VILLAGE_MONSTER_STATE state) {
    this->state = state;

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 240);
    packet.put_byte(this->state);
    packet.put_short(this->uid);
    packet.finalize(true);
    this->area->SendPacket(packet);
}

// IDA_EA: 0x86B3752
// IDA_NAME: _ZN16village_attacked19CVillageMonsterAreaC2EiiiiPSt6vectorI17STAttackedMonsterSaIS2_EEPS1_I7MapAreaSaIS6_EE
// DEMANGLED: village_attacked::CVillageMonsterArea::CVillageMonsterArea(int,int,int,int,std::vector<STAttackedMonster,std::allocator<STAttackedMonster>> *,std::vector<MapArea,std::allocator<MapArea>> *)
CVillageMonsterArea::CVillageMonsterArea(
    int village_value,
    int area_value,
    int max_count_value,
    int regen_seconds_value,
    std::vector<STAttackedMonster>* attacked_monsters_value,
    std::vector<MapArea>* map_areas)
    : monsters(),
      map_area_list(map_areas),
      attacked_monsters(),
      village(village_value),
      area(area_value),
      raw_30(0),
      max_count(max_count_value),
      regen_seconds(regen_seconds_value) {
    attacked_monsters = *attacked_monsters_value;
}

// IDA_EA: 0x86B4C44
// IDA_NAME: _ZN16village_attacked19CVillageMonsterAreaD2Ev
// DEMANGLED: village_attacked::CVillageMonsterArea::~CVillageMonsterArea()
CVillageMonsterArea::~CVillageMonsterArea() {}

// IDA_EA: 0x86B52F0
// IDA_NAME: _ZN16village_attacked19CVillageMonsterAreaC2ERKS0_
// DEMANGLED: village_attacked::CVillageMonsterArea::CVillageMonsterArea(village_attacked::CVillageMonsterArea const&)
CVillageMonsterArea::CVillageMonsterArea(const CVillageMonsterArea& rhs)
    : monsters(rhs.monsters),
      map_area_list(rhs.map_area_list),
      attacked_monsters(rhs.attacked_monsters),
      village(rhs.village),
      area(rhs.area),
      raw_30(rhs.raw_30),
      max_count(rhs.max_count),
      regen_seconds(rhs.regen_seconds) {}

// IDA_EA: 0x86B37FA
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea22SendVillageMonsterListEP5CUser
// DEMANGLED: village_attacked::CVillageMonsterArea::SendVillageMonsterList(CUser *)
void CVillageMonsterArea::SendVillageMonsterList(CUser* user) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 238);
    int count_index = packet.get_index();
    int monster_count = 0;
    packet.put_byte(0);

    for (std::map<uint16_t, CVillageMonster>::iterator it =
             this->monsters.begin();
         it != this->monsters.end();
         ++it) {
        if (it->second.MakeVillageMonsterListPacket(packet) == 1) {
            ++monster_count;
        }
    }

    packet.put_byte(count_index, monster_count);
    packet.finalize(true);
    if (user) {
        user->Send(packet);
    } else {
        SendPacket(packet);
    }
}

// IDA_EA: 0x86B3962
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea26SendVillageMonsterPositionEv
// DEMANGLED: village_attacked::CVillageMonsterArea::SendVillageMonsterPosition(void)
void CVillageMonsterArea::SendVillageMonsterPosition() {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 239);
    int count_index = packet.get_index();
    int monster_count = 0;
    packet.put_byte(0);

    for (std::map<uint16_t, CVillageMonster>::iterator it = monsters.begin();
         it != monsters.end();
         ++it) {
        if (it->second.MakeVillageMonsterPositionPacket(packet) == 1) {
            ++monster_count;
        }
    }

    packet.put_byte(count_index, monster_count);
    packet.finalize(true);
    if (monster_count != 0) {
        SendPacket(packet);
    }
}

// IDA_EA: 0x86B3AEA
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea18GetAttackedMonsterEi
// DEMANGLED: village_attacked::CVillageMonsterArea::GetAttackedMonster(int)
STAttackedMonster* CVillageMonsterArea::GetAttackedMonster(int random_value) {
    if (attacked_monsters.empty()) {
        return 0;
    }

    std::vector<STAttackedMonster>::iterator it = attacked_monsters.begin();
    const std::vector<STAttackedMonster>::iterator end =
        attacked_monsters.end();
    for (; it != end; ++it) {
        if (it->cumulative_count >= random_value) {
            return &*it;
        }
    }
    --it;
    return &*it;
}

// IDA_EA: 0x86B3BD4
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea22OnCreateVillageMonsterEv
// DEMANGLED: village_attacked::CVillageMonsterArea::OnCreateVillageMonster(void)
int CVillageMonsterArea::OnCreateVillageMonster() {
    if (static_cast<size_t>(max_count) <= monsters.size()) {
        return 0;
    }

    STAttackedMonster* attacked = GetAttackedMonster(get_rand_int(10000));
    if (!attacked) {
        return 0;
    }

    CVillageMonster monster(
        static_cast<uint16_t>(attacked->monster_id),
        attacked->position_x,
        attacked->position_y,
        static_cast<int>(monsters.size() % 3),
        this,
        attacked->value_14,
        attacked->value_18);
    const uint16_t uid = static_cast<uint16_t>(monster.GetUID());
    monsters.insert(std::make_pair(uid, monster));

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 246);
    packet.put_short(monster.GetUID());
    packet.put_short(attacked->monster_id);
    packet.put_byte(0);
    packet.put_short(attacked->position_x);
    packet.put_short(attacked->position_y);
    packet.finalize(true);
    SendPacket(packet);

    G_TimerQueue()->InsertTimer(2,
                                0,
                                134,
                                regen_seconds,
                                village,
                                area);
    return 1;
}

// IDA_EA: 0x86B3EE0
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea23OnDestroyVillageMonsterEv
// DEMANGLED: village_attacked::CVillageMonsterArea::OnDestroyVillageMonster(void)
void CVillageMonsterArea::OnDestroyVillageMonster() {
    monsters.clear();
    SendVillageMonsterList(0);
}

// IDA_EA: 0x86B3F06
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea19OnRunVillageMonsterEv
// DEMANGLED: village_attacked::CVillageMonsterArea::OnRunVillageMonster(void)
char CVillageMonsterArea::OnRunVillageMonster() {
    char result = G_GameWorld()->GetUserCount(village, area) == 0;
    if (!result) {
        for (std::map<uint16_t, CVillageMonster>::iterator it =
                 monsters.begin();
             it != monsters.end();
             ++it) {
            it->second.OnRunVillageMonster(map_area_list, raw_30);
        }
        SendVillageMonsterPosition();
        result = static_cast<char>(reinterpret_cast<intptr_t>(this));
        ++raw_30;
    }
    return result;
}

// IDA_EA: 0x86B3ABE
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea10OnMoveAreaEP5CUser
// DEMANGLED: village_attacked::CVillageMonsterArea::OnMoveArea(CUser *)
void CVillageMonsterArea::OnMoveArea(CUser* user) {
    if (!this->monsters.empty()) {
        SendVillageMonsterList(user);
    }
}

// IDA_EA: 0x86B3FE2
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea21OnFightVillageMonsterEP5CUsertRiS3_
// DEMANGLED: village_attacked::CVillageMonsterArea::OnFightVillageMonster(CUser *,unsigned short,int &,int &)
int CVillageMonsterArea::OnFightVillageMonster(CUser* user,
                                              unsigned short monster_uid,
                                              int& dungeon_index,
                                              int& dungeon_route) {
    std::map<uint16_t, CVillageMonster>::iterator it =
        this->monsters.find(monster_uid);
    if (it == this->monsters.end()) {
        return 21;
    }
    return it->second.OnFightVillageMonster(user,
                                            dungeon_index,
                                            dungeon_route);
}

// IDA_EA: 0x86B4070
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea20OnKillVillageMonsterEP5CUsertb
// DEMANGLED: village_attacked::CVillageMonsterArea::OnKillVillageMonster(CUser *,unsigned short,bool)
int CVillageMonsterArea::OnKillVillageMonster(CUser* user,
                                             unsigned short monster_uid,
                                             bool clear) {
    std::map<uint16_t, CVillageMonster>::iterator it =
        this->monsters.find(monster_uid);
    if (it == this->monsters.end()) {
        return 0;
    }

    int result = it->second.OnKillVillageMonster(user,
                                                  this->village,
                                                  this->area,
                                                  this->regen_seconds,
                                                  clear);
    if (clear && result == 0) {
        this->monsters.erase(it);
    }
    return result;
}

// IDA_EA: 0x86B3B9C
// IDA_NAME: _ZN16village_attacked19CVillageMonsterArea10SendPacketER11PacketGuard
// DEMANGLED: village_attacked::CVillageMonsterArea::SendPacket(PacketGuard &)
int CVillageMonsterArea::SendPacket(PacketGuard& packet) {
    G_GameWorld()->send_to_area(this->village, this->area, packet);
    return 0;
}

// IDA_EA: 0x86B45BC
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr10OnMoveAreaEP5CUser
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnMoveArea(CUser *)
void CVillageMonsterMgr::OnMoveArea(CUser* user) {
    const uint8_t area_value = static_cast<uint8_t>(
        user->get_area(false));
    const uint8_t village_value = static_cast<uint8_t>(
        user->getCurCharacVill());
    Zone zone(village_value, area_value);

    std::map<Zone, CVillageMonsterArea>::iterator it =
        this->areas.find(zone);
    if (it == this->areas.end()) {
        return;
    }

    it->second.OnMoveArea(user);
}

// IDA_EA: 0x86B4672
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr21OnFightVillageMonsterEP5CUsert
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnFightVillageMonster(CUser *,unsigned short)
int CVillageMonsterMgr::OnFightVillageMonster(CUser* user,
                                             unsigned short monster_uid) {
    Zone zone(
        static_cast<uint8_t>(user->getCurCharacVill()),
        static_cast<uint8_t>(user->get_area(false)));
    std::map<Zone, CVillageMonsterArea>::iterator it =
        this->areas.find(zone);
    if (it == this->areas.end()) {
        return 21;
    }

    CParty* party = user->GetParty();
    if (!party) {
        return 21;
    }

    int dungeon_index = 0;
    int dungeon_route = 0;
    const int result =
        it->second.OnFightVillageMonster(user,
                                         monster_uid,
                                         dungeon_index,
                                         dungeon_route);
    if (result == 0) {
        // 0x86B4795 loads getCurCharacVill() through AL, then 0x86B479A
        // sign-extends that byte before storing the old CParty+0xC50 field.
        party->battle_field().set_village_monster_battle_context(
            monster_uid,
            static_cast<signed char>(user->getCurCharacVill()),
            user->get_area(false),
            dungeon_route,
            dungeon_index);

        PacketGuard packet;
        packet.clear();
        packet.put_header(0, 243);
        packet.finalize(true);
        party->send_to_party(packet);
    }
    return result;
}

// IDA_EA: 0x86B4866
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr20OnKillVillageMonsterEP5CUserb
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnKillVillageMonster(CUser *,bool)
int CVillageMonsterMgr::OnKillVillageMonster(CUser* user,
                                            bool clear) {
    CParty* party = user->GetParty();
    if (!party) {
        return 21;
    }

    const CBattle_Field& battle_field = party->battle_field();
    const unsigned short monster_uid =
        battle_field.village_monster_timer_token();
    if (!monster_uid) {
        return 21;
    }

    Zone zone(
        static_cast<uint8_t>(battle_field.village_monster_village()),
        static_cast<uint8_t>(battle_field.village_monster_area()));
    std::map<Zone, CVillageMonsterArea>::iterator it =
        this->areas.find(zone);
    if (it == this->areas.end()) {
        return 21;
    }
    return it->second.OnKillVillageMonster(user, monster_uid, clear);
}

// IDA_EA: 0x86B4964
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr20OnKillVillageMonsterEiitb
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnKillVillageMonster(int,int,unsigned short,bool)
int CVillageMonsterMgr::OnKillVillageMonster(int village,
                                                   int area,
                                                   unsigned short monster_uid,
                                                   bool clear) {
    Zone zone(static_cast<uint8_t>(village), static_cast<uint8_t>(area));
    std::map<Zone, CVillageMonsterArea>::iterator it =
        this->areas.find(zone);
    if (it == this->areas.end()) {
        return 21;
    }
    return it->second.OnKillVillageMonster(0, monster_uid, clear);
}

// IDA_EA: 0x86B4A20
// IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr22OnStartVillageAttackedEv
// DEMANGLED: village_attacked::CVillageMonsterMgr::OnStartVillageAttacked(void)
CVillageMonsterMgr* CVillageMonsterMgr::OnStartVillageAttacked() {
    CVillageMonsterMgr* result =
        reinterpret_cast<CVillageMonsterMgr*>(event_state);
    if (result != reinterpret_cast<CVillageMonsterMgr*>(1)) {
        OnCreateVillageMonster();
        event_state = 1;
        result = this;
    }
    return result;
}

}  // namespace village_attacked

// IDA_EA: 0x8633B84
// IDA_NAME: _ZN15TimerMapLoading12dispatch_sigEiij
// DEMANGLED: TimerMapLoading::dispatch_sig(int,int,unsigned int)
int TimerMapLoading::dispatch_sig(int party_index,
                                  int timer_key,
                                  unsigned int /*unused_param*/) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(TIMER_MESSAGE_MAP_LOADING, timer_key)) {
        return 0;
    }
    party->map_load_forced_finish();
    return 1;
}

// IDA_EA: 0x8634C06
// IDA_NAME: _ZN22TimerEPLPReturnVillage12dispatch_sigEiij
// DEMANGLED: TimerEPLPReturnVillage::dispatch_sig(int,int,unsigned int)
int TimerEPLPReturnVillage::dispatch_sig(int party_index,
                                         int timer_key,
                                         unsigned int /*unused_param*/) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(TIMER_MESSAGE_EPLP_RETURN_VILLAGE,
                                timer_key)) {
        return 0;
    }
    party->ReturnToVillage();
    return 1;
}

// IDA_EA: 0x8639000
// IDA_NAME: _ZN29TimerVillageMonsterUpdateRate12dispatch_sigEiij
// DEMANGLED: TimerVillageMonsterUpdateRate::dispatch_sig(int,int,unsigned int)
int TimerVillageMonsterUpdateRate::dispatch_sig(int,
                                                int,
                                                unsigned int) {
    GlobalData::s_villageMonsterMgr->OnRunVillageMonster();
    return 1;
}

// IDA_EA: 0x863901A
// IDA_NAME: _ZN24TimerVillageMonsterRegen12dispatch_sigEiij
// DEMANGLED: TimerVillageMonsterRegen::dispatch_sig(int,int,unsigned int)
int TimerVillageMonsterRegen::dispatch_sig(int,
                                           int village,
                                           unsigned int area) {
    GlobalData::s_villageMonsterMgr->OnRegenVillageMonster(
        village, static_cast<int>(area));
    return 1;
}

// IDA_EA: 0x8639042
// IDA_NAME: _ZN25TimerStartVillageAttacked12dispatch_sigEiij
// DEMANGLED: TimerStartVillageAttacked::dispatch_sig(int,int,unsigned int)
int TimerStartVillageAttacked::dispatch_sig(int, int, unsigned int) {
    GlobalData::s_villageMonsterMgr->OnStartVillageAttacked();
    return 1;
}

// IDA_EA: 0x863905C
// IDA_NAME: _ZN31TimerVillageAttackedPlayTimeout12dispatch_sigEiij
// DEMANGLED: TimerVillageAttackedPlayTimeout::dispatch_sig(int,int,unsigned int)
int TimerVillageAttackedPlayTimeout::dispatch_sig(int party_index,
                                                  int timer_key,
                                                  unsigned int monster_token) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(TIMER_MESSAGE_VILLAGE_ATTACKED_PLAY_TIMEOUT,
                                timer_key) ||
        party->battle_field().village_monster_timer_token() !=
            monster_token) {
        return 0;
    }
    party->OnFinishVillageMonsterFighting(0, true);
    return 1;
}

// IDA_EA: 0x863949E
// IDA_NAME: _ZN36TimerStartExpEventAfterVillageAttack12dispatch_sigEiij
// DEMANGLED: TimerStartExpEventAfterVillageAttack::dispatch_sig(int,int,unsigned int)
int TimerStartExpEventAfterVillageAttack::dispatch_sig(int,
                                                       int,
                                                       unsigned int) {
    // dnf_event_log is the sole runtime enable/disable authority.  Preserve
    // the historical timer callback ABI, but do not inject a default EXP
    // event behind the operator's configured snapshot.
    return 1;
}

namespace game_master {

// IDA_EA: 0x82A8266
// IDA_NAME: _ZN11game_master24CVillageAttackedStartCmdC2Ev
// DEMANGLED: game_master::CVillageAttackedStartCmd::CVillageAttackedStartCmd(void)
CVillageAttackedStartCmd::CVillageAttackedStartCmd()
    : CCommand(), value_(0) {}

// IDA_EA: 0x84AA10C
// IDA_NAME: _ZN11game_master24CVillageAttackedStartCmd5parseEv
// DEMANGLED: game_master::CVillageAttackedStartCmd::parse(void)
void CVillageAttackedStartCmd::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    value_ = std::atoi(command_tail.c_str());
}

// IDA_EA: 0x84AA144
// IDA_NAME: _ZN11game_master24CVillageAttackedStartCmd7executeEv
// DEMANGLED: game_master::CVillageAttackedStartCmd::execute(void)
void CVillageAttackedStartCmd::execute() {
    // The original has no defensive user/proxy gate here: this command is
    // called through CCommand::executeTemplate after a parser has bound the
    // issuing user.  Retain that exact service-chain contract.
    CUser* const user = GetUser();
    Packet_VillageAttackedGMCommand packet;
    packet.uid = static_cast<uint32_t>(user->GetUID());
    packet.charac_no = user->get_charac_no(-1);
    packet.value = value_;
    packet.command = 0;

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendTcpPacket(reinterpret_cast<char*>(&packet),
                         static_cast<int>(sizeof(packet)));
}

// IDA_EA: 0x82A8282
// IDA_NAME: _ZN11game_master23CVillageAttackedStopCmdC2Ev
// DEMANGLED: game_master::CVillageAttackedStopCmd::CVillageAttackedStopCmd(void)
CVillageAttackedStopCmd::CVillageAttackedStopCmd() : CCommand() {}

// IDA_EA: 0x84AA298
// IDA_NAME: _ZN11game_master23CVillageAttackedStopCmd7executeEv
// DEMANGLED: game_master::CVillageAttackedStopCmd::execute(void)
void CVillageAttackedStopCmd::execute() {
    // Exact original body is a single `ret`; it intentionally performs no
    // state transition.  Keep the registered command instead of inventing a
    // stop workflow which the binary did not have.
}

// IDA_EA: 0x82A829E
// IDA_NAME: _ZN11game_master24CVillageAttackedFightCmdC2Ev
// DEMANGLED: game_master::CVillageAttackedFightCmd::CVillageAttackedFightCmd(void)
CVillageAttackedFightCmd::CVillageAttackedFightCmd() : CCommand() {}

// IDA_EA: 0x84AA1DC
// IDA_NAME: _ZN11game_master24CVillageAttackedFightCmd7executeEv
// DEMANGLED: game_master::CVillageAttackedFightCmd::execute(void)
void CVillageAttackedFightCmd::execute() {
    CUser* const user = GetUser();
    CParty* party = user->GetParty();
    if (!party) {
        party = G_CGameManager()->GetParty();
        if (party) {
            party->set_single_play(user);
        }
    }

    if (!party) {
        return;
    }

    const int error = party->OnFightVillageMonster(user, 0);
    if (error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_FIGHT_VILLAGE_MONSTER,
            static_cast<unsigned char>(error));
    }
}

}  // namespace game_master
