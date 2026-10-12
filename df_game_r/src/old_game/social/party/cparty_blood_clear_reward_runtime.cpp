#include "old_game/social/party/party.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>
#include <vector>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistics_runtime.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/item_generator_drop_types.h"
#include "old_data/channel/channel_script_t.h"
#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/dungeon/instance/luck_point_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

void SendBloodRewardOverflowMail(CUser* user, const Inven_Item& reward) {
    char title[21];
    char text[256];
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_153", 0),
                 0x14u);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_154", 0),
                 0xffu);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        reward,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        8,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

}  // namespace

// IDA_EA: 0x85B6A48
// IDA_NAME: _ZN6CParty18onBloodRewardTimerE13TIMER_MESSAGE
// DEMANGLED: CParty::onBloodRewardTimer(TIMER_MESSAGE)
int CParty::onBloodRewardTimer(TIMER_MESSAGE message) {
    if (gen_timer_key(message) == 0 || battle_field_.getBloodState() == 0) {
        return 0;
    }

    CBattle_Field::CBloodClearRewardData& reward =
        battle_field_.blood_clear_reward();
    switch (message) {
        case TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_1:
            if (battle_field_.getBloodState() <= 2) {
                return 0;
            }

            for (int seat = 0; seat <= 3; ++seat) {
                CUser* const user = get_user(seat);
                if (!user || static_cast<unsigned char>(checkValidUser(seat)) !=
                                 1u) {
                    continue;
                }

                BloodClearRewardPacket(
                    seat,
                    reward.getCurrentBloodRound(),
                    reward.getRewardItemCount(0),
                    reward.getRewardItem(0),
                    reward.getRewardItemCount(1),
                    reward.getRewardItem(1),
                    reward.getRewardItemCount(2),
                    reward.getRewardItem(2),
                    reward.getRewardItemCount(3),
                    reward.getRewardItem(3));
            }

            battle_field_.setBloodState(static_cast<ENUM_BLOOD_STATE>(4));
            G_TimerQueue()->InsertTimer(
                1,
                GetPartyIndex(),
                TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_2,
                8,
                static_cast<unsigned int>(
                    gen_timer_key(TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_2)),
                TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_2);
            return 1;

        case TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_2:
            if (battle_field_.getBloodState() <= 3) {
                return 0;
            }

            for (int seat = 0; seat <= 3; ++seat) {
                CUser* const user = get_user(seat);
                if (!user || static_cast<unsigned char>(checkValidUser(seat)) !=
                                 1u) {
                    continue;
                }

                for (int item_slot = 0;
                     item_slot < reward.getRewardItemCount(seat);
                     ++item_slot) {
                    Inven_Item& item = reward.getRewardItem(seat)[item_slot];
                    if (item.item_id_ == -1) {
                        continue;
                    }

                    int inserted_slot = -1;
                    if (item.item_id_ != 0) {
                        inserted_slot = user->getCurCharacInvenW()
                                            ->insertItemIntoInventory(
                                                item,
                                                static_cast<eItemAddReason>(11),
                                                true,
                                                true);
                    } else {
                        const int gained_money = user->getCurCharacInvenW()
                                                     ->gain_money(
                                                         item.get_add_info(),
                                                         static_cast<eMoneyAddReason>(5),
                                                         true,
                                                         0);
                        if (item.get_add_info() > gained_money) {
                            user->SendMoneyFullReason(
                                static_cast<ENUM_MONEY_FULL_REASON>(0),
                                static_cast<unsigned long>(item.get_add_info()),
                                static_cast<unsigned long>(gained_money));
                        }
                        inserted_slot = 0;

                        const int circulation_class =
                            battle_field_.GetDungeon()
                                ->circulation_statistic_class_;
                        if (circulation_class == 1) {
                            GetInstanceCirculationStatistic()
                                ->AddCirculationStatistic(
                                    static_cast<CIRCULATION_STATISTIC_FIELD>(11),
                                    user,
                                    static_cast<unsigned int>(gained_money));
                        } else if (circulation_class == 2) {
                            GetInstanceCirculationStatistic()
                                ->AddCirculationStatistic(
                                    static_cast<CIRCULATION_STATISTIC_FIELD>(12),
                                    user,
                                    static_cast<unsigned int>(gained_money));
                        }
                    }

                    if (inserted_slot < 0) {
                        if ((item.item_id_ == 1251 || item.item_id_ == 1252) &&
                            G_CDataManager()->find_item(item.item_id_)) {
                            SendBloodRewardOverflowMail(user, item);
                        }
                        continue;
                    }

                    PacketGuard packet;
                    packet.put_header(0, 14);
                    packet.put_byte(item.GetItemSpace());
                    packet.put_short(1);
                    user->getCurCharacInvenR()->MakeItemPacket(
                        static_cast<INVEN_TYPE>(GetInvenTypeFromItemSpace(
                            static_cast<ENUM_ITEMSPACE>(item.GetItemSpace()))),
                        inserted_slot,
                        packet);
                    packet.finalize(true);
                    user->Send(packet);
                }

                unsigned int reward_exp =
                    static_cast<unsigned int>(reward.getRewardExp(seat));
                if (reward_exp > 1000000u) {
                    LogManager::logFormat(
                        1,
                        "party.cpp",
                        "bool CParty::onBloodRewardTimer(TIMER_MESSAGE)",
                        0x3bd4,
                        "BLOOD_LOG : EROOR BLOOD CLEAR REWARD EXP!! (%u)\n",
                        reward_exp);
                    reward_exp = 1000000u;
                }

                int gained_sp = 0;
                int gained_sfp = 0;
                if (user->gain_exp_sp(static_cast<int>(reward_exp),
                                      gained_sp,
                                      gained_sfp,
                                      static_cast<eExpAddReason>(2),
                                      0,
                                      false)) {
                    user->SendNotiPacket(
                        static_cast<CUser::eSendTarget>(2),
                        static_cast<ENUM_NOTIPACKET>(2),
                        1);
                }
            }

            battle_field_.setBloodState(static_cast<ENUM_BLOOD_STATE>(5));
            SetEPLPState(2);
            G_TimerQueue()->InsertTimer(
                1,
                GetPartyIndex(),
                TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_3,
                60,
                static_cast<unsigned int>(
                    gen_timer_key(TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_3)),
                TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_3);
            sendInoutConditionDungeon(static_cast<RetryFailReason::T>(
                checkInoutConditionDungeon()));
            return 1;

        case TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_3:
            if (static_cast<unsigned char>(GetEPLPState()) != 2u ||
                static_cast<unsigned char>(GetSelectedEPLPCmd()) == 3u) {
                return 0;
            }
            finishBloodDungeon();
            return 1;

        case TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0:
            if (battle_field_.getBloodState() <= 1 || get_member_count() <= 0) {
                return 0;
            }

            if (static_cast<unsigned char>(check_allmember_die()) == 1u) {
                if (battle_field_.check_grid_clear() == 0 ||
                    !battle_field_.check_end_point() ||
                    reward.isBloodClear() == 0) {
                    beginBloodClearProcess(false);
                    return 1;
                }
            } else if (battle_field_.check_grid_clear() == 0 ||
                       !battle_field_.check_end_point() ||
                       reward.isBloodClear() == 0) {
                return 1;
            }
            beginBloodClearProcess(true);
            break;

        default:
            break;
    }
    return 1;
}

// IDA_EA: 0x85B75D4
// IDA_NAME: _ZN6CParty22beginBloodClearProcessEb
// DEMANGLED: CParty::beginBloodClearProcess(bool)
char CParty::beginBloodClearProcess(bool blood_clear) {
    char result = static_cast<char>(battle_field_.getBloodState() > 2);
    if (result != 0) {
        return result;
    }

    CBattle_Field::CBloodClearRewardData& reward =
        battle_field_.blood_clear_reward();
    unsigned int play_time = static_cast<unsigned int>(reward.getPlayTime());
    int blood_max_round = 0;
    if (blood_clear) {
        blood_max_round = battle_field_.getBloodMaxRound();
        reward.setCurrentBloodRound(blood_max_round);
    } else {
        blood_max_round = reward.getCurrentBloodRound();
    }

    result = static_cast<char>(battle_field_.setBloodState(
        static_cast<ENUM_BLOOD_STATE>(3)) ^ 1);
    if (result != 0) {
        return result;
    }

    CDungeon* const dungeon = battle_field_.GetDungeon();
    if (blood_clear && dungeon->circulation_statistic_class_ == 2) {
        const short next_round_difficulty = static_cast<short>(
            battle_field_.getNextRoundDifficulty(
                static_cast<short>(battle_field_.getBloodMaxRound() - 1)));
        reward.incUltimateRoundClearPoint(next_round_difficulty);
    }

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = get_user(seat);
        if (!user || static_cast<unsigned char>(checkValidUser(seat)) != 1u) {
            continue;
        }

        user->setChattingMessageCount(0);
        onBloodClear(blood_clear, seat);
        user->set_charac_party_bonus_exp(0);
        user->set_charac_member_bonus_exp(0);
        user->set_charac_fatigue_buf_bonus_exp(0);
        user->set_charac_seria_buf_bonus_exp(0);

        checkBloodRenewMyRecord(seat,
                                static_cast<int>(play_time),
                                blood_max_round,
                                battle_field_.get_dungeon_index());

        int reward_item_count =
            10 * blood_max_round / battle_field_.getBloodMaxRound();
        if (reward_item_count == 0) {
            reward_item_count = 1;
        }
        const unsigned int reward_probability_scale =
            static_cast<unsigned int>(std::min(10, blood_max_round));
        generateBloodRewardItem(seat,
                                reward_item_count,
                                static_cast<unsigned int>(
                                    user->get_charac_level()),
                                reward_probability_scale,
                                blood_clear);

        unsigned int base_exp = 0;
        G_CDataManager()->get_mob_reward(user->get_charac_level(), base_exp);
        QuickParty::RewardData_Param random_parameter(this);
        QuickParty::RewardData_Result random_result;
        random_result.raw_00 = static_cast<int>(base_exp);
        set_random_reward(static_cast<QuickParty::RandomBuffType>(0),
                          random_parameter,
                          random_result);
        base_exp = static_cast<unsigned int>(random_result.raw_00);

        const std::vector<float>& inout_weights =
            G_CDataManager()->get_blood_dungeon()
                .reward_exp_inout_count_weight;
        CCharacBloodDungeon* const blood_dungeon =
            static_cast<CCharacBloodDungeon*>(user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(1)));
        const int inout_count = std::min(
            static_cast<int>(inout_weights.size()) - 1,
            blood_dungeon->GetEnterCount());
        generateBloodRewardExp(seat,
                               blood_max_round,
                               base_exp,
                               static_cast<short>(inout_count));

        unsigned int reward_exp =
            static_cast<unsigned int>(reward.getRewardExp(seat));
        if (reward_exp > 1000000u) {
            LogManager::logFormat(
                1,
                "party.cpp",
                "void CParty::beginBloodClearProcess(bool)",
                0x3c79,
                "BLOOD_LOG : EROOR BLOOD CLEAR REWARD EXP(%u)\n",
                reward_exp);
            reward_exp = 1000000u;
            reward.setRewardExp(seat, reward_exp);
        }

        if (blood_max_round == 0) {
            play_time = 0;
            blood_max_round = 0;
        }

        int best_time = user->getBloodBestTimeRecord();
        int best_round = user->getBloodBestRoundRecord();
        if (best_round == -1) {
            best_time = static_cast<int>(play_time);
            best_round = blood_max_round;
        }

        PacketGuard packet;
        packet.put_header(0, 208);
        packet.put_int(static_cast<int>(play_time));
        packet.put_int(blood_max_round);
        packet.put_int(best_time);
        packet.put_int(best_round);
        packet.put_int(battle_field_.getBloodMaxRound());
        packet.put_int(reward.getRewardExp(seat));
        packet.finalize(true);
        user->Send(packet);

        G_TimerQueue()->InsertTimer(
            1,
            GetPartyIndex(),
            TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_1,
            8,
            static_cast<unsigned int>(
                gen_timer_key(TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_1)),
            TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_1);
    }
    return result;
}

// IDA_EA: 0x85B7C9E
// IDA_NAME: _ZN6CParty12onBloodClearEbi
// DEMANGLED: CParty::onBloodClear(bool,int)
int CParty::onBloodClear(bool blood_clear, int member_slot) {
    CUser* const user = get_user(member_slot);
    if (!blood_clear) {
        return OnFailedDungeonClear(user);
    }

    user->history_log().MapClear(
        game_result_set_.raw_4a,
        static_cast<int>(GetMapPlayingTime() / 1000u),
        0);
    if (battle_field_.GetDungeon()->circulation_statistic_class_ == 1) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            user,
            static_cast<APSystem::_ActionGroupIndex>(0),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
    }
    return OnSuccessedDungeonClear(user);
}

// IDA_EA: 0x85B7EA2
// IDA_NAME: _ZN6CParty22generateBloodRewardExpEiijs
// DEMANGLED: CParty::generateBloodRewardExp(int,int,unsigned int,short)
int CParty::generateBloodRewardExp(int member_slot,
                                   int blood_max_round,
                                   unsigned int base_exp,
                                   short inout_count) {
    CUser* const user = get_user(member_slot);
    const stBloodDungeon_t& blood = G_CDataManager()->get_blood_dungeon();

    const float exp_weight = blood.reward_exp_weight[blood_max_round];
    const float round_base_exp = static_cast<float>(
        base_exp * static_cast<unsigned int>(blood_max_round));
    float total_exp = 0.0f;
    if (battle_field_.GetDungeon()->circulation_statistic_class_ == 2) {
        total_exp = exp_weight * round_base_exp * 10.0f;
    } else {
        const float inout_weight = blood.reward_exp_inout_count_weight[
            static_cast<size_t>(inout_count)];
        total_exp = exp_weight * round_base_exp * inout_weight;
    }
    unsigned int reward_exp = static_cast<unsigned int>(total_exp);
    if (reward_exp > 1000000u) {
        const float inout_weight = blood.reward_exp_inout_count_weight[
            static_cast<size_t>(inout_count)];
        LogManager::logFormat(
            1,
            "party.cpp",
            "void CParty::generateBloodRewardExp(int, int, unsigned int, short int)",
            0x3cec,
            "BLOOD_LOG : EROOR BLOOD CLEAR REWARD EXP(%u), BASE EXP (%u), "
            "EXP_WEIGHT(%f), EXP_INOUT_WEIGHT(%f), LAST_ROUND(%d), "
            "INOUT_CNT(%d)\n",
            reward_exp,
            base_exp,
            exp_weight,
            inout_weight,
            blood_max_round,
            inout_count);
    }

    stClearRewardBonusExp_t bonus_exp;
    stClearRewardExpParameter_t parameter;
    parameter.avatar_bonus_rate = static_cast<float>(getAvatarBonus());
    parameter.creature_bonus_rate = static_cast<float>(getCreatureBonus(user));

    CEventBase* const pc_room_event =
        GlobalData::s_event_manager->GetRepeatEvent(8);
    if (pc_room_event->IsEventing(user) != 0) {
        CPCRoomBurningEvent* const event =
            static_cast<CPCRoomBurningEvent*>(pc_room_event);
        parameter.pc_room_burning_rate =
            static_cast<float>(event->GetExpFactor() / 100);
    }

    CEventBase* const double_exp_event =
        GlobalData::s_event_manager->GetRepeatEvent(3);
    if (double_exp_event->IsEventing(0) != 0) {
        CExpDoubleEvent* const event =
            static_cast<CExpDoubleEvent*>(double_exp_event);
        parameter.double_exp_event_rate =
            static_cast<float>(event->GetExpFactor()) / 100.0f;
    }

    CEventBase* const burning_fatigue_event =
        GlobalData::s_event_manager->GetRepeatEvent(15);
    if (burning_fatigue_event->IsEventing(0) != 0) {
        CBurningFatigueEvent* const event =
            static_cast<CBurningFatigueEvent*>(burning_fatigue_event);
        parameter.burning_fatigue_bonus_percent = static_cast<uint32_t>(
            event->getBonusExpRate(
                static_cast<uint16_t>(user->getCurCharacUsedFatigue())));
    }

    channel_script_t* const channels = G_CDataManager()->GetChannelScript();
    const float channel_bonus = channels->getBonusRate(
        static_cast<uint8_t>(G_CEnvironment()->get_server_group()),
        static_cast<uint32_t>(G_CEnvironment()->get_channel_no()),
        static_cast<uint32_t>(battle_field_.GetDungeon()->get_index()));
    parameter.channel_bonus_rate = channel_bonus;

    if (user->IsHavePremiumAdvantage()) {
        parameter.premium_advantage_rate =
            static_cast<float>(
                user->GetPremiumInfo()->GetAdvantageExpRate()) / 100.0f;
    }
    parameter.server_event_rate =
        static_cast<float>(CServerEvent::GetExpRate()) / 100.0f;
    parameter.character_manage_rate = static_cast<float>(CalcurateManageBonus(
        ManageBonusType::Exp, user, reward_exp));

    const unsigned int total_reward_exp =
        getClearRewardBonusExp(user, reward_exp, parameter, bonus_exp);
    if (total_reward_exp > 1000000u) {
        LogManager::logFormat(
            1,
            "party.cpp",
            "void CParty::generateBloodRewardExp(int, int, unsigned int, short int)",
            0x3d12,
            "BLOOD_LOG : EROOR BLOOD CLEAR REWARD EXP!! (%u)\n",
            total_reward_exp);
        LogManager::logFormat(
            1,
            "party.cpp",
            "void CParty::generateBloodRewardExp(int, int, unsigned int, short int)",
            0x3d1c,
            "BLOOD_LOG : EXP PARAMETER : %f, %f, %f, %f, %f, %f, %f, %d\n",
            parameter.avatar_bonus_rate,
            parameter.creature_bonus_rate,
            parameter.pc_room_burning_rate,
            parameter.double_exp_event_rate,
            parameter.channel_bonus_rate,
            parameter.premium_advantage_rate,
            parameter.server_event_rate,
            parameter.burning_fatigue_bonus_percent);
    }

    return static_cast<int>(reinterpret_cast<intptr_t>(
        battle_field_.blood_clear_reward().setRewardExp(member_slot,
                                                         total_reward_exp)));
}

// IDA_EA: 0x85B840E
// IDA_NAME: _ZN6CParty23generateBloodRewardItemEiijjb
// DEMANGLED: CParty::generateBloodRewardItem(int,int,unsigned int,unsigned int,bool)
void CParty::generateBloodRewardItem(int member_slot,
                                     int item_count,
                                     unsigned int character_level,
                                     unsigned int reward_probability_scale,
                                     bool blood_clear) {
    CBattle_Field::CBloodClearRewardData& reward =
        battle_field_.blood_clear_reward();
    CUser* const user = get_user(member_slot);
    reward.setRewardItem(member_slot, item_count);

    const stBloodDungeon_t& blood = G_CDataManager()->get_blood_dungeon();
    WongWork::CItemGenerator* const generator =
        G_CDataManager()->getItemGenerator()->getGenerator(
            WongWork::CItemGeneratorMgr::kClearReward);
    for (int item_slot = 0; item_slot < item_count; ++item_slot) {
        WongWork::stGenerateRefData_t reference;
        WongWork::stGenerateResult_t result;
        GetLuckPoint(&result.luck_point, false);

        const unsigned int random_value = get_rand_int(1000);
        const unsigned int gold_probability =
            reward_probability_scale *
            static_cast<unsigned int>(blood.reward_item_prob_cumulative[0]);
        const unsigned int item_probability =
            reward_probability_scale *
            static_cast<unsigned int>(blood.reward_item_prob_cumulative[1]);
        if (random_value < gold_probability) {
            reference.generator_data.gold_reward.character_level =
                static_cast<uint8_t>(character_level);
            reference.generator_data.gold_reward.dungeon_difficulty = 0;
            reference.generator_data.gold_reward.party_member_count = 1;
            reference.generator_data.gold_reward.exp_bonus_context = 1;
            reference.generator_data.gold_reward.gold_weight_08 = 1;
            reference.generator_data.gold_reward.gold_weight_0a = 1;
            reference.generator_data.gold_reward.gold_reserved_0c = 1;
            reference.generator_data.gold_reward.gold_weight_0e = 1;
            generator->generateSpecificItem(reference, result);

            Inven_Item generated = result.items.front();
            reward.setRewarItemCopy(generated, member_slot, item_slot);
            QuickParty::RewardData_Param basic_parameter(this);
            QuickParty::RewardData_Result basic_result;
            basic_result.raw_00 = generated.get_add_info();
            set_basic_reward(static_cast<QuickParty::BasicRewardType>(11),
                             basic_parameter,
                             basic_result);
            generated.set_add_info(basic_result.raw_00);
            reward.setRewardItemAddInfo(
                blood.reward_gold_weight, member_slot, item_slot);
        } else if (random_value < item_probability) {
            reference.generator_data.clear_reward.character_level_or_world_area =
                static_cast<uint8_t>(character_level);
            reference.generator_data.clear_reward.dungeon_difficulty_or_world_index =
                0;
            reference.generator_data.clear_reward.item_probability_scale =
                1.0f;
            reference.generator_data.clear_reward.clear_reward_drop_probability =
                0;
            reference.generator_data.clear_reward.dungeon = battle_field_.GetDungeon();
            generator->generateRandomItem(reference, result);
            if (!result.items.empty()) {
                const Inven_Item& generated = result.items.front();
                reward.setRewarItemCopy(generated, member_slot, item_slot);
                CItem* const prototype = G_CDataManager()->find_item(
                    generated.item_id_);
                if (prototype) {
                    CLuckPoint::UseLuckPoint(user,
                                             generated.item_category_,
                                             prototype->get_rarity(),
                                             prototype->get_grade());
                }
            }
        }
    }

    if (blood_clear &&
        battle_field_.GetDungeon()->circulation_statistic_class_ == 2) {
        const int ultimate_item_id = reward.getUltimateRewardItem();
        if (G_CDataManager()->find_item(ultimate_item_id)) {
            Inven_Item ultimate_reward;
            ultimate_reward.item_id_ = ultimate_item_id;
            G_CDataManager()->get_item_list()->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, ultimate_reward, 1);
            reward.setRewarItemCopy(
                ultimate_reward, member_slot, item_count - 1);
        }
    }
}

// IDA_EA: 0x85B8BAE
// IDA_NAME: _ZN6CParty23checkBloodRenewMyRecordEiiii
// DEMANGLED: CParty::checkBloodRenewMyRecord(int,int,int,int)
int CParty::checkBloodRenewMyRecord(int member_slot,
                                    int play_time,
                                    int blood_max_round,
                                    int dungeon_index) {
    return get_user(member_slot)->setBloodBestRecord(
        static_cast<unsigned int>(play_time),
        blood_max_round,
        dungeon_index,
        false);
}
