#include "old_game/pvp/battlefield/war_room.h"

#include "old_game/pvp/battlefield/war_area_counter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <map>

// IDA_EA: 0x86C0B8E
// IDA_NAME: _ZN8WarFieldC2Ev
WarField::WarField()
    : map_info_() {
}

// IDA_EA: 0x86C0BA4
// IDA_NAME: _ZN8WarFieldD2Ev
WarField::~WarField() {
}

// IDA_EA: 0x86B8A14
// IDA_NAME: _ZN8WarField5ResetEv
// DEMANGLED: WarField::Reset(void)
WarField* WarField::Reset() {
    map_info_.Clear();
    current_spawn_group_index_ = 0;
    return this;
}

// IDA_EA: 0x86BA1EA
// IDA_NAME: _ZN8WarField18HandleWpPerMonsterEP5CUseriiR11map_monster
// DEMANGLED: WarField::HandleWpPerMonster(CUser *,int,int,map_monster &)
int WarField::HandleWpPerMonster(CUser* user,
                                 int monster_index,
                                 int monster_hp,
                                 map_monster& monster_out) {
    std::map<int, map_monster>::iterator monster_it =
        map_info_.monsters_.find(monster_index);
    if (monster_it == map_info_.monsters_.end()) {
        return 0;
    }

    map_monster& monster = monster_it->second;
    WarAreaCounter* const counter = G_CDataManager()->get_war_area_counter();
    typedef std::map<int, stWarPoint, std::greater<int> > HpPointMap;
    HpPointMap& points_by_hp = counter->hp_to_wp_points;
    HpPointMap::iterator new_hp = points_by_hp.find(monster_hp);
    HpPointMap::iterator old_hp = points_by_hp.find(monster.spawn_weight_);
    if (new_hp == points_by_hp.end()) {
        return 0;
    }
    ++new_hp;
    if (old_hp == points_by_hp.end()) {
        return 0;
    }
    ++old_hp;
    if (monster.spawn_weight_ <= monster_hp) {
        return 0;
    }

    WpBonusPointResult bonus;
    user->checkBonusPoint();
    user->getWpBonusPoint(bonus);

    int point = 0;
    while (old_hp != new_hp) {
        if (old_hp == points_by_hp.end()) {
            break;
        }
        point += old_hp->second.values[monster.monster_kind_];
        ++old_hp;
    }
    point += bonus.normal;
    point += static_cast<int>(
        static_cast<long double>(point) *
            (static_cast<long double>(bonus.premium) / 100.0L) +
        0.5L);
    monster.spawn_weight_ = monster_hp;
    monster_out = monster;
    return point;
}

// IDA_EA: 0x86BA75C
// IDA_NAME: _ZN7WarRoomC2Ev
WarRoom::WarRoom()
    : war_field_(),
      ban_list_() {
    SetState(static_cast<WARROOM_STATE>(-1));
    for (int index = 0; index != 6; ++index) {
        war_field_.room_users_[index] = 0;
    }
}

// IDA_EA: 0x86BA7E6
// IDA_NAME: _ZN7WarRoomD2Ev
WarRoom::~WarRoom() {
}

// IDA_EA: 0x86C0574
// IDA_NAME: _ZN7WarRoom18HandleWpPerMonsterEP5CUseriiS1_
// DEMANGLED: WarRoom::HandleWpPerMonster(CUser *,int,int,CUser *)
int WarRoom::HandleWpPerMonster(CUser* user,
                                int monster_index,
                                int monster_hp,
                                CUser* target_user) {
    (void)user;  // The first user argument is not consumed by 0x86C0574.
    map_monster monster;
    if (!GetWaiterCount()) {
        return 3;
    }

    int point = 0;
    for (int slot = 0; slot <= 5; ++slot) {
        if (war_field_.room_users_[slot] && target_user &&
            war_field_.room_users_[slot] == target_user) {
            point = war_field_.HandleWpPerMonster(target_user,
                                                   monster_index,
                                                   monster_hp,
                                                   monster);
            target_user->WarAreaKill(monster.monster_kind_,
                                     point,
                                     monster.war_room_scale_a_);
        }
    }

    int16_t unique_id = -1;
    if (target_user) {
        unique_id = target_user->get_unique_id();
    }
    PacketGuard packet;
    packet.put_header(0, 337);
    packet.put_int(monster_index);
    packet.put_short(unique_id);
    packet.put_int(monster_hp);
    packet.put_short(point);
    packet.finalize(true);
    SendToRoom(packet);
    return 0;
}
