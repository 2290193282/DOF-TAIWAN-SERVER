#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/dungeon/db_get_death_tower_best_record.h"
#include "old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.h"
#include "old_game/dungeon/tower/death_tower_packet_handler_runtime.h"
#include "old_game/dungeon/tower/death_tower_playdata_statistic_runtime.h"
#include "old_game/dungeon/tower/death_tower_value_statistic_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kDeathTowerTimerLoadTimeout = 73;
const int kDeathTowerTimerFinishReward = 74;
const int kDeathTowerTimerDisplayReward = 75;
const int kDeathTowerTimerGrantReward = 76;
const int kDeathTowerTimerEplpTimeout = 77;
const int kDeathTowerTimerEplpSelected = 78;

const int kDeathTowerEplpRequiredItemId = 0x1057;
const unsigned int kDeathTowerTutorialDungeonIndex = 11000u;
const int kDeathTowerMaxRewardCards = 10;

// CPlayData owns a 23-byte start-member record with the same field order as
// the DB/statistics ABI.  Materialize the typed DB value before crossing that
// ABI boundary instead of relying on an unrelated layout cast.
void copyStartMemberInfo(
    const WongWork::CDeathTower::CPlayData::stStartMemberInfo_t* source,
    stDeathTowerRecordMemberInfo_t (&destination)[4]) {
    for (int index = 0; index <= 3; ++index) {
        std::memcpy(destination[index].charac_name,
                    source[index].character_name_,
                    sizeof(destination[index].charac_name));
        destination[index].level = source[index].character_level_;
        destination[index].job = source[index].character_job_;
        destination[index].grow_type = source[index].grow_type_;
    }
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x8462AAE
// IDA_NAME: _ZN8WongWork11CDeathTower7onTimerE13TIMER_MESSAGEj
// DEMANGLED: WongWork::CDeathTower::onTimer(TIMER_MESSAGE,unsigned int)
int CDeathTower::onTimer(TIMER_MESSAGE message, unsigned int timer_key) {
    if (!_checkTimerKey(message, timer_key)) {
        return 0;
    }
    if (!getTowerState()) {
        return 0;
    }

    switch (static_cast<int>(message)) {
    case kDeathTowerTimerLoadTimeout: {
        if (!party_->get_member_count()) {
            _destroy();
            return 1;
        }

        bool removed_unready_member = false;
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const user = party_->get_user(seat);
            if (!user || party_->checkValidUser(seat) != 1 ||
                play_data_.isMemberReady(seat) == 1) {
                continue;
            }

            CPacketHandler packet;
            packet.makeFinishLoading();
            user->Send(packet);
            handleLeaveUser(user);
            removed_unready_member = true;
        }

        const int member_count = party_->get_member_count();
        if (removed_unready_member && member_count > 0) {
            for (int seat = 0; seat <= 3; ++seat) {
                play_data_.setMemberReady(seat, true);
            }
            CPacketHandler packet;
            packet.makeFinishLoading();
            party_->send_to_party(packet);
        } else if (!member_count) {
            _destroy();
        }
        return 1;
    }

    case kDeathTowerTimerDisplayReward: {
        if (getTowerState() <= 2) {
            return 0;
        }

        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const user = party_->get_user(seat);
            if (!user || party_->checkValidUser(seat) != 1) {
                continue;
            }

            CPacketHandler packet;
            packet.makeDisplayReward(
                play_data_.getRewardExp(seat),
                play_data_.getRewardItemCount(0),
                play_data_.getRewardItem(0),
                play_data_.getRewardItemCount(1),
                play_data_.getRewardItem(1),
                play_data_.getRewardItemCount(2),
                play_data_.getRewardItem(2),
                play_data_.getRewardItemCount(3),
                play_data_.getRewardItem(3));
            deathTowerCardStatistic(play_data_.getRewardItemCount(seat),
                                    user,
                                    play_data_.getRewardItem(seat));
            user->Send(packet);
        }

        setTowerState(static_cast<ENUM_TOWER_STATE>(4));
        const int next_timer_key =
            _genTimerKey(static_cast<TIMER_MESSAGE>(kDeathTowerTimerGrantReward));
        G_TimerQueue()->InsertTimer(
            6,
            static_cast<int>(index_),
            kDeathTowerTimerGrantReward,
            5,
            static_cast<unsigned int>(next_timer_key),
            kDeathTowerTimerGrantReward);
        return 1;
    }

    case kDeathTowerTimerGrantReward: {
        if (getTowerState() <= 3) {
            return 0;
        }

        bool eplp_available = true;
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const user = party_->get_user(seat);
            if (!user || party_->checkValidUser(seat) != 1) {
                continue;
            }

            Inven_Item eplp_item;
            if (user->getCurCharacInvenR()->GetInvenData(
                    kDeathTowerEplpRequiredItemId, eplp_item) < 0) {
                eplp_available = false;
                break;
            }
        }

        CPacketHandler eplp_packet;
        eplp_packet.makeDisplayEPLP(eplp_available);
        party_->send_to_party(eplp_packet);

        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const user = party_->get_user(seat);
            if (!user || party_->checkValidUser(seat) != 1) {
                continue;
            }

            const int reward_item_count = play_data_.getRewardItemCount(seat);
            Inven_Item* const reward_items = play_data_.getRewardItem(seat);
            for (int item_index = 0; item_index < reward_item_count;
                 ++item_index) {
                Inven_Item& item = reward_items[item_index];
                if (item.item_id_ == -1) {
                    continue;
                }

                int inventory_slot = -1;
                if (item.item_id_ != 0) {
                    inventory_slot = user->getCurCharacInvenW()
                                         ->insertItemIntoInventory(
                                             item,
                                             static_cast<eItemAddReason>(11),
                                             true,
                                             true);
                } else {
                    const unsigned int requested_money = item.get_add_info();
                    const int received_money =
                        user->getCurCharacInvenW()->gain_money(
                            static_cast<int>(requested_money),
                            static_cast<eMoneyAddReason>(5),
                            true,
                            0);
                    if (requested_money >
                        static_cast<unsigned int>(received_money)) {
                        user->SendMoneyFullReason(
                            static_cast<ENUM_MONEY_FULL_REASON>(0),
                            requested_money,
                            static_cast<unsigned int>(received_money));
                    }

                    const unsigned int circulation_field =
                        static_cast<unsigned int>(dungeon_manager_.getDungeon()
                                                      ->get_index()) ==
                                kDeathTowerTutorialDungeonIndex
                            ? 4u
                            : 5u;
                    GetInstanceCirculationStatistic()->AddCirculationStatistic(
                        static_cast<CIRCULATION_STATISTIC_FIELD>(
                            circulation_field),
                        user,
                        static_cast<unsigned int>(received_money));
                    inventory_slot = 0;
                }

                if (inventory_slot >= 0) {
                    CPacketHandler item_packet;
                    item_packet.makeUpdateItemList(
                        user,
                        static_cast<ENUM_ITEMSPACE>(item.GetItemSpace()),
                        inventory_slot);
                    user->Send(item_packet);
                }
            }

            const int reward_exp = play_data_.getRewardExp(seat);
            user->calcHelpAbuseRatio(static_cast<unsigned int>(reward_exp),
                                     false);
            int gained_sp = 0;
            int gained_sfp = 0;
            if (user->gain_exp_sp(reward_exp,
                                  gained_sp,
                                  gained_sfp,
                                  static_cast<eExpAddReason>(2),
                                  0,
                                  false)) {
                user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                                     static_cast<ENUM_NOTIPACKET>(2),
                                     1);
            }
        }

        setTowerState(static_cast<ENUM_TOWER_STATE>(5));
        party_->SetEPLPState(2);
        const int next_timer_key =
            _genTimerKey(static_cast<TIMER_MESSAGE>(kDeathTowerTimerEplpTimeout));
        G_TimerQueue()->InsertTimer(
            6,
            static_cast<int>(index_),
            kDeathTowerTimerEplpTimeout,
            60,
            static_cast<unsigned int>(next_timer_key),
            kDeathTowerTimerEplpTimeout);
        return 1;
    }

    case kDeathTowerTimerEplpTimeout:
        if (party_->GetEPLPState() != 2 ||
            party_->GetSelectedEPLPCmd() == 3) {
            return 0;
        }
        _onFinishDeathTower();
        return 1;

    case kDeathTowerTimerEplpSelected:
        if (party_->GetEPLPState() != 3) {
            return 0;
        }
        _onFinishDeathTower();
        return 1;

    case kDeathTowerTimerFinishReward:
        if (getTowerState() <= 1) {
            return 0;
        }
        if (party_->get_member_count() <= 0) {
            _destroy();
            return 1;
        }
        if (_checkMemberDie()) {
            _beginTowerClearProcess(false);
        } else if (stage_.getCurrentStage() == dungeon_manager_.getEndStage()) {
            _beginTowerClearProcess(true);
        }
        return 1;

    default:
        return 1;
    }
}

// IDA_EA: 0x8467208
// IDA_NAME: _ZN8WongWork11CDeathTower23_beginTowerClearProcessEb
// DEMANGLED: WongWork::CDeathTower::_beginTowerClearProcess(bool)
void CDeathTower::_beginTowerClearProcess(bool cleared) {
    if (getTowerState() > 2) {
        return;
    }

    const unsigned int start_member_count =
        static_cast<unsigned int>(play_data_.getStartMemberCnt());
    CDeathTowerRanking* const rank_table =
        CDeathTowerRanking::getRankTable(start_member_count);
    _onClear(cleared);

    if (party_->battle_field().instance_dungeon_mode() == 1) {
        setTowerState(static_cast<ENUM_TOWER_STATE>(5));
        party_->SetEPLPState(2);
        const int timer_key =
            _genTimerKey(static_cast<TIMER_MESSAGE>(kDeathTowerTimerEplpTimeout));
        G_TimerQueue()->InsertTimer(
            6,
            static_cast<int>(index_),
            kDeathTowerTimerEplpTimeout,
            60,
            static_cast<unsigned int>(timer_key),
            kDeathTowerTimerEplpTimeout);

        PacketGuard packet;
        packet.put_header(0, 254);
        packet.finalize(true);
        party_->send_to_party(packet);
        return;
    }

    setTowerState(static_cast<ENUM_TOWER_STATE>(3));
    const int timer_key =
        _genTimerKey(static_cast<TIMER_MESSAGE>(kDeathTowerTimerDisplayReward));
    G_TimerQueue()->InsertTimer(
        6,
        static_cast<int>(index_),
        kDeathTowerTimerDisplayReward,
        1,
        static_cast<unsigned int>(timer_key),
        kDeathTowerTimerDisplayReward);

    unsigned int current_stage =
        static_cast<unsigned int>(stage_.getCurrentStage());
    const unsigned int play_time = cleared
                                       ? static_cast<unsigned int>(
                                             play_data_.getPlayTime())
                                       : static_cast<unsigned int>(
                                             play_data_.getLastPlayTime());
    if (!cleared) {
        current_stage = std::max(1u, current_stage - 1u);
    }

    stDeathTowerRecordMemberInfo_t start_member_info[4];
    copyStartMemberInfo(play_data_.getStartMemberInfo(), start_member_info);

    CDungeon* const dungeon = dungeon_manager_.getDungeon();
    const unsigned int dungeon_index =
        static_cast<unsigned int>(dungeon->get_index());
    const unsigned int rank_index = static_cast<unsigned int>(
        CDeathTowerRanking::makeDungeonIdx2TowerIdx(dungeon_index));
    const char non_tutorial_dungeon =
        static_cast<char>(dungeon_index != kDeathTowerTutorialDungeonIndex);

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }

        user->set_charac_party_bonus_exp(0);
        user->set_charac_member_bonus_exp(0);
        user->set_charac_fatigue_buf_bonus_exp(0);
        user->set_charac_seria_buf_bonus_exp(0);

        const unsigned int charac_no =
            static_cast<unsigned int>(user->getCurCharacNo());
        const int renew_my_record = _checkRenewMyRecord(
            dungeon_index,
            start_member_count,
            charac_no,
            current_stage,
            play_time);
        int record_reward_bonus = 0;
        if (cleared) {
            record_reward_bonus = renew_my_record == 1 ? 1 : 0;
            if (_checkRenewTopRecord(dungeon_index,
                                     start_member_count,
                                     current_stage,
                                     play_time)) {
                record_reward_bonus += 2;
            }
        }

        const int virtual_rank =
            rank_table->getVirtualRank(rank_index, current_stage, play_time);

        GetInstanceDeathTowerPlaydataStatistic()->SendDeathTowerPlayInfo(
            seat,
            non_tutorial_dungeon,
            start_member_info,
            current_stage,
            start_member_count);
        GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
            static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(1),
            non_tutorial_dungeon,
            static_cast<char>(user->get_charac_level()),
            current_stage);
        GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
            static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(0),
            non_tutorial_dungeon,
            static_cast<char>(user->get_charac_level()),
            1);

        if (renew_my_record) {
            DB_UpdateDeathTowerBestRecord::makeRequest(
                rank_index,
                charac_no,
                start_member_count,
                start_member_info,
                current_stage,
                play_time);

            if (virtual_rank > 0) {
                CMonitorServerProxy* const proxy =
                    GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                        SERVER_GROUP_DEFAULT);
                proxy->updateTowerRank(charac_no,
                                       rank_index,
                                       start_member_count,
                                       static_cast<unsigned int>(virtual_rank));
                rank_table->updateRanking(
                    rank_index,
                    charac_no,
                    static_cast<unsigned int>(virtual_rank));
            }
            rank_table->registBestRecord(rank_index,
                                         charac_no,
                                         start_member_info,
                                         current_stage,
                                         play_time);
        }

        CPacketHandler ranking_packet;
        ranking_packet.makeDisplayRangking(start_member_count,
                                           charac_no,
                                           current_stage,
                                           play_time,
                                           renew_my_record == 1,
                                           dungeon_index,
                                           static_cast<unsigned int>(virtual_rank));
        user->Send(ranking_packet);

        int reward_count = dungeon->limitOfStackableItemInTower()
                               ? G_CDataManager()->get_death_tower_reward_card_num(
                                     current_stage)
                               : G_CDataManager()
                                     ->get_death_tower_illusion_reward_card_num(
                                         current_stage);
        reward_count += record_reward_bonus;
        reward_count = std::min(dungeon->getTowerMaxClearRewardItemNum(),
                                reward_count);
        reward_count = std::min(kDeathTowerMaxRewardCards, reward_count);
        const unsigned int reward_probability_scale =
            std::min(static_cast<unsigned int>(kDeathTowerMaxRewardCards),
                     current_stage);
        play_data_.generateRewardItem(seat,
                                      reward_count,
                                      dungeon_manager_,
                                      static_cast<unsigned int>(
                                          user->get_charac_level()),
                                      reward_probability_scale,
                                      party_);

        unsigned int mob_reward = 0;
        G_CDataManager()->get_mob_reward(user->get_charac_level(), mob_reward);
        QuickParty::RewardData_Param reward_parameter(party_);
        QuickParty::RewardData_Result reward_result;
        reward_result.raw_00 = static_cast<int>(mob_reward);
        party_->set_random_reward(
            static_cast<QuickParty::RandomBuffType>(0),
            reward_parameter,
            reward_result);
        play_data_.generateRewardExp(
            seat,
            static_cast<int>(current_stage),
            static_cast<unsigned int>(reward_result.raw_00),
            party_,
            dungeon_manager_);
    }
}

}  // namespace WongWork
