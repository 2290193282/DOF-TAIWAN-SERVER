#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_WAR_FIELD_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_WAR_FIELD_RUNTIME_H_

#include <stddef.h>
#include <map>
#include <stdint.h>

#include "old_game/pvp/battlefield/battle_field.h"

class CUser;
class CDungeon;
class CMap;
class PacketGuard;
enum ENUM_WALKOUT_TYPE : int;

// IDA unit: 99_misc_global/WarField.  MapInfo is the sole non-trivial
// subobject.  The remaining slots are scalar room state physically embedded
// in the original 0x1a4-byte object.
class WarField {
public:
    // IDA_EA: 0x86C0B8E
    WarField();
    // IDA_EA: 0x86C0BA4
    ~WarField();

    // IDA_EA: 0x86B8A14
    // IDA_NAME: _ZN8WarField5ResetEv
    // DEMANGLED: WarField::Reset(void)
    WarField* Reset();

    // IDA_EA: 0x86BA1EA
    // IDA_NAME: _ZN8WarField18HandleWpPerMonsterEP5CUseriiR11map_monster
    // DEMANGLED: WarField::HandleWpPerMonster(CUser *,int,int,map_monster &)
    int HandleWpPerMonster(CUser* user,
                           int monster_index,
                           int monster_hp,
                           map_monster& monster_out);

    // IDA_EA: 0x86B9672
    // IDA_NAME: _ZN8WarField17HandleMonsterKillEiR11PacketGuardR11map_monsterP5CUserPK8CDungeon
    int HandleMonsterKill(int monster_unique_id,
                          PacketGuard& packet,
                          map_monster& monster_out,
                          CUser* drop_owner,
                          const CDungeon* dungeon);

    // IDA_EA: 0x86B954C
    // IDA_NAME: _ZNK8WarField11IsGridClearEv
    bool IsGridClear() const;

    // IDA_EA: 0x86BA190
    // IDA_NAME: _ZN8WarField8DropItemE8map_item
    // DEMANGLED: WarField::DropItem(map_item)
    int DropItem(map_item item);

    // IDA_EA: 0x86B9FE6
    // IDA_NAME: _ZN8WarField12GetFieldItemEiR8map_item
    // DEMANGLED: WarField::GetFieldItem(int,map_item &)
    int GetFieldItem(int unique_id, map_item& item) const;

    // IDA_EA: 0x86BA082
    // IDA_NAME: _ZN8WarField10PickupItemESt17_Rb_tree_iteratorISt4pairIKi8map_itemEE
    // DEMANGLED: WarField::PickupItem(std::_Rb_tree_iterator<std::pair<int const,map_item>>)
    int PickupItem(std::map<int, map_item>::iterator item);

    // IDA_EA: 0x86BA0A0
    // IDA_NAME: _ZN8WarField15CheckPickupItemEiiRSt17_Rb_tree_iteratorISt4pairIKi8map_itemEE
    // DEMANGLED: WarField::CheckPickupItem(int,int,std::_Rb_tree_iterator<std::pair<int const,map_item>> &)
    int CheckPickupItem(int unique_id,
                        int picker_unique_id,
                        std::map<int, map_item>::iterator& item);

    // IDA_EA: 0x86B8A38
    // IDA_NAME: _ZN8WarField10ConsistMapEPvPK8CDungeonPK4CMaphi
    // DEMANGLED: WarField::ConsistMap(void *,CDungeon const*,CMap const*,
    //                                 unsigned char,int)
    // The binary intentionally carries the packet as void*, so retain that
    // exact public ABI rather than strengthening it in the declaration.
    void ConsistMap(void* packet,
                    const CDungeon* dungeon,
                    const CMap* map,
                    unsigned char monster_level,
                    int player_count);

    // C2/D2 only construct/destroy map_info_; all other members deliberately
    // retain the original constructor's uninitialized scalar lifetime.
    int32_t runtime_value_000_;                    // +0x000
    int32_t runtime_value_004_;                    // +0x004
    MapInfo map_info_;                             // +0x008
    int32_t current_spawn_group_index_;            // +0x0f4; Reset() writes 0
    CUser* room_users_[6];                         // +0x0f8
    int32_t room_state_;                           // +0x110
    int32_t seat_states_[6];                       // +0x114
    // GenTimerKey/CheckTimerKey index this range by TIMER_MESSAGE - 63.
    // The recovered WarRoom timers occupy messages 64 through 71.
    int32_t timer_keys_[9];                        // +0x12c
    uint8_t live_flags_[6];                        // +0x150
    uint16_t padding_156_;                         // +0x156
    // IDA 0x86BC0B0/0x86C0404 treat these as the per-seat walk-out reasons.
    ENUM_WALKOUT_TYPE walkout_types_[6];           // +0x158
    // IDA 0x86BBEA6 groups FinishPoint totals into these per-seat values.
    int32_t guild_finish_points_[6];               // +0x170
    // IDA 0x86BC5FE/0x86BE1FE accumulate and consume four float rewards.
    float clear_reward_multipliers_[4];            // +0x188
    // IDA 0x86BB876 stores CSystemTime::getCurSec(); Join reports elapsed.
    int32_t battle_started_at_sec_;                // +0x198
    CDungeon* dungeon_;                            // +0x19c
    CMap* map_;                                    // +0x1a0
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(WarField, runtime_value_000_) == 0x000u,
              "WarField leading scalar must stay at +0x000");
static_assert(offsetof(WarField, runtime_value_004_) == 0x004u,
              "WarField second scalar must stay at +0x004");
static_assert(offsetof(WarField, map_info_) == 0x008u,
              "WarField MapInfo must stay at +0x008");
static_assert(offsetof(WarField, current_spawn_group_index_) == 0x0f4u,
              "WarField spawn-group index must stay at +0x0f4");
static_assert(offsetof(WarField, room_users_) == 0x0f8u,
              "WarField user table must stay at +0x0f8");
static_assert(offsetof(WarField, room_state_) == 0x110u,
              "WarField room state must stay at +0x110");
static_assert(offsetof(WarField, seat_states_) == 0x114u,
              "WarField seat states must stay at +0x114");
static_assert(offsetof(WarField, timer_keys_) == 0x12cu,
              "WarField timer keys must stay at +0x12c");
static_assert(offsetof(WarField, live_flags_) == 0x150u,
              "WarField live flags must stay at +0x150");
static_assert(offsetof(WarField, padding_156_) == 0x156u,
              "WarField scalar alignment must stay at +0x156");
static_assert(offsetof(WarField, walkout_types_) == 0x158u,
              "WarField walk-out reasons must stay at +0x158");
static_assert(offsetof(WarField, guild_finish_points_) == 0x170u,
              "WarField guild FinishPoint table must stay at +0x170");
static_assert(offsetof(WarField, clear_reward_multipliers_) == 0x188u,
              "WarField clear-reward factors must stay at +0x188");
static_assert(offsetof(WarField, battle_started_at_sec_) == 0x198u,
              "WarField battle start time must stay at +0x198");
static_assert(offsetof(WarField, dungeon_) == 0x19cu,
              "WarField dungeon must stay at +0x19c");
static_assert(offsetof(WarField, map_) == 0x1a0u,
              "WarField map must stay at +0x1a0");
static_assert(sizeof(WarField) == 0x1a4u,
              "WarField IDA ABI size must stay 0x1a4");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_WAR_FIELD_RUNTIME_H_
