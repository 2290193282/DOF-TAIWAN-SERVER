#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/event/blue_marble_script.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_effect_manager_abi.h"
#include "common/manager/server_utility.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_data/PVF/data/item_generator_types.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/dungeon/ai/kill_monster_info_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/world/region/world_value_types.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/log/format/log_manager.h"

namespace {

int ApplyGlobalDropEffect(int standard_drop_rate, unsigned int effect_type) {
    CGlobalEffectManager* const effects =
        GlobalData::s_globalEffectManager_;
    if (!effects || !effects->isAffectedEffect(effect_type)) {
        return standard_drop_rate;
    }
    const float effect_value = effects->getEffectValue(effect_type);
    const unsigned int base_drop_rate = static_cast<unsigned int>(
        G_CDataManager()->getDropRate());
    if (effect_value <= static_cast<long double>(base_drop_rate)) {
        return standard_drop_rate;
    }
    standard_drop_rate += static_cast<int>(
        static_cast<long double>(standard_drop_rate) *
        (static_cast<long double>(effect_value) - base_drop_rate));
    const int effect_scaled = static_cast<int>(
        static_cast<long double>(standard_drop_rate) * effect_value);
    return std::max(0, std::min(standard_drop_rate, effect_scaled));
}

void AppendMonsterDrop(PacketGuard& packet, const map_item& item) {
    packet.put_short(static_cast<int>(item.unique_id_));
    packet.put_int(item.item.getKey());
    packet.put_byte(item.item.GetItemAttr());

    stAvatarEmblemInfo_t emblem;
    emblem.init();
    CEquipItem* avatar_item = 0;
    if (item.item.isAvatarItemType()) {
        avatar_item = static_cast<CEquipItem*>(
            G_CDataManager()->find_item(item.item.getKey()));
    }
    if (avatar_item) {
        packet.put_int(86400 * avatar_item->getAvatarPeriod(
                                   item.item.avatar_select_));
        avatar_item->getAvatarSocket(item.item.avatar_select_, emblem);
    } else {
        packet.put_int(item.item.get_add_info());
    }

    packet.put_short(item.item.avatar_select_);
    packet.put_byte(item.item.amplify_option_.getAbilityType());
    packet.put_short(item.item.amplify_option_.getAbilityValue());
    packet.put_packet(item.item);
    if (avatar_item) {
        packet.put_byte(1);
        packet.put_int(30);
        packet.put_binary(reinterpret_cast<char*>(emblem.sockets),
                          sizeof(emblem.sockets));
        // The 0828 client consumes two length-prefixed avatar blocks here:
        // the 30-byte emblem record and a four-byte expansion record.
        stAvatarExpansionInfo_t expansion;
        expansion.init();
        packet.put_int(4);
        packet.put_binary(reinterpret_cast<char*>(&expansion), 4);
    } else {
        packet.put_byte(0);
    }
    packet.put_short(item.monster_type_);
}

}  // namespace

// IDA_EA: 0x830AA46
int CBattle_Field::getItemDropDungeonType() {
    if (!dungeon_) return 3;
    return dungeon_->get_dimension_possible() > 0
               ? 2
               : (dungeon_->isAncientDungeon() ? 1 : 0);
}

// IDA_EA: 0x830B848
int CBattle_Field::setPremiumDropRate(
    WongWork::stGenerateRefData_t& reference,
    int premium_independent_drop_rate) {
    if (dungeon_->get_blood_dungeon_type() > 0 ||
        getItemDropDungeonType() != 0 ||
        dungeon_->isTowerOfDespairDungeon() ||
        reference.generator_data.monster_random_drop.is_hell_party) {
        reference.premium_independent_drop_rate = 0;
    } else {
        reference.premium_independent_drop_rate =
            premium_independent_drop_rate != 0
                ? premium_independent_drop_rate
                : -1;
    }
    return static_cast<int>(reinterpret_cast<intptr_t>(&reference));
}

// IDA_EA: 0x830D704
char CBattle_Field::KillHellPartyGroupMonsterCnt(
    MapInfo& map,
    map_monster& monster,
    bool& group_completed) {
    if (all_hell_groups_killed_) {
        return 1;
    }
    std::map<int, int>::iterator group = map.monster_kill_counts_.find(
        static_cast<int>(monster.hell_party_group_index_));
    if (group == map.monster_kill_counts_.end()) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::KillHellPartyGroupMonsterCnt(MapInfo&, map_monster&, bool&)",
            0x1DD9,
            "HELLPARTY_LOG : HELLPARTY GROUP INDEX (%d), MAP INDEX (%d) : KILL HELL GROUP ERROR!!\n",
            static_cast<int>(monster.hell_party_group_index_),
            map.map_index_);
        return 0;
    }

    monster.monster_kind_ = 0;
    if (group->second != 0) {
        --group->second;
        group_completed = false;
        return 0;
    }

    group_completed = true;
    map.monster_kill_counts_.erase(group);
    if (map.monster_kill_counts_.empty()) {
        all_hell_groups_killed_ = 1;
        return 1;
    }
    return 0;
}

// IDA_EA: 0x830ADF6
void CBattle_Field::_MakeDropItems(
    std::list<map_item>& dropped,
    MapInfo& map,
    const map_monster& monster,
    unsigned short monster_type,
    int party_member_count,
    int money_ratio,
    int standard_drop_rate,
    bool hell_group_completed,
    std::vector<bool>& first_kill_flags,
    float monster_random_drop_scale,
    int premium_independent_drop_rate) {
    WongWork::CItemGeneratorMgr* const generators =
        G_CDataManager()->getItemGenerator();
    WongWork::stGenerateRefData_t reference;
    reference.standard_drop_rate = standard_drop_rate;
    reference.monster_random_drop_scale = monster_random_drop_scale;
    reference.flags = first_kill_flags;

    const uint8_t dungeon_level = static_cast<uint8_t>(
        dungeon_->get_dimension_possible() > 0
            ? dimension_level_
            : dungeon_->get_standard_level());
    const uint8_t dungeon_difficulty =
        static_cast<uint8_t>(get_dungeon_diff());
    const int converted_role = convertAPCRoleType2MonsterRoleType(
        static_cast<char>(monster.monster_kind_));
    const bool suppress_for_circulation =
        dungeon_->circulation_statistic_class_ != 0;
    const bool suppress_for_blood = dungeon_->blood_dungeon_type_ > 0;
    const bool allow_scripted_apc_random =
        suppress_for_circulation && monster.assigned_team_ == 100 &&
        monster.monster_kind_ >= 5 && monster.monster_kind_ <= 8 &&
        monster.random_drop_count_ > 0;

    std::memset(&reference.generator_data, 0,
                sizeof(reference.generator_data));
    reference.generator_data.monster_random_drop.character_level =
        monster.monster_level_;
    reference.generator_data.monster_random_drop.monster_role =
        static_cast<int8_t>(converted_role);
    reference.generator_data.monster_random_drop.dungeon_standard_level =
        dungeon_level;
    reference.generator_data.monster_random_drop.dungeon_difficulty =
        dungeon_difficulty;
    reference.generator_data.monster_random_drop.party_member_count =
        static_cast<uint8_t>(party_member_count);
    reference.generator_data.monster_random_drop.monster_drop_count =
        static_cast<int8_t>(monster.random_drop_count_);
    reference.generator_data.monster_random_drop.bourgeois_rate = money_ratio;
    reference.generator_data.monster_random_drop.is_hell_party = 0;
    reference.generator_data.monster_random_drop.item_drop_dungeon_type =
        static_cast<uint8_t>(getItemDropDungeonType());

    WongWork::stGenerateResult_t generated;
    if (party_) {
        party_->GetLuckPoint(&generated.luck_point, true);
    }

    int blue_marble_gold_buff = 0;
    CUser* const manager = party_ ? party_->getManager() : 0;
    if (party_ && party_->IsSinglePlay() && manager &&
        manager->getMoveSpace() == 1) {
        BlueMarble* const blue_marble = manager->getBlueMarble();
        if (blue_marble) {
            blue_marble_gold_buff = blue_marble->getBuff(
                manager, BlueMarbleBuffType::kGold);
            reference.blue_marble_buff_value = static_cast<uint32_t>(
                blue_marble->getBuff(manager, BlueMarbleBuffType::kDrop));
            reference.blue_marble_context = 1;
            reference.blue_marble_character_job = static_cast<uint32_t>(
                manager->get_charac_job());
            reference.blue_marble_character_level = static_cast<uint32_t>(
                manager->get_charac_level());
        }
    }
    reference.party_member_force_drop = static_cast<uint8_t>(
        party_ ? party_->GetPartyMemberForceDropFlag() : 0);
    setPremiumDropRate(reference, premium_independent_drop_rate);

    WongWork::CItemGenerator* const monster_generator =
        generators->getGenerator(WongWork::CItemGeneratorMgr::kMonsterDrop);
    if ((!suppress_for_circulation || allow_scripted_apc_random) &&
        !suppress_for_blood &&
        monster.drop_limit_excluded_ != 1) {
        // Circulation dungeons normally suppress the broad random/world/event
        // routes.  A hostile scripted APC is narrower: its map row explicitly
        // owns a nonzero random-drop count.  Honor only that monster-local
        // count here; the world/event routes below remain suppressed.
        monster_generator->generateRandomItem(reference, generated);

        int bonus_rate = 0;
        int minimum_upgrade = 0;
        int maximum_upgrade = 0;
        if (party_) {
            for (int seat = 0; seat < 4; ++seat) {
                CUser* const member = party_->get_user(seat);
                const int member_rate =
                    champion_break::GetMagicSealDropBonusRate(member);
                if (member_rate > bonus_rate) {
                    bonus_rate = member_rate;
                    minimum_upgrade =
                        champion_break::GetMagicSealMinimumUpgrade(member);
                    maximum_upgrade =
                        champion_break::GetMagicSealMaximumUpgrade(member);
                }
            }
        }
        // RECONSTRUCTED_0828: upgrade_random_item_drop_rate(7,9,50)
        // increases the expected count of already-native +7..+9 random drops
        // by 50%.  Run one extra canonical generator attempt with that chance
        // and retain only results in the PVF upgrade interval; base drops and
        // their upgrade distribution remain untouched.
        if (bonus_rate > 0 && minimum_upgrade >= 0 &&
            maximum_upgrade >= minimum_upgrade &&
            static_cast<int>(server_utility::get_rand_int(100)) <
                std::min(100, bonus_rate)) {
            WongWork::stGenerateResult_t bonus_generated;
            monster_generator->generateRandomItem(reference, bonus_generated);
            for (std::vector<Inven_Item>::const_iterator item =
                     bonus_generated.items.begin();
                 item != bonus_generated.items.end(); ++item) {
                const int upgrade = item->GetUpgrade();
                if (upgrade >= minimum_upgrade &&
                    upgrade <= maximum_upgrade) {
                    generated.items.push_back(*item);
                }
            }
        }
    }

    std::memset(&reference.generator_data, 0,
                sizeof(reference.generator_data));
    WongWork::stGenerateRefMonsterSpecificDrop_t& specific =
        reference.generator_data.monster_specific_drop;
    specific.character_level = monster.monster_level_;
    specific.dungeon_standard_level = dungeon_level;
    specific.dungeon_difficulty = dungeon_difficulty;
    specific.retained_dungeon_difficulty = dungeon_difficulty;
    specific.monster_id = static_cast<uint32_t>(monster.monster_id_);
    specific.monster_kind = static_cast<int8_t>(monster.monster_kind_);
    specific.monster_drop_count =
        static_cast<int8_t>(monster.specific_drop_count_);
    specific.avatar_drop_context = static_cast<uint8_t>(party_member_count);
    specific.party_member_max_level = static_cast<uint8_t>(
        party_ ? party_->getMemberMaxLevel() : monster.monster_level_);
    specific.hell_party_repeat_specific_items = hell_group_completed ? 1 : 0;
    specific.hell_party_specific_path =
        monster.hell_party_group_order_ != 0 ? 1 : 0;
    specific.hell_rarity_decision_index =
        static_cast<int8_t>(monster.hell_party_difficulty_);
    specific.suppress_normal_specific_drop_a =
        suppress_for_circulation ? 1 : 0;
    specific.suppress_normal_specific_drop_b = suppress_for_blood ? 1 : 0;
    specific.catch_monster_drop_enabled = monster.completed_ != 0 ? 1 : 0;
    if (monster.drop_limit_excluded_ != 1 ||
        monster.hell_party_group_order_ != 0) {
        monster_generator->generateSpecificItem(reference, generated);
    }

    std::memset(&reference.generator_data, 0,
                sizeof(reference.generator_data));
    reference.generator_data.dungeon_drop.items =
        const_cast<std::vector<std::pair<int, int> >*>(
            &dungeon_->getDropItems());
    generators->getGenerator(WongWork::CItemGeneratorMgr::kDungeonDrop)
        ->generateSpecificItem(reference, generated);

    std::memset(&reference.generator_data, 0,
                sizeof(reference.generator_data));
    reference.generator_data.world_drop.world_area = monster.monster_level_;
    reference.generator_data.world_drop.world_index = monster.monster_level_;
    if (!suppress_for_circulation && !suppress_for_blood &&
        monster.drop_limit_excluded_ != 1) {
        int routine_count = 1;
        ServerParameterScript* const parameters =
            G_CDataManager()->get_server_parameter_script();
        if (parameters && party_member_count >= 1 && party_member_count <= 4) {
            routine_count = parameters->getDropRoutineByParty(
                party_member_count);
        }
        WongWork::CItemGenerator* const world_generator =
            generators->getGenerator(WongWork::CItemGeneratorMgr::kWorldDrop);
        for (int index = 0; index < routine_count; ++index) {
            world_generator->generateRandomItem(reference, generated);
        }
    }

    const bool pc_room_player = party_ && party_->hasPCRoomPlayer();
    if (!suppress_for_circulation && !suppress_for_blood &&
        monster.drop_limit_excluded_ != 1 && GlobalData::s_event_manager) {
        CEventBase* const event22 =
            GlobalData::s_event_manager->GetRepeatEvent(22);
        if (event22 && event22->IsEventing(0) &&
            (pc_room_player ||
             pc_room_world_drop_event::AllowsAllAccounts())) {
            generators->getGenerator(
                WongWork::CItemGeneratorMgr::kPcRoomWorldDrop)
                ->generateRandomItem(reference, generated);
        }
        if (pc_room_player) {
            CEventBase* const event48 =
                GlobalData::s_event_manager->GetRepeatEvent(48);
            if (event48 && event48->IsEventing(0)) {
                generators->getGenerator(
                    WongWork::CItemGeneratorMgr::kPcRoom2WorldDrop)
                    ->generateRandomItem(reference, generated);
            }
        }
    }
    if (pc_room_player) {
        generators->getGenerator(
            WongWork::CItemGeneratorMgr::kReservedWorldDrop)
            ->generateRandomItem(reference, generated);
    }

    BlueMarbleScriptManager* const blue_marble_script =
        G_CDataManager()->getBlueMarbleScript();
    const int created_at = GlobalData::s_systemTime_.getCurSec();
    for (size_t index = 0; index < generated.items.size(); ++index) {
        map_item dropped_item;
        dropped_item.routing_eligible_ = 1;
        dropped_item.unique_id_ = static_cast<uint32_t>(
            map.next_drop_item_unique_id_++);
        dropped_item.monster_type_ = monster_type;
        dropped_item.created_at_sec_ = static_cast<uint32_t>(created_at);
        dropped_item.item = generated.items[index];
        if (reference.blue_marble_context && manager && blue_marble_script &&
            dropped_item.item.getKey() == 0) {
            if (blue_marble_gold_buff <= 0) blue_marble_gold_buff = 1;
            const float gold_scale =
                blue_marble_script->info_script.gold_base +
                blue_marble_gold_buff *
                    blue_marble_script->info_script.gold_multi;
            dropped_item.item.set_add_info(static_cast<int>(
                dropped_item.item.get_add_info() * gold_scale));
        }
        dropped_item.luck_point_recipient_ =
            generated.luck_point.GetUserGainItem(static_cast<int>(index));
        dropped.push_back(dropped_item);
        map.Add_Item(dropped_item);
    }
}

// IDA_EA: 0x830B8CC
int CBattle_Field::_MakeNotiPacketDieMonster(
    PacketGuard& packet,
    std::list<map_item>& dropped,
    int monster_unique_id,
    unsigned short /*monster_type*/,
    const map_monster& live_monster,
    map_monster& killed_monster) {
    packet.put_header(0, ENUM_NOTIPACKET_DIE_MONSTER);
    packet.put_short(monster_unique_id);
    packet.put_byte(static_cast<int>(dropped.size()));
    for (std::list<map_item>::const_iterator item = dropped.begin();
         item != dropped.end(); ++item) {
        AppendMonsterDrop(packet, *item);
    }
    // CLIENT_IDA_EVIDENCE: sub_583060 @ 0x58343F reads this discriminator.
    // Kind 0 is the ordinary dungeon branch; kind 1 is the separate WarRoom
    // UID/WP branch used by WarRoom::HandleMonsterKill.
    packet.put_byte(0);
    if (convertAPCRoleType2MonsterRoleType(live_monster.monster_kind_) != 0) {
        // CLIENT_IDA_EVIDENCE: sub_583060 @ 0x583659 compares this signed byte
        // with the same client global that NOTI50/KEY_COUNT writes at
        // sub_590BE0 @ 0x590C09.  -1 suppresses the update; this is not a
        // per-packet death sequence.
        packet.put_byte(key_count_);
    } else {
        packet.put_byte(-1);
    }
    packet.put_byte(killed_monster.no_hit_animation_);
    if (killed_monster.no_hit_animation_ != 0) {
        // Original _MakeNotiPacketDieMonster @0x830B8CC publishes the next
        // dummy-boss position here.  Sending the just-cleared current room
        // makes the 140828 client overwrite its minimap boss marker backwards.
        packet.put_byte(maze_boss_position_.first);
        packet.put_byte(maze_boss_position_.second);
    }
    return packet.finalize(true);
}

// IDA_EA: 0x830BC78
// IDA_NAME: _ZN13CBattle_Field12kill_monsterER11PacketGuardR11map_monsterRjRK17STKillMonsterData
// DEMANGLED: CBattle_Field::kill_monster(PacketGuard &,map_monster &,
//             unsigned int &,STKillMonsterData const&)
int CBattle_Field::kill_monster(PacketGuard& packet,
                                map_monster& killed_monster,
                                unsigned int& gained_exp,
                                const STKillMonsterData& data) {
    MapInfo* const map = GetCurrentMapInfo();
    std::map<int, map_monster>::iterator found =
        map->monsters_.find(static_cast<int>(data.monster_unique_id));
    if (found == map->monsters_.end()) {
        LogManager::logFormat(
            1,
            "dungeon42_diag.cpp",
            "bool CBattle_Field::kill_monster(PacketGuard&, map_monster&, uint&, const STKillMonsterData&)",
            0,
            "[DUNGEON42][LOOKUP_MISS] monster_uid=%u grid=(%d,%d) map_index=%d live_count=%u",
            static_cast<unsigned int>(data.monster_unique_id),
            GetCurPosX(),
            GetCurPosY(),
            map->map_index_,
            static_cast<unsigned int>(map->monsters_.size()));
        for (std::map<int, map_monster>::const_iterator it =
                 map->monsters_.begin();
             it != map->monsters_.end();
             ++it) {
            LogManager::logFormat(
                1,
                "dungeon42_diag.cpp",
                "bool CBattle_Field::kill_monster(PacketGuard&, map_monster&, uint&, const STKillMonsterData&)",
                0,
                "[DUNGEON42][LIVE] key=%d object_uid=%d monster_index=%d role=%d weight=%d packet_value=%u",
                it->first,
                it->second.map_monster_unique_id_,
                it->second.monster_id_,
                static_cast<int>(it->second.monster_kind_),
                it->second.spawn_weight_,
                static_cast<unsigned int>(it->second.packet_value_13_));
        }
        return 0;
    }

    map_monster& live_monster = found->second;
    if (data.mark_monster_completed != 0) {
        live_monster.completed_ = 1;
    }

    // 0627 official CBattle_Field::kill_monster @0x830C0DA..0x830C1A5:
    // tournament opponents are mirrored in repeated_monsters_.  Removing the
    // live opponent from that mirror is the authoritative end-of-round edge;
    // when no opponent remains for the key, the next round consumes its PVF
    // fatigue and advances tournament_cur_round_.  The 0828 client later uses
    // this exact value as NOTI388 byte 0 (placement/EXP/reward tier).
    if (dungeon_ && dungeon_->isTournamentDungeon()) {
        const int tournament_round = GetTournamentCurRound();
        const int monster_key = found->first;
        const std::pair<std::multimap<int, map_monster>::iterator,
                        std::multimap<int, map_monster>::iterator> range =
            map->repeated_monsters_.equal_range(monster_key);
        for (std::multimap<int, map_monster>::iterator repeated = range.first;
             repeated != range.second;
             ++repeated) {
            if (repeated->second.map_monster_unique_id_ ==
                live_monster.map_monster_unique_id_) {
                map->repeated_monsters_.erase(repeated);
                break;
            }
        }

        if (map->repeated_monsters_.find(monster_key) ==
                map->repeated_monsters_.end() &&
            tournament_round != 4) {
            const int round_fatigue = dungeon_->getTournamentRoundFatigue();
            for (int seat = 0; seat <= 3; ++seat) {
                if (party_ && party_->checkValidUser(seat) == 1) {
                    CUser* const member = party_->get_user(seat);
                    if (member) {
                        member->FatigueUp(round_fatigue);
                        member->SendFatigue();
                    }
                }
            }
            SetTournamentCurRound(tournament_round + 1);
        }
    }

    bool hell_party_group_completed = false;
    if (live_monster.hell_party_group_order_ != 0) {
        KillHellPartyGroupMonsterCnt(*map,
                                     live_monster,
                                     hell_party_group_completed);
    }
    if (live_monster.monster_kind_ > 8) {
        LogManager::logFormat(
            1, "dungeon42_diag.cpp",
            "bool CBattle_Field::kill_monster(PacketGuard&, map_monster&, uint&, const STKillMonsterData&)",
            0,
            "[DUNGEON42][ROLE_REJECT] monster_uid=%u monster_index=%d role=%d",
            static_cast<unsigned int>(data.monster_unique_id),
            live_monster.monster_id_,
            static_cast<int>(live_monster.monster_kind_));
        return 0;
    }
    if (live_monster.monster_kind_ <= 3) {
        if (!G_CDataManager()->find_monster(live_monster.monster_id_) &&
            (live_monster.hell_party_group_order_ == 0 ||
             !G_CDataManager()->get_ai_character_list()->get(
                 static_cast<uint32_t>(live_monster.monster_id_)))) {
            LogManager::logFormat(
                1, "dungeon42_diag.cpp",
                "bool CBattle_Field::kill_monster(PacketGuard&, map_monster&, uint&, const STKillMonsterData&)",
                0,
                "[DUNGEON42][MONSTER_DEF_MISS] monster_uid=%u monster_index=%d role=%d",
                static_cast<unsigned int>(data.monster_unique_id),
                live_monster.monster_id_,
                static_cast<int>(live_monster.monster_kind_));
            return 0;
        }
    } else if (!G_CDataManager()->get_ai_character_list()->get(
                   static_cast<uint32_t>(live_monster.monster_id_))) {
        LogManager::logFormat(
            1, "dungeon42_diag.cpp",
            "bool CBattle_Field::kill_monster(PacketGuard&, map_monster&, uint&, const STKillMonsterData&)",
            0,
            "[DUNGEON42][AI_DEF_MISS] monster_uid=%u monster_index=%d role=%d",
            static_cast<unsigned int>(data.monster_unique_id),
            live_monster.monster_id_,
            static_cast<int>(live_monster.monster_kind_));
        return 0;
    }

    killed_monster = live_monster;

    const bool server_named = killed_monster.isNamedMonster();
    const int wire_kind = killed_monster.no_hit_animation_
        ? 3
        : static_cast<int>(killed_monster.monster_kind_);
    const bool server_apc = killed_monster.monster_kind_ >= 5 &&
                            killed_monster.monster_kind_ <= 8;
    if (server_named || killed_monster.monster_kind_ == 1 ||
        killed_monster.monster_kind_ == 2 || server_apc) {
        LogManager::logFormat(
            1,
            "quest_monster_diag.cpp",
            "int CBattle_Field::kill_monster(PacketGuard&,map_monster&,unsigned int&,const STKillMonsterData&)",
            0,
            "[QUEST_MONSTER][NOTI38] dungeon=%d difficulty=%d map=%d uid=%d template=%d kind=%d wire_kind=%d named=%d no_hit=%u team=%d packet_value=%u",
            dungeon_index_,
            get_dungeon_diff(),
            map->map_index_,
            killed_monster.map_monster_unique_id_,
            killed_monster.monster_id_,
            static_cast<int>(killed_monster.monster_kind_),
            wire_kind,
            server_named ? 1 : 0,
            static_cast<unsigned int>(killed_monster.no_hit_animation_),
            killed_monster.assigned_team_,
            static_cast<unsigned int>(killed_monster.packet_value_13_));
    }

    // The official owner of hunt-monster progress is the client chain
    // NOTI38 -> event 3 -> CMD33.  This trace deliberately observes the
    // server-side classification without decrementing, avoiding a double
    // count when the legitimate CMD33 follows.
    if (party_) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (party_->checkValidUser(seat) != 1) {
                continue;
            }
            CUser* const member = party_->get_user(seat);
            const UserQuest* const quest_state = member
                ? member->getCurCharacQuestR()
                : 0;
            if (!member || !quest_state) {
                continue;
            }
            for (int quest_slot = 0; quest_slot <= 19; ++quest_slot) {
                const int quest_index = static_cast<int>(
                    quest_state->play_quest[quest_slot]);
                Quest* const quest = G_CDataManager()->find_quest(quest_index);
                if (!quest || quest->clear_condition_values.size() < 5u) {
                    continue;
                }
                const std::vector<int>& values =
                    quest->clear_condition_values;
                const bool quest_item_drop =
                    quest->trigger_type == 2 && values.front() == -2;
                const bool hunt_enemy = quest->trigger_type == 6;
                if (!quest_item_drop && !hunt_enemy) {
                    continue;
                }
                const int area = hunt_enemy ? values[0] : values[1];
                const int min_difficulty =
                    hunt_enemy ? values[1] : values[2];
                const int selector = hunt_enemy ? values[2] : values[3];
                const int target_enemy_type = hunt_enemy ? values[3] : -1;
                const int server_enemy_type = server_apc ? 2 : 1;
                const bool selector_match =
                    (selector >= 0 &&
                     selector == killed_monster.monster_id_) ||
                    (selector == -1 && !server_named &&
                     killed_monster.monster_kind_ == 0) ||
                    (selector == -2 && server_named) ||
                    (selector == -3 &&
                     (killed_monster.monster_kind_ == 3 ||
                      killed_monster.monster_kind_ == 8)) ||
                    (selector == -4 &&
                     (killed_monster.monster_kind_ == 1 ||
                      killed_monster.monster_kind_ == 2 ||
                      killed_monster.monster_kind_ == 6 ||
                       killed_monster.monster_kind_ == 7));
                const bool enemy_type_match =
                    !hunt_enemy || target_enemy_type == server_enemy_type;
                const bool contract_match =
                    selector_match && enemy_type_match;
                const int packed = static_cast<int>(
                    quest_state->play_quest_trigger[quest_slot]);
                LogManager::logFormat(
                    1,
                    "quest_monster_diag.cpp",
                    "int CBattle_Field::kill_monster(PacketGuard&,map_monster&,unsigned int&,const STKillMonsterData&)",
                    0,
                    "[QUEST_MONSTER][KILL_CONTEXT] account=%d quest=%d slot=%d trigger_type=%d group=0 area=%d min_diff=%d selector=%d target_enemy_type=%d server_enemy_type=%d dungeon=%d difficulty=%d map=%d monster_uid=%d template=%d kind=%d wire_kind=%d named=%d selector_match=%d enemy_type_match=%d contract_match=%d packed=%d lanes=%d/%d/%d",
                    member->get_acc_id(),
                    quest_index,
                    quest_slot,
                    quest->trigger_type,
                    area,
                    min_difficulty,
                    selector,
                    target_enemy_type,
                    server_enemy_type,
                    dungeon_index_,
                    get_dungeon_diff(),
                    map->map_index_,
                    killed_monster.map_monster_unique_id_,
                    killed_monster.monster_id_,
                    static_cast<int>(killed_monster.monster_kind_),
                    wire_kind,
                    server_named ? 1 : 0,
                    selector_match ? 1 : 0,
                    enemy_type_match ? 1 : 0,
                    contract_match ? 1 : 0,
                    packed,
                    packed & 0x1FF,
                    (packed >> 9) & 0x1FF,
                    (packed >> 18) & 0x1FF);
            }
        }
    }

    std::vector<bool> first_kill_flags(2, false);
    if (party_) {
        CUser* const manager = party_->getManager();
        CKillMonsterInfo* const kill_info = manager
            ? static_cast<CKillMonsterInfo*>(manager->GetCharacExpandData(
                  static_cast<ENUM_CHARAC_EXPAND_TYPE>(12)))
            : 0;
        const uint8_t dungeon_difficulty =
            static_cast<uint8_t>(get_dungeon_diff());
        bool tracked_monster = false;
        if (kill_info && killed_monster.monster_kind_ == 3) {
            tracked_monster = true;
            if (!kill_info->CheckBossKill(killed_monster.monster_id_,
                                          dungeon_difficulty) &&
                kill_info->RegisterBossKill(killed_monster.monster_id_,
                                            dungeon_difficulty)) {
                first_kill_flags[0] = true;
            }
        } else if (kill_info && killed_monster.monster_kind_ == 8) {
            tracked_monster = true;
            if (!kill_info->CheckAPCBossKill(killed_monster.monster_id_,
                                             dungeon_difficulty) &&
                kill_info->RegisterAPCBossKill(killed_monster.monster_id_,
                                               dungeon_difficulty)) {
                first_kill_flags[0] = true;
            }
        } else if (kill_info && killed_monster.isNamedMonster()) {
            tracked_monster = true;
            if (!kill_info->CheckNamedKill(killed_monster.monster_id_,
                                           dungeon_difficulty) &&
                kill_info->RegisterNamedKill(killed_monster.monster_id_,
                                             dungeon_difficulty)) {
                first_kill_flags[1] = true;
            }
        }
        if (tracked_monster && !party_->IsSinglePlay()) {
            party_->MemberRegisterKillMonster(killed_monster,
                                              dungeon_difficulty);
        }
    }

    std::list<map_item> dropped;
    if (killed_monster.assigned_team_ == 100 &&
        data.suppress_random_drop != 1 && instance_dungeon_mode_ != 1) {
        int premium_user_count = 0;
        for (int seat = 0; seat <= 3; ++seat) {
            if (party_ && party_->checkValidUser(seat) == 1) {
                CUser* const member = party_->get_user(seat);
                if (member && member->isAffectedPremium(
                                  static_cast<ENUM_PREMIUM_TYPE>(79))) {
                    ++premium_user_count;
                }
            }
        }
        int premium_independent_drop_rate = 0;
        if (premium_user_count > 0 && premium_user_count <= 4) {
            for (int seat = 0; seat <= 3; ++seat) {
                if (party_ && party_->checkValidUser(seat) == 1) {
                    CUser* const member = party_->get_user(seat);
                    if (member) {
                        premium_independent_drop_rate =
                            member->getAddIndependentDropRateFromPremium(
                                static_cast<ENUM_PREMIUM_TYPE>(79),
                                static_cast<unsigned int>(premium_user_count));
                        break;
                    }
                }
            }
        }
        // variousbufflist index2 supplies {rarity-context=3, rate=120}; the
        // independent-drop owner already applies this percentage only when
        // generator-data+0x08 equals context 3.
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const member = party_ ? party_->get_user(seat) : 0;
            premium_independent_drop_rate = std::max(
                premium_independent_drop_rate,
                champion_break::GetUniqueDropBonusRate(member));
        }
        int adjusted_standard_drop_rate = data.standard_drop_rate;
        adjusted_standard_drop_rate = ApplyGlobalDropEffect(
            adjusted_standard_drop_rate, 1u);
        adjusted_standard_drop_rate = ApplyGlobalDropEffect(
            adjusted_standard_drop_rate, 4u);
        _MakeDropItems(dropped,
                       *map,
                       killed_monster,
                       data.monster_type,
                       data.party_member_count,
                       data.money_ratio,
                       adjusted_standard_drop_rate,
                       hell_party_group_completed,
                       first_kill_flags,
                       data.random_drop_scale,
                       premium_independent_drop_rate);
    }

    if (convertAPCRoleType2MonsterRoleType(
            static_cast<char>(live_monster.monster_kind_)) == 1 &&
        std::rand() % 100 > 0) {
        ++key_count_;
    }
    _MakeNotiPacketDieMonster(packet,
                              dropped,
                              data.monster_unique_id,
                              data.monster_type,
                              live_monster,
                              killed_monster);

    G_CDataManager()->get_mob_reward(killed_monster.monster_level_, gained_exp);

    // OFFICIAL_0627_IDA: CBattle_Field::kill_monster
    // 0x830D1E4..0x830D24B advances the dungeon-wide boss-room entrance
    // status before erasing the killed monster.  Shadow Maze imports
    // [boss room entrance condition] = [hunt monster], 50097, 1, 1; the
    // condition monster is injected only into the selected maze room, and
    // killing it must therefore turn progress 0/1 into 1/1.  Keeping this
    // update in the authoritative kill transaction (rather than a passive
    // object or client-notification path) also preserves the original
    // ordering observed by check_boss_map().
    if (dungeon_ && dungeon_->enter_boss_room_condition_.enabled == 1) {
        for (std::vector<stConditionEnterBossRoomStatus_t>::iterator status =
                 boss_room_conditions_.begin();
             status != boss_room_conditions_.end();
             ++status) {
            if (status->condition_type != killed_monster.monster_id_) {
                continue;
            }
            ++status->boss_room_id;
            LogManager::logFormat(
                1,
                "battle_field.cpp",
                "bool CBattle_Field::kill_monster(PacketGuard&, map_monster&, uint&, const STKillMonsterData&)",
                0x1d24,
                "[BOSS_ROOM_CONDITION][KILL_PROGRESS] dungeon=%d map=%d template=%d progress=%d required=%d",
                dungeon_index_,
                map->map_index_,
                killed_monster.monster_id_,
                status->boss_room_id,
                status->status);
        }
    }

    // IDA 0x830D16C..0x830D1A9: the Elevator Dungeon's named controller
    // changes the countdown state after its death. This is deliberately
    // emitted before the live monster is erased, matching the original
    // independent CMD253 broadcast ordering.
    if (live_monster.monster_id_ == 56716) {
        elevator_timer_state_ =
            static_cast<int8_t>(elevator_timer_count_) > 3 ? 2 : 1;
        send_elevator_time_packet(
            static_cast<char>(elevator_timer_state_));
    }

    map->monsters_.erase(found);
    // 0627 official CBattle_Field::kill_monster @0x830D279..0x830D2A9:
    // every blood-dungeon erase immediately feeds the remaining live count
    // back into the round controller.  Ultimate Altar (blood type 2) relies
    // on this callback to close the current small round and enqueue timer 38;
    // without it the first monster disappears but no later wave can start.
    if (dungeon_ && dungeon_->get_blood_dungeon_type() > 0) {
        checkKilledSpawnBloodMonsters(
            static_cast<int>(map->monsters_.size()));
    }
    LogManager::logFormat(
        1,
        "dungeon42_diag.cpp",
        "bool CBattle_Field::kill_monster(PacketGuard&, map_monster&, uint&, const STKillMonsterData&)",
        0,
        "[DUNGEON42][ERASED] monster_uid=%u monster_index=%d exp_base=%u no_reward=%u complete=%u remaining=%u grid_clear=%d",
        static_cast<unsigned int>(data.monster_unique_id),
        killed_monster.monster_id_,
        gained_exp,
        static_cast<unsigned int>(data.suppress_random_drop),
        static_cast<unsigned int>(data.mark_monster_completed),
        static_cast<unsigned int>(map->monsters_.size()),
        check_grid_clear());
    return 1;
}
