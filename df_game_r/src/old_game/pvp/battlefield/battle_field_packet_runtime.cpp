#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/dungeon/mission/dungeon_mission.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/interface_packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

namespace {

void PutMonster(InterfacePacketBuf* packet, const map_monster& monster) {
    packet->put_short(monster.hell_party_group_order_);
    packet->put_int(monster.spawn_sequence_);
    packet->put_short(monster.map_monster_unique_id_);
    packet->put_int(monster.monster_id_);
    packet->put_byte(monster.monster_level_);
    packet->put_byte(monster.no_hit_animation_
                         ? 3
                         : monster.monster_kind_);
    packet->put_byte(monster.packet_value_13_);
    packet->put_byte(monster.stage_monster_type_);
}

int GetMapMoveMonsterWireOwner(CBattle_Field* field) {
    if (!field || !dungeon_runtime::IsGoldenCommerceDungeon(
                      field->get_dungeon_index())) {
        return 0;
    }
    dungeon_runtime::MapMoveMonster projection;
    if (!dungeon_runtime::GetMapMoveMonsterProjection(
            field->get_dungeon_index(), 0, &projection) ||
        projection.monster_id <= 0) {
        return 0;
    }
    return projection.monster_id;
}

void PutItem(InterfacePacketBuf* packet, const map_item& item) {
    packet->put_byte(item.packet_value_01_);
    packet->put_short(item.unique_id_);
    packet->put_int(item.item.item_id_);
    packet->put_int(item.item.get_add_info());
    packet->put_short(item.item.avatar_select_);
    packet->put_byte(item.item.amplify_option_.getAbilityType());
    packet->put_short(item.item.amplify_option_.getAbilityValue());
}

void PutMapHeader(CBattle_Field* field,
                  InterfacePacketBuf* packet,
                  int packet_id) {
    packet->put_header(0, packet_id);
    int pos_x = -1;
    int pos_y = -1;
    field->getCurPosXY(pos_x, pos_y);
    packet->put_byte(pos_x);
    packet->put_byte(pos_y);

    MapInfo* const info = field->GetCurrentMapInfo();
    packet->put_int(info->random_map_seed_);
}

}  // namespace

// IDA_EA: 0x830ECA2
// IDA_NAME: _ZN13CBattle_Field24CheckAppearRidableObjectEP11PacketGuard
// DEMANGLED: CBattle_Field::CheckAppearRidableObject(PacketGuard *)
int CBattle_Field::CheckAppearRidableObject(PacketGuard* packet) {
    int pos_x = -1;
    int pos_y = -1;
    getCurPosXY(pos_x, pos_y);
    return ridable_.CheckAppearRidableObject(party_, pos_x, pos_y, packet);
}

// IDA_EA: 0x83090A4
// IDA_NAME: _ZN13CBattle_Field10MakePacketEiPv
// DEMANGLED: CBattle_Field::MakePacket(int,void *)
void CBattle_Field::MakePacket(int packet_id, void* packet_value) {
    InterfacePacketBuf* const packet =
        reinterpret_cast<InterfacePacketBuf*>(packet_value);
    if (!packet) {
        return;
    }

    if (packet_id == 211 || packet_id == 387) {
        PutMapHeader(this, packet, packet_id);
        packet->put_byte(0);

        MapInfo* const info = GetCurrentMapInfo();
        if (info->visited_goto_) {
            packet->put_byte(0);
            packet->finalize(true);
            return;
        }

        packet->put_byte(static_cast<int8_t>(info->map_clearable_));
        packet->put_short(info->map_index_);
        if (G_CDataManager()->find_map(info->map_index_)) {
            packet->put_byte(0);
            packet->put_byte(0);
            packet->put_byte(0);
            packet->finalize(true);
        }
        return;
    }

    if (packet_id == 212) {
        packet->put_byte(selected_maze_index_);
        packet->put_byte(maze_boss_position_.first);
        packet->put_byte(maze_boss_position_.second);
        packet->put_byte(0);
        packet->put_byte(0);
        packet->put_byte(instance_dungeon_mode_);
        packet->put_byte(party_ ? party_->get_random_buff_type() : 11);
        packet->finalize(true);
        return;
    }

    if (packet_id == ENUM_NOTIPACKET_DUNGEON_INFO) {
        packet->put_byte(selected_maze_index_);
        packet->put_byte(maze_boss_position_.first);
        packet->put_byte(maze_boss_position_.second);
        packet->put_byte(quest_stage_pos_x_);
        packet->put_byte(quest_stage_pos_y_);
        packet->put_byte(instance_dungeon_mode_);

        packet->put_byte(ridable_.GetObjectCnt());
        std::vector<std::pair<int, int> > object_positions;
        ridable_.GetObjectPos(object_positions);
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 object_positions.begin();
             it != object_positions.end();
             ++it) {
            packet->put_byte(it->first);
            packet->put_byte(it->second);
        }

        int layer = 0;
        if (party_) {
            CUser* const manager = party_->getManager();
            if (manager) {
                TOD_UserState* const tod_state = static_cast<TOD_UserState*>(
                    manager->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)));
                if (tod_state) {
                    layer = tod_state->getTodayEnterLayer().GetLayer();
                }
            }
        }
        packet->put_short(layer);
        packet->put_byte(party_ ? party_->get_random_buff_type() : 11);
        champion_break::WriteDungeonInfoEnableFlag(
            *reinterpret_cast<PacketGuard*>(packet), get_dungeon_index());
        dungeon_mission::WriteDungeonInfoTail(party_, this, packet);
        packet->finalize(true);
        return;
    }

    if (packet_id == ENUM_NOTIPACKET_START_MAP) {
        packet->put_header(0, ENUM_NOTIPACKET_START_MAP);
        int pos_x = -1;
        int pos_y = -1;
        getCurPosXY(pos_x, pos_y);
        packet->put_byte(pos_x);
        packet->put_byte(pos_y);

        MapInfo* const info = GetCurrentMapInfo();
        packet->put_byte(info->completed_ ? 1 : 0);
        packet->put_int(info->random_map_seed_);
        packet->put_byte(hell_party_difficulty_);
        packet->put_byte(hell_party_event_type_);
        // CLIENT_IDA_EVIDENCE: NOTI29 sub_58A7C0 reads this u32 and stores it
        // in the active mission object's this[7].  Its vtable then creates the
        // room-local monster/object; -1 explicitly means no mission resource.
        packet->put_int(
            dungeon_mission::GetStartMapResourceIndex(party_, this));

        if (info->visited_goto_) {
            packet->put_byte(0);
            CheckAppearRidableObject(
                reinterpret_cast<PacketGuard*>(packet));
        } else {
            packet->put_byte(static_cast<int8_t>(info->map_clearable_));
            packet->put_short(info->map_index_);
            // CLIENT_IDA_EVIDENCE: ordinary NOTI29 creation goes through
            // 9CD5D0 and never inserts the object into the map-move
            // controller.  NOTI476's 4F10B0 consumer owns that insertion and
            // creates the cart itself.  Keep the authoritative map_monster on
            // the server, but do not create a duplicate client object with
            // the same uid through START_MAP.
            const int map_move_monster_id = GetMapMoveMonsterWireOwner(this);
            int ordinary_monster_count = 0;
            for (std::map<int, map_monster>::const_iterator it =
                     info->monsters_.begin();
                 it != info->monsters_.end(); ++it) {
                if (map_move_monster_id == 0 ||
                    it->second.monster_id_ != map_move_monster_id) {
                    ++ordinary_monster_count;
                }
            }
            packet->put_byte(ordinary_monster_count);
            for (std::map<int, map_monster>::const_iterator it =
                     info->monsters_.begin();
                 it != info->monsters_.end();
                 ++it) {
                if (map_move_monster_id != 0 &&
                    it->second.monster_id_ == map_move_monster_id) {
                    continue;
                }
                PutMonster(packet, it->second);
            }

            packet->put_byte(static_cast<int>(info->items_.size()));
            for (std::map<int, map_item>::const_iterator it =
                     info->items_.begin();
                 it != info->items_.end();
                 ++it) {
                PutItem(packet, it->second);
            }
            packet->put_byte(hell_party_enabled_);
            CheckAppearRidableObject(
                reinterpret_cast<PacketGuard*>(packet));
        }

        CDungeon* const dungeon = GetDungeon();
        if (dungeon && dungeon->IsEnterEachMap()) {
            const int zombie_focus =
                zombie_map_runtime::ResolveGlobalFocusSeat(this);
            // -1 means an ordinary field; -2 is a recognized but invalid
            // adapter and emits the native no-focus sentinel rather than an
            // incorrect singleton seat 0.
            packet->put_byte(zombie_focus >= 0
                                 ? zombie_focus
                                 : (zombie_focus == -1
                                        ? current_seat_no_
                                        : 255));
        } else {
            packet->put_byte(255);
        }
        packet->finalize(true);
        return;
    }

    if (packet_id == 386) {
        const int member_count = party_ ? party_->get_member_count() : 0;
        packet->put_byte(member_count);

        for (short round = 1; round <= 4; ++round) {
            packet->put_byte(round);
            std::map<short, std::vector<TournamentInfo> >::const_iterator
                found = tournament_infos_.find(round);
            const std::vector<TournamentInfo>* const infos =
                found == tournament_infos_.end() ? 0 : &found->second;
            packet->put_byte(infos ? static_cast<int>(infos->size()) : 0);
            if (!infos) {
                continue;
            }
            for (std::vector<TournamentInfo>::const_iterator info =
                     infos->begin();
                 info != infos->end();
                 ++info) {
                packet->put_byte(info->round_);
                for (int seat = 0; seat < member_count && seat < 2; ++seat) {
                    packet->put_int(info->monsters_[seat].monster_id);
                    packet->put_short(info->monsters_[seat].count);
                }
            }
        }

        std::map<int, map_monster>::const_iterator monster =
            GetCurrentMapInfo()->monsters_.begin();
        const std::map<int, map_monster>::const_iterator monster_end =
            GetCurrentMapInfo()->monsters_.end();
        for (int group = 1; group <= 4; ++group) {
            packet->put_byte(group);
            for (int seat = 0; seat < member_count && monster != monster_end;
                 ++seat, ++monster) {
                packet->put_short(monster->second.map_monster_unique_id_);
                packet->put_int(monster->second.monster_id_);
                packet->put_byte(monster->second.monster_level_);
                packet->put_byte(monster->second.monster_kind_);
            }
        }
        packet->finalize(true);
    }
}
