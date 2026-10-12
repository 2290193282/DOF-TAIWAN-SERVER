#include "old_game/pvp/battlefield/war_room.h"

#include <cstring>
#include <list>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_data/PVF/data/item_generator_types.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

void AppendWarDrop(PacketGuard& packet, const map_item& item) {
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
            static_cast<unsigned char>(item.item.avatar_select_)));
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
        stAvatarExpansionInfo_t expansion;
        expansion.init();
        packet.put_int(4);
        packet.put_binary(reinterpret_cast<char*>(&expansion), 4);
    } else {
        packet.put_byte(0);
    }
    packet.put_short(item.monster_type_);
}

void GenerateWarDrops(const map_monster& monster,
                      CUser* owner,
                      const CDungeon& dungeon,
                      std::list<map_item>& dropped,
                      MapInfo& map) {
    WongWork::CItemGeneratorMgr* const generators =
        G_CDataManager()->getItemGenerator();
    if (!generators) return;

    int money_ratio = 0;
    if (owner) {
        const int money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
            owner->get_charac_level(), owner->get_acc_name());
        if (money_limit > 0) {
            money_ratio = 100 * owner->getCurCharacMoney() / money_limit;
        }
    }

    WongWork::stGenerateRefData_t reference;
    reference.standard_drop_rate = 100;
    reference.monster_random_drop_scale = 1.0f;
    WongWork::stGenerateResult_t generated;
    WongWork::CItemGenerator* const monster_generator =
        generators->getGenerator(WongWork::CItemGeneratorMgr::kMonsterDrop);
    if (monster_generator) {
        std::memset(&reference.generator_data, 0,
                    sizeof(reference.generator_data));
        reference.generator_data.monster_random_drop.character_level =
            monster.monster_level_;
        reference.generator_data.monster_random_drop.monster_role =
            monster.monster_kind_;
        reference.generator_data.monster_random_drop.dungeon_standard_level =
            static_cast<uint8_t>(dungeon.get_standard_level());
        reference.generator_data.monster_random_drop.dungeon_difficulty = 1;
        reference.generator_data.monster_random_drop.party_member_count = 1;
        reference.generator_data.monster_random_drop.monster_drop_count = 1;
        reference.generator_data.monster_random_drop.bourgeois_rate = money_ratio;
        monster_generator->generateRandomItem(reference, generated);

        std::memset(&reference.generator_data, 0,
                    sizeof(reference.generator_data));
        reference.generator_data.monster_specific_drop.character_level =
            monster.monster_level_;
        reference.generator_data.monster_specific_drop.dungeon_standard_level =
            static_cast<uint8_t>(dungeon.get_standard_level());
        reference.generator_data.monster_specific_drop.dungeon_difficulty = 1;
        reference.generator_data.monster_specific_drop.retained_dungeon_difficulty = 1;
        reference.generator_data.monster_specific_drop.monster_id =
            static_cast<uint32_t>(monster.monster_id_);
        reference.generator_data.monster_specific_drop.monster_kind =
            monster.monster_kind_;
        reference.generator_data.monster_specific_drop.monster_drop_count = 1;
        reference.generator_data.monster_specific_drop.party_member_max_level = 1;
        monster_generator->generateSpecificItem(reference, generated);
    }

    WongWork::CItemGenerator* const dungeon_generator =
        generators->getGenerator(WongWork::CItemGeneratorMgr::kDungeonDrop);
    if (dungeon_generator) {
        std::memset(&reference.generator_data, 0,
                    sizeof(reference.generator_data));
        reference.generator_data.dungeon_drop.items =
            const_cast<std::vector<std::pair<int, int> >*>(
                &dungeon.getDropItems());
        dungeon_generator->generateSpecificItem(reference, generated);
    }
    WongWork::CItemGenerator* const world_generator =
        generators->getGenerator(WongWork::CItemGeneratorMgr::kWorldDrop);
    if (world_generator) {
        std::memset(&reference.generator_data, 0,
                    sizeof(reference.generator_data));
        reference.generator_data.world_drop.world_area = 1;
        reference.generator_data.world_drop.world_index = monster.monster_level_;
        world_generator->generateRandomItem(reference, generated);
    }

    const uint16_t owner_uid = owner ? owner->get_unique_id() : 0xffffu;
    for (std::vector<Inven_Item>::const_iterator it = generated.items.begin();
         it != generated.items.end();
         ++it) {
        map_item item;
        item.routing_eligible_ = 1u;
        item.unique_id_ = static_cast<uint32_t>(
            map.next_drop_item_unique_id_++);
        // WarField uses this physical short as the pickup owner and re-emits
        // it in NOTI38's trailing item field.
        item.monster_type_ = owner_uid;
        item.created_at_sec_ = static_cast<uint32_t>(
            GlobalData::s_systemTime_.getCurSec());
        item.item = *it;
        map.Add_Item(item);
        dropped.push_back(item);
    }
}

}  // namespace

// IDA_EA: 0x86B954C
// IDA_NAME: _ZNK8WarField11IsGridClearEv
bool WarField::IsGridClear() const {
    return map_info_.monsters_.empty();
}

// IDA_EA: 0x86B9672
// IDA_NAME: _ZN8WarField17HandleMonsterKillEiR11PacketGuardR11map_monsterP5CUserPK8CDungeon
int WarField::HandleMonsterKill(int monster_unique_id,
                                PacketGuard& packet,
                                map_monster& monster_out,
                                CUser* drop_owner,
                                const CDungeon* dungeon) {
    if (!dungeon) return 0;
    std::map<int, map_monster>::iterator it =
        map_info_.monsters_.find(monster_unique_id);
    if (it == map_info_.monsters_.end() ||
        !G_CDataManager()->find_monster(it->second.monster_id_)) {
        return 0;
    }

    monster_out = it->second;
    std::list<map_item> dropped;
    GenerateWarDrops(monster_out, drop_owner, *dungeon, dropped, map_info_);

    packet.put_header(0, ENUM_NOTIPACKET_DIE_MONSTER);
    packet.put_short(monster_unique_id);
    packet.put_byte(static_cast<int>(dropped.size()));
    for (std::list<map_item>::const_iterator drop = dropped.begin();
         drop != dropped.end();
         ++drop) {
        AppendWarDrop(packet, *drop);
    }
    map_info_.monsters_.erase(it);
    return 1;
}

// IDA_EA: 0x86BE1FE
// IDA_NAME: _ZN7WarRoom17HandleMonsterKillEP5CUseriS1_
// DEMANGLED: WarRoom::HandleMonsterKill(CUser *,int,CUser *)
int WarRoom::HandleMonsterKill(CUser* user,
                               int monster_unique_id,
                               CUser* target) {
    if (!GetWaiterCount() || !war_field_.dungeon_) {
        return 0;
    }

    map_monster snapshot;
    int wp = 0;
    for (int seat = 0; seat <= 5; ++seat) {
        if (target && war_field_.room_users_[seat] == target) {
            wp = war_field_.HandleWpPerMonster(target, monster_unique_id, 0,
                                                snapshot);
        }
    }
    if (snapshot.monster_id_ == 0) {
        std::map<int, map_monster>::const_iterator it =
            war_field_.map_info_.monsters_.find(monster_unique_id);
        if (it == war_field_.map_info_.monsters_.end()) return 0;
        snapshot = it->second;
    }

    PacketGuard packet;
    if (!war_field_.HandleMonsterKill(monster_unique_id, packet, snapshot,
                                      target, war_field_.dungeon_)) {
        return 0;
    }
    if (snapshot.monster_kind_ >= 0 && snapshot.monster_kind_ < 4) {
        war_field_.clear_reward_multipliers_[snapshot.monster_kind_] +=
            snapshot.war_room_scale_b_;
    }

    const int dungeon_index = war_field_.dungeon_->get_index();
    const int monster_kind = snapshot.monster_kind_;
    const int monster_exp = monster_kind >= 0 && monster_kind < 4
        ? static_cast<int>(snapshot.war_room_scale_a_ *
            war_field_.dungeon_->war_room_data_.extra_group_values_[monster_kind])
        : 0;
    for (int seat = 0; seat <= 5; ++seat) {
        CUser* const member = war_field_.room_users_[seat];
        if (!member) continue;
        const bool is_target = target && member == target;
        if (is_target) {
            member->WarAreaKill(monster_kind, wp, snapshot.war_room_scale_a_);
            member->CheckQuestMonster(dungeon_index, snapshot.monster_id_,
                                      static_cast<ENUM_QUEST_ENEMY_TYPE>(1));
        }
        ExpBonusPointResult bonus;
        if (is_target) {
            member->checkBonusPoint();
            member->getExpBonusPoint(bonus);
        }
        int base_exp = monster_exp + bonus.normal;
        base_exp += static_cast<int>(static_cast<long double>(base_exp) *
                                     bonus.premium / 100.0L);
        int fatigue_bonus_exp = 0;
        if (member->getCurCharacFatigueGrownUpBuff() != 0) {
            // Raw WarRoom::HandleMonsterKill awards a second copy of this
            // per-monster base and preserves it in the character's EXP
            // breakdown before gain_exp_sp serializes the notification.
            fatigue_bonus_exp = base_exp;
            member->set_charac_fatigue_buf_bonus_exp(fatigue_bonus_exp);
        }
        int total_exp = base_exp + member->gainExpAsUpperMember(base_exp) +
            member->gainGuildSkillExp(base_exp) +
            fatigue_bonus_exp +
            member->gainPowerWarRewardExp(0);
        int gained_sp = 0;
        int gained_sfp = 0;
        if (member->gain_exp_sp(total_exp, gained_sp, gained_sfp,
                                static_cast<eExpAddReason>(4),
                                snapshot.monster_id_, false)) {
            member->SendNotiPacket(static_cast<CUser::eSendTarget>(3),
                                   static_cast<ENUM_NOTIPACKET>(2), 1);
        }
    }

    packet.put_byte(1);
    packet.put_short(target ? target->get_unique_id() : 0xffff);
    packet.put_short(wp);
    packet.finalize(true);
    SendToRoom(packet);

    if (war_field_.IsGridClear()) {
        for (int seat = 0; seat <= 5; ++seat) {
            CUser* const member = war_field_.room_users_[seat];
            if (!member) continue;

            const int pending_creature_exp = member->takePendingCreatureExp();
            if (pending_creature_exp <= 0) continue;

            const user_creature::CCreatureMgr* const read_creatures =
                member->getCurCharacInvenR()->GetCreatureMgrR();
            if (read_creatures &&
                !read_creatures->IsGrowCreature_Equipped_Creature()) {
                user_creature::CCreatureMgr* const write_creatures =
                    member->getCurCharacInvenW()->GetCreatureMgrW();
                if (write_creatures) {
                    write_creatures->GainExp(pending_creature_exp);
                }
            }
        }
        GenTimerKey(static_cast<TIMER_MESSAGE>(69));
        if (current_spawn_step_ ==
            war_field_.dungeon_->war_room_data_.timing_values_[0]) {
            battle_finished_ = 1;
            ChangeState(static_cast<WARROOM_STATE>(5));
        } else {
            ChangeState(static_cast<WARROOM_STATE>(4));
        }
    }
    return 1;
}
