#include "old_game/pvp/battlefield/battle_field.h"

#include <cstdint>
#include <list>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "old_data/PVF/data/item_generator_types.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace WongWork {

// IDA_EA: 0x814D300
// IDA_NAME: _ZN8WongWork10CBossStage17KillMonsterGetExpERK11map_monsterRj
// DEMANGLED: WongWork::CBossStage::KillMonsterGetExp(map_monster const&,
//             unsigned int &)
int CBossStage::KillMonsterGetExp(const map_monster& monster,
                                  unsigned int& gained_exp) {
    const int member_level_gap = party_->getMemberLevelGap();
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (!user ||
            static_cast<unsigned char>(
                party_->checkValidUser(member_slot)) != 1u ||
            party_->battle_field().instance_dungeon_mode() == 1) {
            continue;
        }

        G_CDataManager()->get_mob_reward(
            user->get_charac_level(), gained_exp);
        gained_exp = static_cast<unsigned int>(
            party_->getMonsterTotalExp(
                gained_exp,
                static_cast<char>(monster.monster_kind_),
                static_cast<int>(stage_value_aec_),
                1.0f,
                1.0f,
                false));

        stDeathTower_t* const tower_script =
            G_CDataManager()->GetTowerScript(
                static_cast<unsigned int>(dungeon_->get_index()));
        if (monster.monster_kind_ == 5u) {
            gained_exp = static_cast<unsigned int>(
                static_cast<int>(
                    tower_script->apc_exp_weight *
                    static_cast<float>(gained_exp)));
        }
        gained_exp = static_cast<unsigned int>(
            static_cast<int>(
                tower_script->monster_exp_weight *
                static_cast<float>(gained_exp)));

        STGainedEachExpData gained_each_exp;
        gained_each_exp.gained_exp = gained_exp;
        gained_each_exp.member_level_gap = member_level_gap;
        gained_exp = static_cast<unsigned int>(
            party_->sumGainedEachExp(user, gained_each_exp));
        user->calcHelpAbuseRatio(gained_exp, false);

        int first_result = 0;
        int second_result = 0;
        const eExpAddReason reason = static_cast<eExpAddReason>(
            monster.monster_kind_ == 5 ? 12 : 11);
        if (user->gain_exp_sp(
                static_cast<int>(gained_exp),
                first_result,
                second_result,
                reason,
                monster.monster_id_,
                false)) {
            user->SendNotiPacket(
                static_cast<CUser::eSendTarget>(2),
                static_cast<ENUM_NOTIPACKET>(2),
                1);
        }

        ENUM_QUEST_ENEMY_TYPE enemy_type =
            static_cast<ENUM_QUEST_ENEMY_TYPE>(1);
        const signed char signed_monster_kind =
            static_cast<signed char>(monster.monster_kind_);
        if (signed_monster_kind > 4 && signed_monster_kind <= 8) {
            enemy_type = static_cast<ENUM_QUEST_ENEMY_TYPE>(2);
        }
        user->CheckQuestMonster(dungeon_->get_index(),
                                monster.monster_id_,
                                enemy_type);
    }
    return 1;
}

// IDA_EA: 0x814D628
// IDA_NAME: _ZN8WongWork10CBossStage19killMonsterDropItemERK11map_monsteritt
// DEMANGLED: WongWork::CBossStage::killMonsterDropItem(map_monster const&,
//             int,unsigned short,unsigned short)
int CBossStage::killMonsterDropItem(const map_monster& monster,
                                    int monster_kind,
                                    unsigned short monster_id,
                                    unsigned short monster_type) {
    std::vector<unsigned long> tower_specific_items;
    std::list<map_item> dropped_items;
    stGenerateResult_t generated;
    tower_specific_items.clear();
    dropped_items.clear();
    generated.items.clear();

    if (static_cast<signed char>(monster.monster_kind_) <= 4) {
        CMonster* const monster_data =
            G_CDataManager()->find_monster(monster.monster_id_);
        if (monster_data) {
            stGenerateRefData_t reference;
            reference.generator_data.boss_monster_drop.monster_kind =
                static_cast<std::uint8_t>(monster_kind);
            reference.generator_data.boss_monster_drop.member_count_class =
                static_cast<std::uint8_t>(member_count_class_);
            reference.generator_data.boss_monster_drop.party_member_count =
                static_cast<std::uint8_t>(party_->get_member_count());
            monster_data->generateIndependentItem(
                reference, generated.items);
        }
    } else {
        CAICharacterList* const ai_character_list =
            G_CDataManager()->get_ai_character_list();
        CAICharacter* const ai_character =
            ai_character_list->get(
                static_cast<std::uint32_t>(monster.monster_id_));
        if (ai_character) {
            if (static_cast<unsigned char>(
                    dungeon_->hasTowerItemDrop()) != 0u) {
                ai_character->deathTowerSpecifyItemDrop(
                    tower_specific_items);
            }
            stGenerateRefData_t reference;
            reference.generator_data.boss_monster_drop.monster_kind =
                static_cast<std::uint8_t>(monster_kind);
            reference.generator_data.boss_monster_drop.member_count_class =
                static_cast<std::uint8_t>(member_count_class_);
            reference.generator_data.boss_monster_drop.party_member_count =
                static_cast<std::uint8_t>(party_->get_member_count());
            ai_character->generateIndependentItem(
                reference, generated.items);
        }
    }

    for (std::vector<Inven_Item>::const_iterator item =
             generated.items.begin();
         item != generated.items.end();
         ++item) {
        map_item dropped_item;
        dropped_item.routing_eligible_ = 1u;
        dropped_item.monster_type_ = monster_type;
        dropped_item.created_at_sec_ =
            static_cast<std::uint32_t>(
                GlobalData::s_systemTime_.getCurSec());
        dropped_item.item = *item;
        dropped_item.unique_id_ = static_cast<std::uint32_t>(
            map_info_.next_drop_item_unique_id_++);
        map_info_.Add_Item(dropped_item);
        dropped_items.push_back(dropped_item);
    }

    int item_count = 0;
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_DIE_MONSTER);
    packet.put_short(monster_id);
    int item_count_index = packet.get_index();
    packet.put_byte(item_count);

    stAvatarEmblemInfo_t emblem;
    emblem.init();
    for (std::list<map_item>::const_iterator item =
             dropped_items.begin();
         item != dropped_items.end();
         ++item) {
        packet.put_short(static_cast<int>(item->unique_id_));
        packet.put_int(item->item.getKey());
        packet.put_byte(item->item.GetItemAttr());

        CEquipItem* avatar_item = 0;
        if (item->item.isAvatarItemType()) {
            avatar_item = static_cast<CEquipItem*>(
                G_CDataManager()->find_item(
                    item->item.getKey()));
        }
        if (item->item.isAvatarItemType() && avatar_item) {
            const int avatar_period = avatar_item->getAvatarPeriod(
                static_cast<unsigned char>(
                    item->item.avatar_select_));
            packet.put_int(86400 * avatar_period);
            emblem.init();
            avatar_item->getAvatarSocket(
                item->item.avatar_select_, emblem);
        } else {
            packet.put_int(item->item.get_add_info());
        }

        packet.put_short(item->item.avatar_select_);
        packet.put_byte(
            item->item.amplify_option_.getAbilityType());
        packet.put_short(
            item->item.amplify_option_.getAbilityValue());
        packet.put_packet(item->item);
        if (item->item.isAvatarItemType()) {
            packet.put_byte(1);
            packet.put_int(30);
            packet.put_binary(reinterpret_cast<char*>(emblem.sockets),
                              sizeof(emblem.sockets));
        } else {
            packet.put_byte(0);
        }
        packet.put_short(item->monster_type_);
        ++item_count;
    }

    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(item_count_index, item_count);
    packet.finalize(true);
    party_->send_to_party(packet);
    return 1;
}

// IDA_EA: 0x8150388
// IDA_NAME: _ZN8WongWork10CBossStage17handleKillMonsterEP5CUserttRK15MSG_MONSTER_DIEPi
// DEMANGLED: WongWork::CBossStage::handleKillMonster(CUser *,unsigned short,
//             unsigned short,MSG_MONSTER_DIE const&,int *)
int CBossStage::handleKillMonster(CUser* user,
                                  unsigned short monster_id,
                                  unsigned short monster_type,
                                  const MSG_MONSTER_DIE& message,
                                  int* result) {
    (void)result;

    map_monster monster;
    int monster_kind = 0;
    if (static_cast<unsigned char>(
            killMonsterErase(monster_id, monster, monster_kind)) != 1u) {
        return 0;
    }

    if (party_->battle_field().instance_dungeon_mode() == 1) {
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_DIE_MONSTER);
        packet.put_short(monster_id);
        packet.put_byte(0);
        packet.put_byte(0);
        packet.put_byte(0);
        packet.put_byte(0);
        packet.finalize(true);
        party_->send_to_party(packet);
        return 1;
    }

    unsigned int gained_exp = 0;
    if (static_cast<unsigned char>(
            KillMonsterGetExp(monster, gained_exp)) != 1u) {
        return 0;
    }

    user->history_log().KillMonster(
        monster.monster_id_,
        message.field_a1e_,
        message.field_a1a_,
        message.field_a24_,
        message.field_a26_,
        message,
        static_cast<char>(monster.monster_kind_),
        monster.monster_kind_ == 5,
        user->getCurCharacExp(),
        static_cast<int>(gained_exp),
        static_cast<eExpAddReason>(11));

    return static_cast<unsigned char>(
               killMonsterDropItem(monster,
                                   monster_kind,
                                   monster_id,
                                   monster_type)) == 1u;
}

}  // namespace WongWork
