#include "old_game/pvp/battlefield/battle_field.h"

#include <cstdint>
#include <list>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace WongWork {

namespace {

// IDA 0x846483E: the fast-kill counter applies to every character above 29,
// and to level 25..29 characters only while one of the active 23..27 premium
// records is present.  Querying selector 9999 is the reconstructed provider's
// all-premium sentinel (the same selector is used by the user cleanup path).
bool IsDeathTowerFastKillHackEligible(CUser* user) {
    if (!user) return false;

    const int charac_level = user->get_charac_level();
    if (charac_level > 29) return true;
    if (charac_level <= 24) return false;

    const std::vector<SUserPremiumInfo> premiums =
        user->GetPremiumInfo()->GetPremiumInfoList(9999);
    for (std::vector<SUserPremiumInfo>::const_iterator it = premiums.begin();
         it != premiums.end();
         ++it) {
        if (it->premium_type > 22 && it->premium_type <= 27 &&
            it->state == 1) {
            return true;
        }
    }
    return false;
}

void AppendDrop(PacketGuard& packet, const map_item& item) {
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
    if (item.item.isAvatarItemType()) {
        packet.put_byte(1);
        packet.put_int(30);
        packet.put_binary(reinterpret_cast<char*>(emblem.sockets),
                          sizeof(emblem.sockets));
    } else {
        packet.put_byte(0);
    }
    packet.put_short(item.monster_type_);
}

void SendDeathTowerMonsterDie(CParty& party,
                              unsigned short monster_uid,
                              const std::list<map_item>& drops) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_DIE_MONSTER);
    packet.put_short(monster_uid);
    int drop_count_index = packet.get_index();
    packet.put_byte(0);

    int drop_count = 0;
    for (std::list<map_item>::const_iterator it = drops.begin();
         it != drops.end();
         ++it) {
        AppendDrop(packet, *it);
        ++drop_count;
    }
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(drop_count_index, drop_count);
    packet.finalize(true);
    party.send_to_party(packet);
}

}  // namespace

// IDA_EA: 0x84616A8
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage8dropItemER8map_item
int CDeathTower::CStage::dropItem(map_item& item) {
    item.unique_id_ = static_cast<uint32_t>(map_info_.next_drop_item_unique_id_++);
    map_info_.Add_Item(item);
    return static_cast<int>(item.unique_id_);
}

// IDA_EA: 0x8461708
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage11killMonsterEiR11map_monster
int CDeathTower::CStage::killMonster(int monster_unique_id,
                                     map_monster& monster) {
    std::map<int, map_monster>::iterator found =
        map_info_.monsters_.find(monster_unique_id);
    if (found == map_info_.monsters_.end()) {
        return 0;
    }
    monster = found->second;
    const int monster_level = static_cast<unsigned char>(
        found->second.monster_level_);
    map_info_.monsters_.erase(found);
    return monster_level;
}

// IDA_EA: 0x846483E
// IDA_NAME: _ZN8WongWork11CDeathTower17handleKillMonsterEP5CUserttRK15MSG_MONSTER_DIEPi
int CDeathTower::handleKillMonster(CUser* user,
                                   unsigned short monster_id,
                                   unsigned short monster_type,
                                   const MSG_MONSTER_DIE& message,
                                   int* result) {
    // The state gate is part of the original CMD42 protocol.  A stale client
    // report must receive the tower-specific error rather than mutate stage
    // state or fall through into ordinary dungeon handling.
    if (stage_.getState() != 1) {
        if (user) user->SendCmdErrorPacket(ENUM_CMDPACKET_DIE_MONSTER, 19);
        return 0;
    }

    if (!party_) return 0;

    // IDA 0x846483E only applies the short-window counter when no tower
    // consumable was used.  Its level and active-premium eligibility split is
    // intentionally evaluated before resetting the same counter state.
    if (!map_monster_kill_checker_.getUseSkillMaterialTime()) {
        const uint32_t now = static_cast<uint32_t>(
            GlobalData::s_systemTime_.getCurTickCount());
        if (map_monster_kill_checker_.dieMonster(now, 1000u) > 2 &&
            IsDeathTowerFastKillHackEligible(user)) {
            map_monster_kill_checker_.initMonsterInfo();
            user->getHackAnalyzer()->addServerHackCnt(
                user, static_cast<ENUM_HACKTYPE>(0xd6), 1u, 0u, 0u);
        }
    }

    map_monster monster;
    const int killed_type = stage_.killMonster(
        static_cast<int>(monster_id), monster);
    if (killed_type == 0) {
        return 0;
    }
    if (result) {
        *result = monster.monster_id_;
    }

    CDungeon* const dungeon = dungeon_manager_.getDungeon();
    if (!dungeon) return 0;

    std::vector<Inven_Item> generated_items;
    std::vector<unsigned long> specified_items;
    std::list<map_item> dropped_items;
    int pvp_win_point = 0;

    if (static_cast<signed char>(monster.monster_kind_) <= 4) {
        CMonster* const monster_data =
            G_CDataManager()->find_monster(monster.monster_id_);
        if (monster_data && dungeon->hasTowerItemDrop()) {
            const int specified_item = monster_data->deathTowerSpecifyItemDrop();
            if (specified_item != -2) {
                specified_items.push_back(
                    static_cast<unsigned long>(specified_item));
            }
        }
    } else {
        CAICharacter* const ai_character =
            G_CDataManager()->get_ai_character_list()->get(
                static_cast<uint32_t>(monster.monster_id_));
        if (ai_character) {
            if (dungeon->hasTowerItemDrop()) {
                ai_character->deathTowerSpecifyItemDrop(specified_items);
            }
            if (monster.assigned_team_ == 100 ||
                monster.assigned_team_ == 200) {
                pvp_win_point = ai_character->getPvPWinPoint();
            }
        }
    }

    // Original semantics create one configured tower drop per party member.
    for (std::vector<unsigned long>::const_iterator it = specified_items.begin();
         it != specified_items.end();
         ++it) {
        if (!G_CDataManager()->find_item(static_cast<int>(*it))) continue;
        Inven_Item item;
        item.item_id_ = static_cast<int>(*it);
        G_CDataManager()->get_item_list()->create_item(
            ENUM_ITEM_CREATE_TYPE_DEFAULT, item, 1);
        for (int member = 0; member < party_->get_member_count(); ++member) {
            generated_items.push_back(item);
        }
    }

    const int member_level_gap = party_->getMemberLevelGap();
    stDeathTower_t* const tower_script =
        G_CDataManager()->GetTowerScript(
            static_cast<unsigned int>(dungeon->get_index()));
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = party_->get_user(seat);
        if (!member || party_->checkValidUser(seat) != 1 ||
            party_->battle_field().instance_dungeon_mode() == 1) {
            continue;
        }

        unsigned int gained_exp = 0;
        G_CDataManager()->get_mob_reward(member->get_charac_level(), gained_exp);
        gained_exp = static_cast<unsigned int>(party_->getMonsterTotalExp(
            gained_exp, 0, 0, 1.0f, 1.0f, false));
        QuickParty::RewardData_Param reward_parameter(party_);
        QuickParty::RewardData_Result reward_result;
        reward_result.raw_00 = static_cast<int>(gained_exp);
        party_->set_random_reward(static_cast<QuickParty::RandomBuffType>(0),
                                  reward_parameter, reward_result);
        gained_exp = static_cast<unsigned int>(reward_result.raw_00);
        if (tower_script) {
            if (monster.monster_kind_ == 5) {
                gained_exp = static_cast<unsigned int>(
                    static_cast<int>(tower_script->apc_exp_weight * gained_exp));
            }
            gained_exp = static_cast<unsigned int>(
                static_cast<int>(tower_script->monster_exp_weight * gained_exp));
        }

        STGainedEachExpData gained_each_exp;
        gained_each_exp.gained_exp = gained_exp;
        gained_each_exp.member_level_gap = member_level_gap;
        gained_exp = static_cast<unsigned int>(
            party_->sumGainedEachExp(member, gained_each_exp));

        if (pvp_win_point && GlobalData::s_event_manager) {
            CDeathTowerWinPointEvent* const event =
                static_cast<CDeathTowerWinPointEvent*>(
                    GlobalData::s_event_manager->GetRepeatEvent(39));
            const long double rate = event ? event->getBonusRate() : 0.0L;
            member->gainWinPoint(rate == 0.0L ? pvp_win_point :
                                 static_cast<int>(pvp_win_point * rate),
                                 static_cast<eWPAddReason>(2));
        }

        member->calcHelpAbuseRatio(gained_exp, false);
        int gained_sp = 0;
        int gained_sfp = 0;
        const eExpAddReason reason = static_cast<eExpAddReason>(
            monster.monster_kind_ == 5 ? 12 : 11);
        if (member->gain_exp_sp(static_cast<int>(gained_exp), gained_sp,
                                gained_sfp, reason, monster.monster_id_,
                                false)) {
            member->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                                   static_cast<ENUM_NOTIPACKET>(2), 1);
        }
        member->history_log().KillMonster(monster.monster_id_, message.field_a1e_,
                                          message.field_a1a_, message.field_a24_,
                                          message.field_a26_, message,
                                          static_cast<char>(monster.monster_kind_),
                                          monster.monster_kind_ == 5,
                                          member->getCurCharacExp(),
                                          static_cast<int>(gained_exp), reason);
        ENUM_QUEST_ENEMY_TYPE enemy_type =
            static_cast<ENUM_QUEST_ENEMY_TYPE>(
                monster.monster_kind_ > 4 && monster.monster_kind_ <= 8 ? 2 : 1);
        member->CheckQuestMonster(dungeon->get_index(), monster.monster_id_,
                                  enemy_type);
    }

    for (std::vector<Inven_Item>::const_iterator it = generated_items.begin();
         it != generated_items.end();
         ++it) {
        map_item item;
        item.routing_eligible_ = 1u;
        item.monster_type_ = monster_type;
        item.created_at_sec_ = static_cast<uint32_t>(
            GlobalData::s_systemTime_.getCurSec());
        item.item = *it;
        stage_.dropItem(item);
        dropped_items.push_back(item);
    }
    SendDeathTowerMonsterDie(*party_, monster_id, dropped_items);

    if (stage_.checkClearStage()) {
        _onFinishStage();
        if (stage_.getCurrentStage() == dungeon_manager_.getEndStage()) {
            _onPrepareFinishTower();
        }
    }
    return 1;
}

}  // namespace WongWork
