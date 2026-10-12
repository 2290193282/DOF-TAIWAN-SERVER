#include "old_game/social/party/party.h"
#include "common/config.h"

#include <algorithm>
#include <set>
#include <vector>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_effect_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/channel/channel_script_t.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/item_generator_drop_types.h"
#include "old_data/PVF/dungeon/maze_script.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_game/dungeon/instance/luck_point_runtime.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/activity/schedule/character_event_records.h"
#include "old_game/activity/condition/condition_event_manager_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/activity/schedule/king_of_pick.h"
#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/character_manage_script.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/log_manager.h"
#include "infra/network/packet/packet_guard.h"

namespace {

void ResetRewardPair(Inven_Item& first) {
    first.reset();
    (&first)[1].reset();
}

int CountPlayableMazeCells(const MazeScript* maze) {
    if (!maze) return 0;

    int count = 0;
    for (int x = 0; x < maze->grid_width; ++x) {
        for (int y = 0; y < maze->grid_height; ++y) {
            if (maze->getGridR(x, y).grid_type != 0) {
                ++count;
            }
        }
    }
    return count;
}

int GetMinimumStandardDropRate(CParty* party) {
    int rate = 100;
    ServiceRestrictManager* const restrictions =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (!restrictions ||
        restrictions->isRestricted(RestrictCategory::Security, 1) == 0) {
        return rate;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(party->checkValidUser(seat)) != 1u) {
            continue;
        }
        rate = std::min(rate, party->get_user(seat)->getStdDropRate());
    }
    return rate;
}

bool IsStandardDropRateRestricted() {
    ServiceRestrictManager* const restrictions =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    return restrictions &&
           restrictions->isRestricted(RestrictCategory::Security, 1) != 0;
}

int ResolveClearRewardItemLevel(const CDungeon* dungeon, int standard_level) {
    // Risk dungeons use level 100 monsters, but the 0828 equipment pool ends
    // at the character cap.  A level-100 card searches 94..106 and finds no
    // ordinary equipment.  Normalize only the item reference, never EXP/gold.
    return dungeon && dungeon->isRiskDungeon()
        ? std::min(standard_level, static_cast<int>(oldsrv::config::kCharacterLevelMaximum))
        : standard_level;
}

void PrepareClearRewardReference(WongWork::stGenerateRefData_t& reference,
                                 const CDungeon* dungeon,
                                 int standard_level,
                                 int dungeon_difficulty,
                                 int member_count,
                                 int standard_drop_rate,
                                 float item_probability_scale) {
    reference.standard_drop_rate = standard_drop_rate;
    // The top-level +0x04 field is the per-member death penalty.  The map
    // completion/item probability belongs to the clear-reward union at
    // +0x38; the two fields are consumed separately by _generateRandomItem.
    reference.clear_reward_scale = 0.0f;
    reference.generator_data.clear_reward.character_level_or_world_area =
        static_cast<uint8_t>(ResolveClearRewardItemLevel(dungeon, standard_level));
    reference.generator_data.clear_reward.dungeon_difficulty_or_world_index =
        static_cast<uint8_t>(dungeon_difficulty);
    reference.generator_data.clear_reward.party_member_count =
        static_cast<uint8_t>(member_count);
    reference.generator_data.clear_reward.item_probability_scale =
        item_probability_scale;
    reference.generator_data.clear_reward.clear_reward_drop_probability = 0;
    reference.generator_data.clear_reward.dungeon =
        const_cast<CDungeon*>(dungeon);
}

void PrepareGoldRewardReference(WongWork::stGenerateRefData_t& reference,
                                const BattleData& battle_data,
                                int standard_level,
                                int dungeon_difficulty,
                                int member_count,
                                int rank_bonus_index) {
    reference.generator_data.gold_reward.character_level =
        static_cast<uint8_t>(standard_level);
    reference.generator_data.gold_reward.dungeon_difficulty =
        static_cast<uint8_t>(dungeon_difficulty);
    reference.generator_data.gold_reward.party_member_count =
        static_cast<uint8_t>(member_count);
    reference.generator_data.gold_reward.exp_bonus_context = rank_bonus_index;
    reference.generator_data.gold_reward.gold_weight_08 =
        battle_data.clear_reward_gold_weight_08();
    reference.generator_data.gold_reward.gold_weight_0a =
        battle_data.clear_reward_gold_weight_0a();
    reference.generator_data.gold_reward.gold_reserved_0c = 0;
    reference.generator_data.gold_reward.gold_weight_0e =
        battle_data.clear_reward_gold_weight_0e();
}

void CopyNextGeneratedItem(const WongWork::stGenerateResult_t& result,
                           std::vector<Inven_Item>::const_iterator& it,
                           Inven_Item& destination) {
    destination.reset();
    if (it == result.items.end()) return;
    destination.setCopy(*it);
    ++it;
}

float ToFloat(long double value) {
    return static_cast<float>(value);
}

unsigned int AppendQuestClearItems(CUser* user,
                                   int dungeon_index,
                                   PacketGuard& packet) {
    UserQuest* const quests = user->getCurCharacQuestRefR();
    if (!quests) return 0;

    unsigned int inserted_count = 0;
    for (int quest_slot = 0; quest_slot <= 19; ++quest_slot) {
        if (!quests->play_quest[quest_slot] ||
            !quests->play_quest_trigger[quest_slot]) {
            continue;
        }

        Quest* const quest = G_CDataManager()->find_quest(
            static_cast<int>(quests->play_quest[quest_slot]));
        if (!quest || !quest->check_clear_item(
                          dungeon_index,
                          static_cast<int>(quests->authen_data.clear_count))) {
            continue;
        }

        const std::vector<std::pair<int, int> > clear_items =
            quest->get_clear_itemlist(dungeon_index);
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 clear_items.begin();
             it != clear_items.end(); ++it) {
            Inven_Item item;
            item.reset();
            item.item_id_ = it->first;
            G_CDataManager()->get_item_list()->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, item, it->second);

            const int inserted_slot =
                user->getCurCharacInvenW()->insertItemIntoInventory(
                    item, static_cast<eItemAddReason>(11), true, true);
            if (inserted_slot < 0) {
                continue;
            }

            packet.put_short(inserted_slot);
            packet.put_int(item.item_id_);
            packet.put_int(item.get_add_info());
            packet.put_short(item.avatar_select_);
            packet.put_byte(item.amplify_option_.getAbilityType());
            packet.put_short(item.amplify_option_.getAbilityValue());
            ++inserted_count;

            if (item.item_category_ == 1) {
                CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
                if (prototype) {
                    CLuckPoint::UseLuckPoint(user,
                                             item.item_category_,
                                             prototype->get_rarity(),
                                             prototype->get_grade());
                }
            }
        }
    }
    return inserted_count;
}

}  // namespace

// IDA_EA: 0x85B61BE
// IDA_NAME: _ZN6CParty12GetLuckPointEP10CLuckPointb
// DEMANGLED: CParty::GetLuckPoint(CLuckPoint *,bool)
bool CParty::GetLuckPoint(CLuckPoint* luck_point, bool live_members_only) {
    if (!luck_point) return live_members_only;

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }
        if (live_members_only && battle_data_.member_live(seat) != 1u) {
            continue;
        }
        luck_point->AddUser(get_user(seat));
    }
    // The original loop's final comparison (seat <= 3) is the return value,
    // so a non-null output object returns false after exhausting all seats.
    return false;
}

// IDA_EA: 0x859D70E
// IDA_NAME: _ZN6CParty17MemberPenaltyRateEv
// DEMANGLED: CParty::MemberPenaltyRate(void)
int CParty::MemberPenaltyRate() {
    return 1;
}

// IDA_EA: 0x85ADA30
// IDA_NAME: _ZN6CParty20CalcurateManageBonusEN15ManageBonusType1TEP5CUserj
// DEMANGLED: CParty::CalcurateManageBonus(ManageBonusType::T,CUser *,unsigned int)
long double CParty::CalcurateManageBonus(ManageBonusType::T bonus_type,
                                         CUser* user,
                                         unsigned int base_value) {
    if (!user || !user->getCurCharacR() ||
        user->get_charac_level() >= user->GetUserMaxLevel()) {
        return 0.0L;
    }

    CharacManageScript* const script = CharacManageScript::getInsance();
    if (!script) return 0.0L;

    const int manage_level = user->GetManageLevel();
    if (bonus_type == ManageBonusType::Gold) {
        return static_cast<long double>(base_value) *
               script->GetGoldBonus(manage_level);
    }
    return script->GetExpBonus(manage_level);
}

// IDA_EA: 0x85ADB00
// IDA_NAME: _ZN6CParty30set_clear_reward_exp_parameterEP5CUserjR27stClearRewardExpParameter_t
// DEMANGLED: CParty::set_clear_reward_exp_parameter(CUser *,unsigned int,stClearRewardExpParameter_t &)
int CParty::set_clear_reward_exp_parameter(
    CUser* user,
    unsigned int base_exp,
    stClearRewardExpParameter_t& parameter) {
    CDungeon* const dungeon = battle_field_.GetDungeon();
    if (!dungeon || !user) return 0;

    int member_slot = -1;
    for (int seat = 0; seat <= 3; ++seat) {
        if (get_user(seat) == user) {
            member_slot = seat;
            break;
        }
    }
    const unsigned int rank_bonus_index = static_cast<unsigned int>(
        G_CDataManager()->getRankBonusIndex(game_result_set_.raw_48));

    // The original keeps the member index adjacent to the parameter object
    // on the stack.  The recovered C++ body uses an explicit lookup instead
    // and writes its dead-member reduction into the first parameter slot.
    parameter.member_slot = member_slot;
    parameter.rank_bonus_index = static_cast<int32_t>(rank_bonus_index);
    parameter.death_penalty_rate =
        ToFloat(calDeadUserDungeonPenalty(parameter.member_slot));
    parameter.rank_bonus_rate = ToFloat(
        G_CDataManager()->get_server_parameter_script()
            ->GetExpBonusWeight(parameter.rank_bonus_index));
    parameter.avatar_bonus_rate = ToFloat(CalcAvatarBonus());
    parameter.creature_bonus_rate = ToFloat(CalcCreatureBonus(user));

    CEventBase* const pc_room_event =
        GlobalData::s_event_manager->GetRepeatEvent(8);
    if (pc_room_event && pc_room_event->IsEventing(user) == 1) {
        CPCRoomBurningEvent* const event =
            static_cast<CPCRoomBurningEvent*>(pc_room_event);
        parameter.pc_room_burning_rate =
            static_cast<float>(event->GetExpFactor()) / 100.0f;
    }

    CEventBase* const double_exp_event =
        GlobalData::s_event_manager->GetRepeatEvent(3);
    if (double_exp_event && double_exp_event->IsEventing(0) == 1) {
        CExpDoubleEvent* const event =
            static_cast<CExpDoubleEvent*>(double_exp_event);
        parameter.double_exp_event_rate =
            static_cast<float>(event->GetExpFactor()) / 100.0f;
    }

    if (user->IsExpItemAffected() == 1 &&
        user->EnablePeriodExpItem() == 1) {
        parameter.period_exp_item_rate = user->GetExpAffectRate();
    }

    CEventBase* const burning_fatigue_event =
        GlobalData::s_event_manager->GetRepeatEvent(15);
    if (burning_fatigue_event && burning_fatigue_event->IsEventing(0) == 1) {
        CBurningFatigueEvent* const event =
            static_cast<CBurningFatigueEvent*>(burning_fatigue_event);
        parameter.burning_fatigue_bonus_percent = static_cast<uint32_t>(
            event->getBonusExpRate(
                static_cast<uint16_t>(user->getCurCharacUsedFatigue())));
    }

    channel_script_t* const channels = G_CDataManager()->GetChannelScript();
    if (channels) {
        const uint8_t server_group = static_cast<uint8_t>(
            G_CEnvironment()->get_server_group());
        const uint32_t channel_no = static_cast<uint32_t>(
            G_CEnvironment()->get_channel_no());
        const long double channel_bonus =
            (G_CEnvironment()->get_server_type() == 1 ||
             G_CEnvironment()->get_server_type() == 2)
                ? channels->getBonusRateEventServer(server_group, channel_no)
                : channels->getBonusRate(server_group,
                                         channel_no,
                                         dungeon->get_index());
        parameter.channel_bonus_rate = ToFloat(channel_bonus);
    }

    if (user->IsHavePremiumAdvantage()) {
        parameter.premium_advantage_rate =
            static_cast<float>(
                user->GetPremiumInfo()->GetAdvantageExpRate()) / 100.0f;
    }
    parameter.server_event_rate =
        static_cast<float>(CServerEvent::GetExpRate()) / 100.0f;

    if (GlobalData::s_globalEffectManager_ &&
        GlobalData::s_globalEffectManager_->isAffectedEffect(2u) != 0) {
        parameter.global_effect_rate =
            GlobalData::s_globalEffectManager_->getEffectValue(2u);
    }

    const int pc_room_no = user->GetPCRoomNo();
    if (pc_room_no != 0 &&
        existSamePCRoomUser(static_cast<unsigned int>(pc_room_no)) != 0) {
        parameter.same_pc_room_party_rate =
            G_CDataManager()->get_server_parameter_script()
                ->get_fatigue_battery_constant_table()
                .pc_room_party_exp_bonus_rate_;
    }
    if (IsReturnUserParty() != 0) {
        parameter.return_user_party_rate = 0.2f;
    }

    parameter.character_manage_rate = ToFloat(
        CalcurateManageBonus(ManageBonusType::Exp, user, base_exp));
    const STExpReward growth_power = user->get_growth_power_exp_reward();
    parameter.growth_power_rate =
        static_cast<float>(growth_power.clear_reward_exp_bonus_rate) / 100.0f;
    if (user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(79)) == 1) {
        parameter.premium_exp_rate =
            static_cast<float>(user->getAddExpRateFromPremium(
                static_cast<ENUM_PREMIUM_TYPE>(79))) / 100.0f;
    }

    if (IsEventCharacParty() == 1) {
        EventNewCharacterReward* const event =
            static_cast<EventNewCharacterReward*>(
                GlobalData::s_event_manager->GetRepeatEvent(162));
        if (event) {
            parameter.new_character_party_rate = ToFloat(event->eventExpRatio());
        }
    }

    user->set_charac_party_bonus_exp(0);
    user->set_charac_member_bonus_exp(0);
    user->set_charac_fatigue_buf_bonus_exp(0);
    user->set_charac_seria_buf_bonus_exp(0);
    return 1;
}

// IDA_EA: 0x859BA98
// IDA_NAME: _ZN6CParty25increase_result_card_goldERN8WongWork18stGenerateResult_tE
// DEMANGLED: CParty::increase_result_card_gold(WongWork::stGenerateResult_t &)
bool CParty::increase_result_card_gold(WongWork::stGenerateResult_t& result) {
    std::vector<Inven_Item>::iterator item = result.items.begin();
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(checkValidUser(seat)) != 1u) {
            continue;
        }
        if (item == result.items.end()) {
            continue;
        }

        QuickParty::RewardData_Param parameter(this);
        QuickParty::RewardData_Result reward;
        reward.raw_00 = item->get_add_info();
        set_basic_reward(static_cast<QuickParty::BasicRewardType>(0x0b),
                         parameter,
                         reward);
        item->set_add_info(reward.raw_00);
        ++item;
    }
    return false;
}

// IDA_EA: 0x85AE0D4
// IDA_NAME: _ZN6CParty12clear_rewardEi
// DEMANGLED: CParty::clear_reward(int)
//
// CMD049 calls this before it starts the card-selection timers.  The paired
// slots are later consumed by CParty::dungeon_clear(), so they are prepared
// here in BattleData rather than through a dispatcher-local cache.
void CParty::clear_reward(int rank_index) {
    CDungeon* const dungeon = battle_field_.GetDungeon();
    CUser* const manager = getManager();
    PvP_Room* const zombie_room = manager ? manager->GetPVPRoom() : 0;
    if ((dungeon && dungeon->isZombieDungeon()) ||
        (zombie_room &&
         (zombie_mode::IsActiveRoom(zombie_room) ||
          zombie_mode::IsFinishedRoom(zombie_room)))) {
        return;
    }
    const int member_count = get_member_count();
    if (!dungeon || member_count <= 0 || dungeon->isTournamentDungeon()) {
        return;
    }

    const int standard_level = dungeon->get_dimension_possible() > 0
        ? getStandardDimensionLevel()
        : dungeon->get_standard_level();
    const int dungeon_difficulty = battle_field_.get_dungeon_diff();
    const int rank_bonus_index = G_CDataManager()->getRankBonusIndex(rank_index);
    MazeScript* const maze = battle_field_.getMaze();
    const int playable_cell_count = CountPlayableMazeCells(maze);

    ++battle_data_.map_move_count();
    float clear_reward_scale = 0.0f;
    if (playable_cell_count > 0) {
        clear_reward_scale =
            static_cast<float>(battle_data_.map_move_count()) /
            static_cast<float>(playable_cell_count);
    }

    WongWork::CClearReward* const generator =
        static_cast<WongWork::CClearReward*>(
            G_CDataManager()->getItemGenerator()->getGenerator(
                WongWork::CItemGeneratorMgr::kClearReward));
    if (!generator) return;

    const int maximum_rate =
        generator->getRewardItemRatePerMapMaxCount(playable_cell_count);
    if (maximum_rate != 0) {
        clear_reward_scale = static_cast<float>(maximum_rate) *
            clear_reward_scale / 10000.0f;
    }

    // The original first takes the lowest party rate for the gold card, then
    // overwrites the generator's per-user rate before every item-card roll.
    // Keep those two uses separate: sharing the reduced rate across all item
    // cards makes a single restricted user suppress everybody's item drops.
    const bool standard_drop_rate_restricted = IsStandardDropRateRestricted();
    const int minimum_standard_drop_rate = GetMinimumStandardDropRate(this);
    WongWork::stGenerateRefData_t item_reference;
    PrepareClearRewardReference(item_reference,
                                dungeon,
                                standard_level,
                                dungeon_difficulty,
                                member_count,
                                100,
                                clear_reward_scale);

    WongWork::stGenerateRefData_t gold_reference;
    PrepareGoldRewardReference(gold_reference,
                               battle_data_,
                               standard_level,
                               dungeon_difficulty,
                               member_count,
                               rank_bonus_index);

    // The normal pair's first member is the clear-gold record.  Power-War
    // dungeons use their dedicated per-character generator; all other
    // dungeons use the generic clear-gold virtual path.
    WongWork::stGenerateResult_t gold_result;
    if (dungeon->circulation_statistic_class_ != 0) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u) {
                generator->power_war_dungeon_gold_clear_reward(
                    gold_reference,
                    gold_result,
                    get_user(seat)->get_charac_level());
            }
        }
    } else {
        generator->generateSpecificItem(gold_reference, gold_result);
    }
    increase_result_card_gold(gold_result);

    std::vector<Inven_Item>::const_iterator gold_it = gold_result.items.begin();
    float dead_member_penalties[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int seat = 0; seat <= 3; ++seat) {
        ResetRewardPair(battle_data_.normal_reward_item(seat, 0));
        ResetRewardPair(battle_data_.special_reward_item(seat, 0));
        ResetRewardPair(battle_data_.event_reward_item(seat, 0));
        play_result_values_[1][seat] = 0;

        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }
        CopyNextGeneratedItem(gold_result,
                              gold_it,
                              battle_data_.normal_reward_item(seat, 0));

        // CParty +0x380/+0x384/+0x388 are BattleData's live/entered/state
        // flags.  The old path increments the personal map counter only when
        // none of those marks indicates an eligible member, then derives the
        // dead-member money penalty from that counter.
        if (battle_data_.member_live(seat) != 1u &&
            battle_data_.member_enter_map(seat) != 1u &&
            battle_data_.member_state_flag(seat) != 1u) {
            ++battle_data_.member_map_move_count(seat);
        }
        const long double dead_member_penalty = calDeadUserDungeonPenalty(seat);
        dead_member_penalties[seat] =
            static_cast<float>(dead_member_penalty);
        Inven_Item& normal_money = battle_data_.normal_reward_item(seat, 0);
        if (dead_member_penalty > 0.0L) {
            normal_money.set_add_info(static_cast<int>(
                static_cast<long double>(normal_money.get_add_info()) *
                (1.0L - dead_member_penalty)));
            battle_data_.member_state_flag(seat) = 1;
        }
        normal_money.set_add_info(
            normal_money.get_add_info() * minimum_standard_drop_rate / 100);

    }

    // Event-dungeon admission marks the invitation as consumed.  The clear
    // path then delivers CDungeon+0x86c's reward pair vector before any card
    // selection state is exposed to the client.
    if (dungeon->isEventDungeon()) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
                continue;
            }
            CUser* const user = get_user(seat);
            if (user->IsDeleteEventDungeonInvitation() == 1) {
                user->EventDungeonClearReward(dungeon->recovery_stair_items_);
            }
        }
    }

    // The guild share marker is set for every valid member who has the same
    // guild key as any active "today guild member".  The original deliberately
    // does not require the matching member itself to carry that daily mark.
    bool same_guild_reward[4] = {false, false, false, false};
    for (int source_seat = 0; source_seat <= 3; ++source_seat) {
        if (static_cast<unsigned char>(_checkValidUser(source_seat)) != 1u ||
            get_user(source_seat)->IsTodayGuildMember() != 1) {
            continue;
        }
        const int guild_key = get_user(source_seat)->get_charac_guildkey();
        same_guild_reward[source_seat] = true;
        for (int target_seat = 0; target_seat <= 3; ++target_seat) {
            if (static_cast<unsigned char>(_checkValidUser(target_seat)) == 1u &&
                get_user(target_seat)->get_charac_guildkey() == guild_key) {
                same_guild_reward[target_seat] = true;
            }
        }
    }

    GuildParameterScript* const guild_parameters =
        G_CDataManager()->GetGuildParameterScriptForBuild();
    const bool top_rank_clear =
        G_CDataManager()->getNewRankIndex(rank_index) <= 2;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = get_user(seat);
        if (top_rank_clear) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                user,
                static_cast<APSystem::_ActionGroupIndex>(16),
                0,
                static_cast<ENUM_PACKETCLASS>(0));
        }

        const int guild_temp_exp = user->get_guild_temp_exp();
        bool grant_guild_exp = dungeon->circulation_statistic_class_ != 0;
        if (!grant_guild_exp) {
            const long double level_penalty = G_CDataManager()->BaseExpPenalty(
                user->get_charac_level(), standard_level);
            grant_guild_exp = level_penalty >= 1.0L || dungeon->isRiskDungeon();
        }

        if (grant_guild_exp && guild_parameters) {
            STGuildDBInfoOnly* const guild_info = user->GetGuildDBInfo();
            const int guild_member_count =
                guild_info ? guild_info->guild_member_count : 0;
            int guild_exp = static_cast<int>(
                static_cast<long double>(guild_temp_exp) *
                guild_parameters->getExpWeight(guild_member_count));
            if (same_guild_reward[seat]) {
                guild_exp += static_cast<int>(
                    guild_parameters->today_guild_member_weight / 100.0 *
                    static_cast<double>(guild_exp));
                user->add_guild_today_exp(guild_exp);
            } else {
                user->add_guild_exp(guild_exp);
            }
        }
        user->reset_guild_temp_exp();
    }

    // The original advances the shared reward RNG per member in this exact
    // order: normal card, paid card, then PC-room card.  Keep all three in one
    // seat loop so a party does not redistribute random rewards among members.
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }

        item_reference.standard_drop_rate = standard_drop_rate_restricted
            ? get_user(seat)->getStdDropRate()
            : 100;

        // The normal card always carries clear gold and may add one item.  A
        // failed weighted roll deliberately leaves its second record empty.
        item_reference.clear_reward_scale = dead_member_penalties[seat];
        item_reference.generator_data.clear_reward.item_probability_scale =
            clear_reward_scale;
        item_reference.generator_data.clear_reward.clear_reward_drop_probability =
            get_user(seat)->isAffectedPremium(
                static_cast<ENUM_PREMIUM_TYPE>(12)) == 1 ? 3 : 0;
        WongWork::stGenerateResult_t normal_item_result;
        GetLuckPoint(&normal_item_result.luck_point, false);
        generator->generateRandomItem(item_reference, normal_item_result);
        if (!normal_item_result.items.empty()) {
            battle_data_.normal_reward_item(seat, 1).setCopy(
                normal_item_result.items.front());
        }

        // The paid-card pair reuses the normal card's record shape, but the
        // original clears the copied money amount: its visible reward is the
        // independently generated item.  Its cost is physical CParty
        // +0x1f4..+0x200, the same array used later by
        // _checkGoldCardSelectable().
        battle_data_.event_reward_item(seat, 0).setCopy(
            battle_data_.normal_reward_item(seat, 0));
        battle_data_.event_reward_item(seat, 0).set_add_info(0);
        // The free-gold-card event keeps the map completion probability.
        // Only the normal paid path replaces it with the configured
        // gold-card creation rate (official v152 lifecycle).
        item_reference.generator_data.clear_reward.item_probability_scale =
            clear_reward_scale;
        CEventBase* const no_cost_event =
            GlobalData::s_event_manager->GetRepeatEvent(16);
        if (!no_cost_event || no_cost_event->IsEventing(0) != 1) {
            long double commission_multiplier = 1.0L;
            CEventBase* const premium_gold_card_event =
                GlobalData::s_event_manager->GetRepeatEvent(65);
            if (premium_gold_card_event &&
                premium_gold_card_event->IsEventing(0) == 1 &&
                IsPremiumGoldCardParty() == 1) {
                commission_multiplier = G_CDataManager()
                    ->getPremiumGoldCardCommissionMultiplier();
            }
            CConditionEventManager* const condition_events =
                G_CGameManager()->GetConditionEventManager();
            if (condition_events &&
                condition_events->IsFreeGoldCard(
                    get_user(seat),
                    static_cast<char>(
                        get_user(seat)->GetCurConditionEventStep() + 1)) != 0) {
                commission_multiplier = 0.0L;
            }
            play_result_values_[1][seat] = static_cast<int>(
                static_cast<long double>(
                    generator->getGoldCardCommission(standard_level)) *
                commission_multiplier);
            item_reference.generator_data.clear_reward.item_probability_scale =
                static_cast<float>(generator->getGoldCardCreateRate());
        }

        item_reference.generator_data.clear_reward.clear_reward_drop_probability =
            get_user(seat)->isAffectedPremium(
                static_cast<ENUM_PREMIUM_TYPE>(12)) == 1 ? 4 : 1;
        item_reference.clear_reward_scale = dead_member_penalties[seat];
        WongWork::stGenerateResult_t gold_card_result;
        GetLuckPoint(&gold_card_result.luck_point, false);
        CEventBase* const premium_gold_card_event =
            GlobalData::s_event_manager->GetRepeatEvent(65);
        if (premium_gold_card_event &&
            premium_gold_card_event->IsEventing(0) == 1 &&
            IsPremiumGoldCardParty() == 1) {
            generator->generatePremiumGoldCardItem(item_reference,
                                                   gold_card_result,
                                                   -1,
                                                   GetPremiumGoldCardDefaultItem());
        } else {
            generator->generateRandomItem(item_reference, gold_card_result);
            if (gold_card_result.items.empty() &&
                (IsReturnUserParty() == 1 || IsEventCharacParty() == 1)) {
                generator->generatePremiumGoldCardItem(item_reference,
                                                       gold_card_result,
                                                       1,
                                                       10000);
            }
        }

        if (!gold_card_result.items.empty()) {
            battle_data_.event_reward_item(seat, 1).setCopy(
                gold_card_result.items.front());
            if (battle_data_.event_reward_item(seat, 1).getKey() == -1 ||
                dungeon->circulation_statistic_class_ != 0) {
                battle_data_.event_reward_item(seat, 1).reset();
            }
        } else {
            CEventBase* const blank_card_event =
                GlobalData::s_event_manager->GetRepeatEvent(19);
            if (blank_card_event && blank_card_event->IsEventing(0) == 1) {
                generator->generateGoldCardBlankItem(
                    battle_data_.event_reward_item(seat, 1));
            }
        }

        if (dungeon->isRiskDungeon()) {
            LogManager::logFormat(1, "party.cpp", __FUNCTION__, __LINE__,
                "[CARD_FLOW][GENERATED] dungeon=%d char=%d standard_level=%d item_level=%d normal=%d gold=%d fee=%d",
                dungeon->get_index(), get_user(seat)->getCurCharacNo(), standard_level,
                ResolveClearRewardItemLevel(dungeon, standard_level),
                battle_data_.normal_reward_item(seat, 1).getKey(),
                battle_data_.event_reward_item(seat, 1).getKey(), play_result_values_[1][seat]);
        }

        // `value_87a` is the original dungeon-script switch for an event
        // clear-card replacement.  It clears all three pairs, then keeps the
        // one fixed item in the normal card's second slot and resets the
        // shared event-dungeon clear counters.
        if (dungeon->village_attack_revenge_dungeon_ != 0) {
            Inven_Item replacement_card;
            replacement_card.reset();
            replacement_card.item_id_ =
                G_CDataManager()->getEventDungeonClearRewardItemId();
            G_CDataManager()->get_item_list()->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, replacement_card, 1);
            ResetRewardPair(battle_data_.special_reward_item(seat, 0));
            ResetRewardPair(battle_data_.event_reward_item(seat, 0));
            ResetRewardPair(battle_data_.normal_reward_item(seat, 0));
            battle_data_.normal_reward_item(seat, 1).setCopy(replacement_card);
            battle_data_.ResetEventDungeonClearPoint();
        }

        // PC-room members receive the third independently generated pair.
        if (get_user(seat)->isAffectedPremium(
                static_cast<ENUM_PREMIUM_TYPE>(12)) == 1) {
            battle_data_.special_reward_item(seat, 0).setCopy(
                battle_data_.normal_reward_item(seat, 0));
            battle_data_.special_reward_item(seat, 0).set_add_info(0);
            item_reference.generator_data.clear_reward
                .clear_reward_drop_probability = 5;
            item_reference.clear_reward_scale = dead_member_penalties[seat];
            item_reference.generator_data.clear_reward.item_probability_scale =
                clear_reward_scale;
            WongWork::stGenerateResult_t pc_room_result;
            GetLuckPoint(&pc_room_result.luck_point, false);
            generator->generateRandomItem(item_reference, pc_room_result);
            if (!pc_room_result.items.empty()) {
                battle_data_.special_reward_item(seat, 1).setCopy(
                    pc_room_result.items.front());
            } else {
                CEventBase* const pc_room_blank_event =
                    GlobalData::s_event_manager->GetRepeatEvent(29);
                if (pc_room_blank_event &&
                    pc_room_blank_event->IsEventing(0) == 1) {
                    generator->generatePcRoomCardBlankItem(
                        battle_data_.special_reward_item(seat, 1));
                }
            }
        }
    }

    // The card-selection state machine runs later, but clear EXP is awarded
    // immediately after the three reward pairs have been fixed.  This is the
    // original ordering: a level-up can notify the client before CMD35 is
    // assembled and before any player picks a card.
    unsigned int standard_mob_reward = 0;
    if (dungeon->circulation_statistic_class_ == 0) {
        G_CDataManager()->get_mob_reward(standard_level, standard_mob_reward);
    }
    CEventBase* const party_exp_event =
        GlobalData::s_event_manager->GetRepeatEvent(24);
    const long double party_event_weight = party_exp_event
        ? static_cast<CPartyExpBonusEvent*>(party_exp_event)->getBonusRate(
              member_count, isHelpAbuseParty() != 0)
        : 0.0L;
    const long double party_weight = static_cast<long double>(
        G_CDataManager()->get_server_parameter_script()
            ->GetClearDungeonPartyExpWeight(member_count)) +
        party_event_weight;
    const long double difficulty_weight = static_cast<long double>(
        G_CDataManager()->get_server_parameter_script()
            ->GetClearDungeonDifficultyExpWeight(dungeon_difficulty));

    const long double standard_total_exp =
        static_cast<long double>(battle_data_.GetTotalKilledMonsterCount()) *
        static_cast<long double>(standard_mob_reward) / 2.0L * party_weight *
        difficulty_weight * dungeon->get_exp_weight();

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = get_user(seat);
        user->setChattingMessageCount(0);

        long double member_total_exp = standard_total_exp;
        if (dungeon->circulation_statistic_class_ != 0) {
            unsigned int power_war_mob_reward = 0;
            G_CDataManager()->get_mob_reward(user->get_charac_level(),
                                              power_war_mob_reward);
            member_total_exp =
                static_cast<long double>(battle_data_.GetTotalKilledMonsterCount()) *
                static_cast<long double>(power_war_mob_reward) / 2.0L *
                party_weight * difficulty_weight * dungeon->get_exp_weight();
        }

        unsigned int base_exp = 0;
        if (dungeon->circulation_statistic_class_ != 0) {
            base_exp = static_cast<unsigned int>(member_total_exp) /
                static_cast<unsigned int>(member_count);
        } else {
            long double level_penalty = 1.0L;
            if (!dungeon->isRiskDungeon()) {
                level_penalty = G_CDataManager()->BaseExpPenalty(
                    user->get_charac_level(), standard_level);
            }
            base_exp = static_cast<unsigned int>(
                static_cast<long double>(MemberPenaltyRate()) *
                member_total_exp * level_penalty /
                static_cast<long double>(member_count));
        }

        QuickParty::RewardData_Param reward_parameter(this);
        QuickParty::RewardData_Result reward_result;
        reward_result.raw_00 = base_exp;
        set_random_reward(static_cast<QuickParty::RandomBuffType>(0),
                          reward_parameter,
                          reward_result);
        base_exp = reward_result.raw_00;

        stClearRewardExpParameter_t exp_parameter;
        stClearRewardBonusExp_t bonus_exp;
        set_clear_reward_exp_parameter(user, base_exp, exp_parameter);
        unsigned int total_exp = getClearRewardBonusExp(
            user, base_exp, exp_parameter, bonus_exp);
        const int champion_clear_rate =
            champion_break::GetClearExpBonusRate(user);
        const unsigned int champion_clear_bonus =
            champion_clear_rate > 0
                ? static_cast<unsigned int>(
                      static_cast<unsigned long long>(base_exp) *
                      static_cast<unsigned int>(champion_clear_rate) / 100u)
                : 0u;
        total_exp += champion_clear_bonus;
        LogManager::logFormat(
            1,
            "dungeon42_diag.cpp",
            "void CParty::clear_reward(int)",
            0,
            "[DUNGEON42][CLEAR_EXP] uid=%d dungeon=%d killed=%d standard_level=%d standard_mob_reward=%u base_exp=%u total_exp=%u party_weight=%.6f difficulty_weight=%.6f dungeon_weight=%.6f",
            user->GetUID(),
            dungeon->get_index(),
            battle_data_.GetTotalKilledMonsterCount(),
            standard_level,
            standard_mob_reward,
            base_exp,
            total_exp,
            static_cast<double>(party_weight),
            static_cast<double>(difficulty_weight),
            static_cast<double>(dungeon->get_exp_weight()));
        user->calcHelpAbuseRatio(total_exp, isHelpAbuseParty(user) != 0);
        user->reset_level_before_dungeon();

        int gained_sp = 0;
        int gained_sfp = 0;
        if (user->gain_exp_sp(static_cast<int>(total_exp),
                              gained_sp,
                              gained_sfp,
                              static_cast<eExpAddReason>(2),
                              0,
                              false)) {
            PacketGuard level_packet;
            level_packet.put_header(0, 2);
            level_packet.put_byte(1);
            level_packet.put_short(1);
            user->make_basic_info(level_packet, 1);
            level_packet.finalize(true);
            user->Send(level_packet);
        }

        // NOTI35 is the clear-result report.  Its 18 EXP components are the
        // physical stClearRewardBonusExp_t order returned above; keep that
        // order rather than repacking a new result structure.
        PacketGuard clear_packet;
        clear_packet.put_header(0, 35);
        clear_packet.put_int(static_cast<int>(bonus_exp.reserved_bonus_exp) +
                             static_cast<int>(base_exp));
        clear_packet.put_int(static_cast<int>(bonus_exp.rank_bonus_exp));
        user->set_charac_party_bonus_exp(
            static_cast<int>(bonus_exp.rank_bonus_exp));

        int party_bonus_exp = 0;
        if (party_weight != 0.0L) {
            party_bonus_exp = static_cast<int>(base_exp) - static_cast<int>(
                static_cast<long double>(base_exp) / party_weight);
        }
        if (base_exp != 0u) {
            const unsigned int bonus_scale =
                bonus_exp.premium_advantage_bonus_exp +
                bonus_exp.server_event_bonus_exp +
                bonus_exp.reserved_bonus_exp +
                bonus_exp.burning_fatigue_bonus_exp +
                bonus_exp.event_bonus_exp + bonus_exp.avatar_bonus_exp +
                bonus_exp.rank_bonus_exp + bonus_exp.channel_bonus_exp;
            party_bonus_exp *= static_cast<int>(bonus_scale / base_exp) + 1;
        }
        clear_packet.put_int(party_bonus_exp);
        clear_packet.put_int(static_cast<int>(bonus_exp.avatar_bonus_exp));
        clear_packet.put_byte(static_cast<int>(bonus_exp.event_bonus_type));
        // 0828 presents these sources in separate result categories.  Keep
        // the legacy event field in its original slot and place the two
        // independently produced bonuses in their newly appended slots.
        clear_packet.put_int(static_cast<int>(bonus_exp.event_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.period_exp_item_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.premium_advantage_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.same_pc_room_party_bonus_exp));
        clear_packet.put_int(static_cast<int>(bonus_exp.channel_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.upper_member_bonus_exp));
        clear_packet.put_int(static_cast<int>(bonus_exp.creature_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.return_user_party_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.new_character_party_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.character_manage_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.growth_power_bonus_exp));
        clear_packet.put_int(
            static_cast<int>(bonus_exp.premium_exp_bonus_exp));

        // f17: server-wide EXP event (client category 0).
        clear_packet.put_int(
            static_cast<int>(bonus_exp.server_event_bonus_exp));
        // f18: Heras Laboratory; no authoritative producer is restored.
        clear_packet.put_int(0);
        // f19: burning-fatigue accelerated reward (client category 8).
        clear_packet.put_int(
            static_cast<int>(bonus_exp.burning_fatigue_bonus_exp));
        // f20: variousbufflist Champion_Break/dungeon_clear_exp_rate.
        clear_packet.put_int(static_cast<int>(champion_clear_bonus));
        // f21: character bonus; no proven producer.
        clear_packet.put_int(0);

        // IDA 0x85B1B26 keeps this cursor in a mutable stack local: the
        // indexed PacketBuf overload writes at the saved offset, so it has
        // the original `int&` ABI rather than a const cursor.
        int quest_item_count_index = clear_packet.get_index();
        clear_packet.put_byte(0);
        clear_packet.put_byte(quest_item_count_index,
                              AppendQuestClearItems(
                                  user, battle_field_.get_dungeon_index(),
                                  clear_packet));

        for (int member_slot = 0; member_slot <= 3; ++member_slot) {
            if (static_cast<unsigned char>(
                    _checkValidUser(member_slot)) == 1u) {
                _makeRewardItemInfo(
                    get_user(member_slot),
                    &battle_data_.normal_reward_item(member_slot, 0),
                    clear_packet);
            } else {
                clear_packet.put_byte(0);
            }
        }

        // The 0828 receiver always consumes eight normal-card slots.  The
        // 0627 reward generator owns only the first four party-member slots.
        for (int member_slot = 4; member_slot < 8; ++member_slot) {
            clear_packet.put_byte(0);
        }
        clear_packet.put_int(play_result_values_[1][seat]);

        // After the clear-card cost, the 0828 client reads two independent
        // eight-slot card groups.  The first is the official empty group.
        for (int member_slot = 0; member_slot < 8; ++member_slot) {
            clear_packet.put_byte(0);
        }

        // The second group is the premium-type-12 special clear card for the
        // four authoritative party seats.  _makeRewardItemInfo preserves the
        // original try-inventory/result grammar; the actual reward insertion
        // remains owned by dungeon_clear().  0828 widened the receiver to
        // eight slots, so the four non-party tail slots stay explicitly zero.
        for (int member_slot = 0; member_slot <= 3; ++member_slot) {
            CUser* const member =
                static_cast<unsigned char>(_checkValidUser(member_slot)) == 1u
                    ? get_user(member_slot) : 0;
            if (member && member->isAffectedPremium(
                              static_cast<ENUM_PREMIUM_TYPE>(12)) == 1) {
                _makeRewardItemInfo(
                    member,
                    &battle_data_.special_reward_item(member_slot, 0),
                    clear_packet);
            } else {
                clear_packet.put_byte(0);
            }
        }
        for (int member_slot = 4; member_slot < 8; ++member_slot) {
            clear_packet.put_byte(0);
        }

        const int hell_party_max = battle_field_.hell_party_value_max();
        const int hell_party_percent = hell_party_max != 0
            ? static_cast<int>(battle_data_.hell_party_value_total() * 100.0f /
                               static_cast<float>(hell_party_max))
            : 0;
        clear_packet.put_int(hell_party_percent);
        clear_packet.put_byte(IsReturnUserParty() == 1 ? 1 : 0);
        clear_packet.put_byte(IsEventCharacParty() == 1 ? 1 : 0);
        clear_packet.put_byte(exp_parameter.death_penalty_rate != 0.0f
                                  ? 1
                                  : 0);
        if (dungeon->get_index() == 3900) {
            // The client's outer NOTI714 log hides the clear-result body.
            // Capture this bounded plaintext packet before encryption so a
            // missing result view can be distinguished from missing rewards.
            const int length = clear_packet.get_index();
            const int captured = std::min(length, 512);
            const unsigned char* bytes = reinterpret_cast<const unsigned char*>(
                clear_packet.get_packet(0));
            const char hex_digits[] = "0123456789ABCDEF";
            char hex[1025] = {};
            if (bytes) {
                for (int index = 0; index < captured; ++index) {
                    hex[index * 2] = hex_digits[bytes[index] >> 4];
                    hex[index * 2 + 1] = hex_digits[bytes[index] & 15];
                }
            }
            LogManager::logFormat(
                1, __FILE__, "CParty::clear_reward", __LINE__,
                "[HERAS_RESULT][NOTI35] charac=%d party=%d map=%d "
                "clear_state=%d base_exp=%u total_exp=%u "
                "gold=%d card_item=%d len=%d captured=%d hex=%s",
                user->getCurCharacNo(), GetPartyIndex(),
                battle_field_.GetCurrentMapIndex(), get_dungeon_clear_state(),
                base_exp, total_exp,
                battle_data_.normal_reward_item(seat, 0).get_add_info(),
                battle_data_.normal_reward_item(seat, 1).getKey(),
                length, captured, hex);
        }
        clear_packet.finalize(true);
        user->Send(clear_packet);

        // CParty+0xcd8 is CBattle_Field::instance_dungeon_mode_.  The two
        // original progress tails both suppress instance mode 1.  The first
        // additionally excludes blood, dimension, circulation and TOD maps.
        const bool common_progress_dungeon =
            battle_field_.instance_dungeon_mode() != 1 &&
            dungeon->get_blood_dungeon_type() == 0 &&
            dungeon->get_dimension_possible() == 0 &&
            dungeon->circulation_statistic_class_ == 0 &&
            !dungeon->isTowerOfDespairDungeon();
        if (common_progress_dungeon && user->getCurCharacR() &&
            user->IsProperDungeonUser() == 1) {
            ARAD::Singleton<HeroMissionEvent>::Get()->processMission(
                user, HeroMissionCondition::MissionNo::CLEAR_DUNGEON, 1);
            user->IncProperDungeonClearCount();
            king_of_pick::OnProperDungeonClear(user);
            shooting_star_dimension_pocket::OnProperDungeonClear(user);
            CConditionEventManager* const condition_events =
                G_CGameManager()->GetConditionEventManager();
            if (condition_events) {
                condition_events->ProcessCheckStepUp(user, 2);
            }
        }

        const bool proper_level_progress_dungeon =
            battle_field_.instance_dungeon_mode() != 1 &&
            dungeon->get_blood_dungeon_type() == 0 &&
            dungeon->circulation_statistic_class_ == 0 &&
            !dungeon->isTowerOfDespairDungeon();
        if (proper_level_progress_dungeon && user->getCurCharacR() &&
            user->IsProperLevelDungeonUser() == 1) {
            user->ProcProperLevelDungeonClear();
        }
    }

    if (IsPremiumGoldCardParty() == 1) {
        PacketGuard premium_gold_card_packet;
        premium_gold_card_packet.put_header(0, 298);
        premium_gold_card_packet.finalize(true);
        send_to_party(premium_gold_card_packet);
    }
    if (!dungeon->isEventDungeon()) {
        DungeonPermission(rank_index);
    }
}

// IDA_EA: 0x85BDDFC
// IDA_NAME: _ZN6CParty15addDungeonClearEPK8CDungeonii
// DEMANGLED: CParty::addDungeonClear(CDungeon const*,int,int)
int CParty::addDungeonClear(const CDungeon* dungeon,
                            int rank_index,
                            int member_slot) {
    if (!dungeon || member_slot < 0 || member_slot > 3) return 0;

    CUser* const user = get_user(member_slot);
    if (!user) return 0;
    for (int result_slot = 0; result_slot <= 3; ++result_slot) {
        if (static_cast<unsigned short>(user->get_unique_id()) !=
            game_result_set_.shot_counts[result_slot].shot_count) {
            continue;
        }

        int dungeon_type = 0;
        if (dungeon->circulation_statistic_class_ != 0) {
            dungeon_type = 1;
        } else if (dungeon->get_dimension_possible() > 0) {
            dungeon_type = 2;
        } else if (dungeon->getDungeonKind() == 1) {
            dungeon_type = 3;
        } else if (dungeon->get_blood_dungeon_type() == 1) {
            dungeon_type = 4;
        } else if (dungeon->get_blood_dungeon_type() == 2) {
            dungeon_type = 5;
        } else if (dungeon->isTowerOfDespairDungeon()) {
            dungeon_type = 6;
        } else if (dungeon->isTournamentDungeon()) {
            dungeon_type = 7;
        }
        const unsigned int clear_count = static_cast<unsigned int>(
            game_result_set_.shot_counts[result_slot].raw_04);
        if (dungeon_type == 1) {
            if (G_CDataManager()->getDungeonPermissionUltimateRankRequirement() <=
                    rank_index &&
                clear_count <= 30u &&
                members_[member_slot].used_coin_count == 0) {
                return user->AddDungeonClear(battle_field_.get_dungeon_index(), 3);
            }
        } else if (dungeon_type == 2) {
            if (G_CDataManager()->getDungeonPermissionUltimateRankRequirement() <=
                    rank_index &&
                clear_count <= 50u &&
                members_[member_slot].used_coin_count == 0) {
                return user->AddDungeonClear(battle_field_.get_dungeon_index(), 3);
            }
        } else if (dungeon_type == 0 &&
                   G_CDataManager()->getDungeonPermissionNormalRankRequirement() <=
                       rank_index &&
                   clear_count <= 10u &&
                   members_[member_slot].used_coin_count == 0) {
            return user->AddDungeonClear(battle_field_.get_dungeon_index(), 3);
        }
        return 0;
    }
    return 0;
}

// IDA_EA: 0x85B12F8
// IDA_NAME: _ZN6CParty17DungeonPermissionEi
// DEMANGLED: CParty::DungeonPermission(int)
char CParty::DungeonPermission(int rank_index) {
    CDungeon* const dungeon = G_CDataManager()->find_dungeon(
        battle_field_.get_dungeon_index());
    if (!dungeon) return 0;

    const int dungeon_index = battle_field_.get_dungeon_index();
    const int dungeon_difficulty = battle_field_.get_dungeon_diff();
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) continue;
        CUser* const user = get_user(seat);
        if (!user || user->get_charac_level() < dungeon->get_min_level()) {
            continue;
        }

        int newly_permitted = 0;
        switch (dungeon_difficulty) {
        case 1:
            if (G_CDataManager()->getDungeonPermissionHardRankRequirement() <=
                rank_index) {
                newly_permitted = user->AddDungeonClear(dungeon_index, 2);
            }
            break;
        case 2: {
            const std::set<int>* const ultimate_dungeons =
                reinterpret_cast<const std::set<int>*>(
                    G_CDataManager()->getUltimateDungeonList());
            if (ultimate_dungeons &&
                ultimate_dungeons->find(dungeon_index) !=
                    ultimate_dungeons->end()) {
                newly_permitted = addDungeonClear(dungeon, rank_index, seat);
            }
            break;
        }
        case 0:
            newly_permitted = user->AddDungeonClear(dungeon_index, 1);
            break;
        default:
            break;
        }

        if (newly_permitted != 0) {
            PacketGuard packet;
            packet.put_header(0, 5);
            packet.put_short(1);
            packet.put_short(dungeon_index);
            packet.put_byte(dungeon_difficulty + 1);
            packet.finalize(true);
            user->Send(packet);
        }
    }
    return 0;
}
