#include "old_game/social/party/party.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/history_log_abi.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/statistic_proxy_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/dungeon/instance/luck_point_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

void ResetRewardPair(Inven_Item& reward) {
    reward.reset();
    (&reward)[1].reset();
}

void SendOverflowRewardMail(CUser* user,
                            const Inven_Item& reward,
                            const char* title_key) {
    char title[21];
    char text[256];
    std::strncpy(title,
                 g_scriptStringManager_.findString(4, title_key, 0),
                 0x14u);
    std::strncpy(text,
                 g_scriptStringManager_.findString(4, "game_server_msg_104", 0),
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

// IDA_EA: 0x85BD7AE
// IDA_NAME: _ZN6CParty21MakeQuestRewardPacketEP11PacketGuardP5CUser
// DEMANGLED: CParty::MakeQuestRewardPacket(PacketGuard *,CUser *)
char CParty::MakeQuestRewardPacket(PacketGuard* packet, CUser* user) {
    if (!packet || !user) {
        return 0;
    }

    UserQuest* const quests = user->getCurCharacQuestRefR();
    if (!quests) {
        return 0;
    }

    int count_index = packet->get_index();
    packet->put_byte(0);
    int reward_count = 0;
    const int dungeon_index = battle_field_.get_dungeon_index();

    // The old packet builder walks all twenty active quest slots and requires
    // both slot values to be nonzero before it evaluates a clear-item reward.
    for (int slot = 0; slot <= 19; ++slot) {
        const uint32_t quest_index = quests->play_quest[slot];
        if (quest_index == 0 || quests->play_quest_trigger[slot] == 0) {
            continue;
        }

        Quest* const quest = G_CDataManager()->find_quest(
            static_cast<int>(quest_index));
        if (!quest ||
            quest->check_clear_item(
                dungeon_index,
                static_cast<int>(quests->authen_data.clear_count)) != 1) {
            continue;
        }

        const std::vector<std::pair<int, int> > items =
            quest->get_clear_itemlist(dungeon_index);
        for (std::vector<std::pair<int, int> >::const_iterator item =
                 items.begin();
             item != items.end();
             ++item) {
            Inven_Item reward;
            reward.reset();
            reward.item_id_ = item->first;
            G_CDataManager()->get_item_list()->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, reward, item->second);

            const int inserted_slot =
                user->getCurCharacInvenW()->insertItemIntoInventory(
                    reward, static_cast<eItemAddReason>(11), true, true);
            if (inserted_slot < 0) {
                continue;
            }

            packet->put_short(inserted_slot);
            packet->put_int(reward.item_id_);
            packet->put_int(reward.get_add_info());
            packet->put_short(reward.avatar_select_);
            packet->put_byte(reward.amplify_option_.getAbilityType());
            packet->put_short(reward.amplify_option_.getAbilityValue());
            ++reward_count;

            if (reward.item_category_ == 1) {
                CItem* const prototype = G_CDataManager()->find_item(
                    reward.item_id_);
                if (prototype) {
                    CLuckPoint::UseLuckPoint(user,
                                             reward.item_category_,
                                             prototype->get_rarity(),
                                             prototype->get_grade());
                }
            }
        }
    }

    if (reward_count != 0) {
        packet->put_byte(count_index, reward_count);
    }
    return static_cast<char>(reward_count != 0);
}

// IDA_EA: 0x85BCB40
// IDA_NAME: _ZN6CParty20MakeTournamentRewardER24CTournamentDungeonReward
// DEMANGLED: CParty::MakeTournamentReward(CTournamentDungeonReward &)
void CParty::MakeTournamentReward(CTournamentDungeonReward& reward) {
    const int tournament_dungeon_index =
        battle_field_.get_dungeon_index();
    CDungeon* const dungeon =
        G_CDataManager()->find_dungeon(tournament_dungeon_index);
    // 140828 owns tournament rewards per DGN.  In particular, dungeon 122
    // must consume blackdragon.dgn's item/EXP/gold tables; it must never fall
    // back to the Yellow/Blue (120/121) profiles when its data is missing.
    if (!dungeon || dungeon->get_index() != tournament_dungeon_index) {
        LogManager::logFormat(
            1,
            "party.cpp",
            "void CParty::MakeTournamentReward(CTournamentDungeonReward&)",
            0x4540,
            "Unknown Dungeon %d",
            tournament_dungeon_index);
        return;
    }

    const bool tournament_victory = tournament_victory_ != 0;
    const int member_count = get_member_count();
    reward.MakeReward(&dungeon->tournament_rewards_,
                      &dungeon->tournament_reward_items_,
                      battle_field_.GetTournamentCurRound(),
                      tournament_victory);
    if (member_count <= 0) {
        LogManager::logFormat(
            1,
            "party.cpp",
            "void CParty::MakeTournamentReward(CTournamentDungeonReward&)",
            0x4545,
            "No Member[%d] in party",
            member_count);
        return;
    }

    WongWork::stGenerateRefData_t reference;
    // CClearReward::generateSpecificItem interprets the final 16-byte union
    // through stGenerateRefGoldReward_t.  The old reconstruction populated
    // the clear-item view instead, leaving the tournament gold weights at
    // zero (or pointer fragments from clear_reward.dungeon).
    reference.generator_data.gold_reward.character_level =
        static_cast<uint8_t>(dungeon->get_standard_level());
    reference.generator_data.gold_reward.dungeon_difficulty =
        static_cast<uint8_t>(battle_field_.get_dungeon_diff());
    reference.generator_data.gold_reward.party_member_count =
        static_cast<uint8_t>(member_count);
    reference.generator_data.gold_reward.exp_bonus_context = 1;
    reference.generator_data.gold_reward.gold_weight_08 =
        battle_data_.clear_reward_gold_weight_08();
    reference.generator_data.gold_reward.gold_weight_0a =
        battle_data_.clear_reward_gold_weight_0a();
    reference.generator_data.gold_reward.gold_reserved_0c = 0;
    reference.generator_data.gold_reward.gold_weight_0e =
        battle_data_.clear_reward_gold_weight_0e();

    WongWork::stGenerateResult_t generated;
    WongWork::CItemGenerator* const generator =
        G_CDataManager()->getItemGenerator()->getGenerator(
            WongWork::CItemGeneratorMgr::kClearReward);
    if (generator) {
        generator->generateSpecificItem(reference, generated);
    }

    float gold_scale = dungeon->tournament_reward_weight_;
    if (gold_scale < 0.0f) {
        gold_scale = 1.0f;
    }
    reward.SetRewardGold(generated, gold_scale);

    int reward_round = battle_field_.GetTournamentCurRound();
    if (!tournament_victory) {
        --reward_round;
    }
    if (reward_round < 0) {
        reward_round = 0;
    }

    uint32_t gained_exp = 0;
    const std::map<uint8_t, uint32_t>::const_iterator exp_rate =
        dungeon->tower_reward_rates_.find(static_cast<uint8_t>(reward_round));
    if (exp_rate != dungeon->tower_reward_rates_.end()) {
        gained_exp = exp_rate->second;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = get_user(seat);
        if (!member) {
            continue;
        }

        member->setChattingMessageCount(0);
        member->calcHelpAbuseRatio(gained_exp, isHelpAbuseParty(member) != 0);
        member->reset_level_before_dungeon();

        int gained_sp = 0;
        int gained_sfp = 0;
        if (member->gain_exp_sp(static_cast<int>(gained_exp),
                                gained_sp,
                                gained_sfp,
                                static_cast<eExpAddReason>(2),
                                0,
                                false)) {
            PacketGuard basic_info;
            basic_info.put_header(0, 2);
            basic_info.put_byte(1);
            basic_info.put_short(1);
            member->make_basic_info(basic_info, 1);
            basic_info.finalize(true);
            member->Send(basic_info);
        }

        PacketGuard tournament_packet;
        tournament_packet.put_header(0, 388);
        tournament_packet.put_byte(reward_round);
        tournament_packet.put_int(static_cast<int>(gained_exp));
        if (tournament_victory) {
            MakeQuestRewardPacket(&tournament_packet, member);
        } else {
            tournament_packet.put_byte(0);
        }
        reward.MakeRewardPacket(&tournament_packet, tournament_victory);
        tournament_packet.finalize(true);
        member->Send(tournament_packet);
    }

    SetTournamentDungeonClearState(1);
    InsertTournamentDungeonTimer(
        TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_NOTI_REWARD);
}

// IDA_EA: 0x85ACA0A
// IDA_NAME: _ZN6CParty28sendPremiumGoldCardStatisticEiii
// DEMANGLED: CParty::sendPremiumGoldCardStatistic(int,int,int)
void CParty::sendPremiumGoldCardStatistic(int standard_level,
                                          int member_count,
                                          int selected_card_count) {
    Packet_Goldcard_Event_Statistic_GTS packet;
    packet.statistic_0a.byte_00 = static_cast<uint8_t>(standard_level);
    packet.statistic_0a.dword_01 = static_cast<uint32_t>(member_count);
    packet.statistic_0a.dword_05 = static_cast<uint32_t>(selected_card_count);

    CStatisticServerProxy* const proxy =
        GlobalData::s_statistic_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(0));
    proxy->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}

// IDA_EA: 0x85AAB84
// IDA_NAME: _ZN6CParty16_putItemIntoUserEP5CUserP10Inven_ItemR11PacketGuard
// DEMANGLED: CParty::_putItemIntoUser(CUser *,Inven_Item *,PacketGuard &)
int CParty::_putItemIntoUser(CUser* user,
                             Inven_Item* reward_pair,
                             PacketGuard& packet) {
    int record_count = 0;
    const int initial_item_space = 0;

    packet.clear();
    packet.put_header(0, 14);
    packet.put_byte(0);
    int count_index = packet.get_index();
    packet.put_short(0);

    Inven_Item& money_reward = reward_pair[0];
    if (money_reward.get_add_info() > 0) {
        const int gained_money = user->getCurCharacInvenW()->gain_money(
            money_reward.get_add_info(), static_cast<eMoneyAddReason>(5), true, 0);
        if (money_reward.get_add_info() > gained_money) {
            user->SendMoneyFullReason(static_cast<ENUM_MONEY_FULL_REASON>(0),
                                      static_cast<unsigned long>(
                                          money_reward.get_add_info()),
                                      static_cast<unsigned long>(gained_money));
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(3),
            user,
            static_cast<unsigned int>(gained_money));

        CDungeon* const dungeon = battle_field().GetDungeon();
        if (dungeon) {
            const CIRCULATION_STATISTIC_FIELD statistic_field =
                dungeon->circulation_statistic_class_
                    ? static_cast<CIRCULATION_STATISTIC_FIELD>(15)
                    : static_cast<CIRCULATION_STATISTIC_FIELD>(1);
            GetInstanceCirculationStatistic()->AddCirculationStatistic(
                statistic_field, user, static_cast<unsigned int>(gained_money));
        }
        user->saveTaxMoneyForUpperMember(money_reward.get_add_info());

        packet.put_short(0);
        packet.put_int(0);
        packet.put_int(user->getCurCharacInvenR()->get_money());
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_byte(0);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_byte(0);
        packet.put_packet(g_emptySlot);
        ++record_count;
    }

    Inven_Item& item_reward = reward_pair[1];
    if (initial_item_space != item_reward.GetItemSpace()) {
        if (record_count > 0) {
            packet.put_short(count_index, record_count);
            packet.finalize(true);
            user->Send(packet);
        }
        record_count = 0;
        packet.clear();
        packet.put_header(0, 14);
        packet.put_byte(item_reward.GetItemSpace());
        count_index = packet.get_index();
        packet.put_short(record_count);
    }

    const int inserted_slot = user->getCurCharacInvenW()->insertItemIntoInventory(
        item_reward, static_cast<eItemAddReason>(11), true, true);
    CDungeon* const dungeon = battle_field().GetDungeon();
    const bool tournament_dungeon = dungeon && dungeon->isTournamentDungeon();

    if (inserted_slot <= 0) {
        CEventBase* const overflow_event =
            GlobalData::s_event_manager->GetRepeatEvent(65);
        if (overflow_event->IsEventing(0) && item_reward.getKey() != 0 &&
            item_reward.getKey() != -1 && !tournament_dungeon) {
            if (G_CDataManager()->find_item(item_reward.getKey())) {
                SendOverflowRewardMail(user, item_reward, "game_server_msg_155");
            }
        } else if (item_reward.getKey() != 0 && item_reward.getKey() != -1 &&
                   tournament_dungeon &&
                   G_CDataManager()->find_item(item_reward.getKey())) {
            SendOverflowRewardMail(user, item_reward, "game_server_msg_250");
        }
    } else {
        Inven_Item* const inserted_item = user->getCurCharacInvenR()->GetInvenRef(
            item_reward.GetInventoryType(), inserted_slot);
        const int32_t item_id = inserted_item->item_id_;
        const uint16_t avatar_select = inserted_item->avatar_select_;
        const uint8_t seal_or_state = inserted_item->seal_or_state_;
        const int32_t extend_info = inserted_item->extend_info_;
        const stAmplifyOption_t amplify_option =
            inserted_item->amplify_option_;
        packet.put_short(inserted_slot);
        packet.put_int(item_id);
        packet.put_int(inserted_item->get_add_info());
        packet.put_byte(inserted_item->GetItemAttr());
        packet.put_short(avatar_select);
        packet.put_byte(seal_or_state);
        packet.put_int(extend_info);
        packet.put_byte(amplify_option.getAbilityType());
        packet.put_short(amplify_option.getAbilityValue());
        if (inserted_item->isAvatarItemType()) {
            const WongWork::CAvatarItemMgr* const avatar_item_mgr =
                user->getCurCharacInvenR()->GetAvatarItemMgrR();
            packet.put_int(30);
            packet.put_binary(reinterpret_cast<const char*>(
                                  avatar_item_mgr->getJewelSocketData(
                                      inserted_item->get_add_info())),
                              30);
            const stAvatarExpansionInfo_t* const expansion =
                avatar_item_mgr->GetExpansionInfo(inserted_item->get_add_info());
            packet.put_int(4);
            if (expansion) {
                packet.put_binary(reinterpret_cast<const char*>(expansion), 4);
            } else {
                stAvatarExpansionInfo_t empty_expansion;
                empty_expansion.init();
                packet.put_binary(
                    reinterpret_cast<const char*>(&empty_expansion), 4);
            }
        }
        packet.put_byte(0);
        packet.put_packet(*inserted_item);
        ++record_count;

        CItem* const item_prototype = G_CDataManager()->find_item(item_reward.getKey());
        if (item_prototype) {
            int sell_price = 0;
            G_Store()->GetSellItemPrice(
                item_reward,
                item_prototype,
                static_cast<int16_t>(item_reward.add_info_),
                false,
                sell_price);
            GetInstanceValueStatistic()->AddValueStatistic(
                static_cast<VALUE_STATISTIC_FIELD>(4),
                user,
                static_cast<unsigned int>(sell_price));
        }
        if (money_reward.item_category_ == 1 && item_prototype) {
            CLuckPoint::UseLuckPoint(user,
                                     money_reward.item_category_,
                                     item_prototype->get_rarity(),
                                     item_prototype->get_grade());
        }
    }

    if (record_count > 0) {
        packet.put_short(count_index, record_count);
        packet.finalize(true);
        user->Send(packet);
    }
    return record_count;
}

// IDA_EA: 0x85AD0BE
// IDA_NAME: _ZN6CParty19_makeRewardItemInfoEPK5CUserP10Inven_ItemR11PacketGuard
// DEMANGLED: CParty::_makeRewardItemInfo(CUser const*,Inven_Item *,PacketGuard &)
int CParty::_makeRewardItemInfo(const CUser* user,
                                Inven_Item* reward_pair,
                                PacketGuard& packet) {
    Inven_Item& money_reward = reward_pair[0];
    Inven_Item& item_reward = reward_pair[1];

    // The client representation distinguishes a missing item (-1 in the
    // generator result) from an explicitly empty card (0).  Original code
    // normalizes the former before both the insertion attempt and packet
    // serialization.
    if (item_reward.getKey() == -1) {
        item_reward.item_id_ = 0;
        item_reward.set_add_info(0);
    }

    if (item_reward.getKey() != 0) {
        CInventory* const inventory = user->getCurCharacInvenR();
        if (inventory->tryInsertItemIntoInventory(item_reward) < 0) {
            packet.put_byte(1);
            item_reward.item_id_ = 0;
            item_reward.set_add_info(0);
        } else {
            packet.put_byte(2);
        }
    } else {
        packet.put_byte(1);
    }

    packet.put_int(0);
    packet.put_int(money_reward.get_add_info());
    if (item_reward.getKey() != 0) {
        packet.put_int(item_reward.getKey());
        return packet.put_int(item_reward.get_add_info());
    }
    return 0;
}

// IDA_EA: 0x85AB736
// IDA_NAME: _ZN6CParty13dungeon_clearEv
// DEMANGLED: CParty::dungeon_clear(void)
void CParty::dungeon_clear() {
    SaveCoin(static_cast<eCoinSubReason>(0));

    PacketGuard packet;
    const int member_count = get_member_count();
    int selected_card_count = 0;

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }

        CUser* const user = get_user(seat);
        int reward_record_count = 0;
        reward_record_count += _putItemIntoUser(
            user, &battle_data().normal_reward_item(seat, 0), packet);
        reward_record_count += _putItemIntoUser(
            user, &battle_data().special_reward_item(seat, 0), packet);

        if (static_cast<unsigned char>(HaveAlreadySelectedCard(
                static_cast<char>(seat),
                static_cast<eClearRewardCardType_t>(1))) == 1u) {
            Inven_Item* const event_reward =
                &battle_data().event_reward_item(seat, 0);
            CItem* const event_item =
                G_CDataManager()->find_item(event_reward[1].getKey());
            if (event_item) {
                int sell_price = 0;
                G_Store()->GetSellItemPrice(
                    event_reward[1],
                    event_item,
                    static_cast<int16_t>(event_reward[1].add_info_),
                    false,
                    sell_price);
                GetInstanceValueStatistic()->AddValueStatistic(
                    static_cast<VALUE_STATISTIC_FIELD>(5),
                    user,
                    static_cast<unsigned int>(sell_price));
            }

            reward_record_count += _putItemIntoUser(user, event_reward, packet);
            CEventBase* const duplicate_card_event =
                GlobalData::s_event_manager->GetRepeatEvent(64);
            if ((duplicate_card_event->IsEventing(0) || IsReturnUserParty() ||
                 IsEventCharacParty()) &&
                static_cast<unsigned char>(IsPremiumGoldCardParty()) != 1u) {
                reward_record_count += _putItemIntoUser(
                    user, event_reward, packet);
            }
            ++selected_card_count;
        }

        ResetRewardPair(battle_data().normal_reward_item(seat, 0));
        ResetRewardPair(battle_data().special_reward_item(seat, 0));
        ResetRewardPair(battle_data().event_reward_item(seat, 0));
        battle_data().ResetEventDungeonClearPoint();
        HistoryLog::WriteDunClear(user->history_stream());
        OnSuccessedDungeonClear(user);

        // The old body accumulates this return value for its local packet
        // accounting; the actual emission occurs in _putItemIntoUser.
        static_cast<void>(reward_record_count);
    }

    SetEPLPState(2);
    int return_to_village_delay = 60;
    if (static_cast<unsigned char>(secret_shop_data_.IsOpen()) != 0u) {
        return_to_village_delay =
            60 * G_CDataManager()->get_secret_shop_eplp_wait_time();
    }
    const unsigned int timer_key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_EPLP_COMMAND_WAIT));
    G_TimerQueue()->InsertTimer(1,
                                GetPartyIndex(),
                                TIMER_MESSAGE_EPLP_COMMAND_WAIT,
                                return_to_village_delay,
                                timer_key,
                                0);
    set_state(1);
    sendInoutConditionDungeon(
        static_cast<RetryFailReason::T>(checkInoutConditionDungeon()));

    if (static_cast<unsigned char>(IsPremiumGoldCardParty()) != 0u) {
        CDungeon* const dungeon = battle_field().GetDungeon();
        // The raw 0x85AB736 call is unconditional in the premium branch.
        // Preserve that contract rather than substituting a default level.
        const int standard_level = dungeon->get_standard_level();
        sendPremiumGoldCardStatistic(standard_level,
                                     member_count,
                                     selected_card_count);
    }
    CheckHackAverageHitCount();
    ClearMapHitCount();
    secu().dungeonClear();
    champion_break::FinishDungeonRun(this);
}

// IDA_EA: 0x85BD236
// IDA_NAME: _ZN6CParty22TournamentDungeonClearEb
// DEMANGLED: CParty::TournamentDungeonClear(bool)
void CParty::TournamentDungeonClear(bool tournament_victory) {
    CDungeon* const dungeon = battle_field_.GetDungeon();
    if (!dungeon) {
        LogManager::logFormat(1,
                              "party.cpp",
                              "void CParty::TournamentDungeonClear(bool)",
                              0x45bc,
                              "Tournament Dungeon is NULL");
        return;
    }

    if (!dungeon->isTournamentDungeon()) {
        LogManager::logFormat(1,
                              "party.cpp",
                              "void CParty::TournamentDungeonClear(bool)",
                              0x45c2,
                              "Dungeon(%d) is can not call function",
                              battle_field_.get_dungeon_index());
        return;
    }

    SaveCoin(static_cast<eCoinSubReason>(0));

    PacketGuard packet;
    CTournamentDungeonReward& reward = battle_field_.tournament_reward();
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }

        CUser* const user = get_user(seat);
        Inven_Item reward_items[2];
        if (reward.GetItemFromCard(ENUM_TOURNAMENT_REWARD_CARD_TYPE_0,
                                   seat,
                                   reward_items,
                                   2u) != 0) {
            _putItemIntoUser(user, reward_items, packet);
        }

        if (tournament_victory) {
            ResetRewardPair(*reward_items);
            if (reward.GetItemFromCard(ENUM_TOURNAMENT_REWARD_CARD_TYPE_1,
                                       seat,
                                       reward_items,
                                       2u) != 0) {
                _putItemIntoUser(user, reward_items, packet);
            }
            HistoryLog::WriteDunClear(user->history_stream());
            OnSuccessedDungeonClear(user);
            if (user && user->IsProperLevelDungeonUser()) {
                user->ProcProperLevelDungeonClear();
            }
        } else {
            OnFailedDungeonClear(user);
        }
    }

    SetEPLPState(2);
    int return_to_village_delay = 60;
    if (static_cast<unsigned char>(secret_shop_data_.IsOpen()) != 0u) {
        return_to_village_delay =
            60 * G_CDataManager()->get_secret_shop_eplp_wait_time();
    }
    const unsigned int timer_key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_EPLP_COMMAND_WAIT));
    G_TimerQueue()->InsertTimer(1,
                                GetPartyIndex(),
                                TIMER_MESSAGE_EPLP_COMMAND_WAIT,
                                return_to_village_delay,
                                timer_key,
                                0);
    set_state(1);
    sendInoutConditionDungeon(
        static_cast<RetryFailReason::T>(checkInoutConditionDungeon()));
    CheckHackAverageHitCount();
    ClearMapHitCount();
    secu_.dungeonClear();
}
