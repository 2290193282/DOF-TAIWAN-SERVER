#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_game/player/core/title_book_runtime.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/PVF/n_quest/quest_parameter_script.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_game/progression/quest/quest_delivery_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <algorithm>
#include <cstring>

namespace {

bool RewardMatchesUser(const RewardItemInfo& reward, const CUser& user) {
    return reward.item_type == oldsrv::config::kCharacterJobCommon ||
           (reward.item_type == user.get_charac_job() &&
            (reward.period == -1 ||
             reward.period == user.getCurCharFirstGrowType()));
}

void AddScaledRewardItems(const Quest& quest,
                          const CUser& user,
                          int trigger_value,
                          std::vector<std::pair<int, int> >& items) {
    const std::vector<RewardItemInfo>* const rewards =
        CUser::getUserQuestRewardItem(&quest, &user);
    for (std::vector<RewardItemInfo>::const_iterator it = rewards->begin();
         it != rewards->end();
         ++it) {
        if (RewardMatchesUser(*it, user)) {
            items.push_back(std::make_pair(
                it->item_id, trigger_value * it->item_count));
        }
    }

}

int CanDeliverQuestReward(CUser& user,
                          Quest& quest,
                          int& selected_reward_index,
                          int trigger_value,
                          const std::vector<std::pair<int, int> >& give_items) {
    CInventory shadow_inventory;
    InventoryMemory shadow_memory;
    shadow_inventory.SetInventoryMemory(&shadow_memory);
    const CInventory* const inventory = user.getCurCharacInvenRefR();
    shadow_inventory.setCopy(*inventory);

    if ((quest.trigger_type == 0 || quest.trigger_type == 25) &&
        !shadow_inventory.tryDeleteEventItems(give_items)) {
        return 17;
    }

    if (quest.reward_type != 0) {
        return 0;
    }

    std::vector<std::pair<int, int> > reward_items;
    AddScaledRewardItems(quest, user, trigger_value, reward_items);
    if (shadow_inventory.tryInsertEventItems(reward_items) !=
        static_cast<int>(reward_items.size())) {
        return 4;
    }

    selected_reward_index = quest.ConvertRewardSelectIndex(
        &user, selected_reward_index);
    if (static_cast<int>(quest.light_reward_items.size()) <=
        selected_reward_index) {
        return 21;
    }
    if (selected_reward_index < 0 || selected_reward_index == 0xffff) {
        return 0;
    }

    const RewardItemInfo& item =
        quest.light_reward_items[selected_reward_index];
    Inven_Item inventory_item;
    inventory_item.item_id_ = item.item_id;
    G_CDataManager()->get_item_list()->create_item(
        static_cast<ENUM_ITEM_CREATE_TYPE>(4),
        inventory_item,
        trigger_value * item.item_count);
    return shadow_inventory.tryInsertItemIntoInventory(inventory_item) == -1
               ? 4
               : 0;
}

void PutDeletedItemData(PacketGuard& packet,
                        const std::vector<int>& deleted_slots) {
    for (std::vector<int>::const_iterator it = deleted_slots.begin();
         it != deleted_slots.end();) {
        packet.put_byte(*it++);
        packet.put_short(*it++);
        packet.put_int(*it++);
    }
}

void PutInsertedItemData(PacketGuard& packet,
                         const CInventory& inventory,
                         const std::vector<int>& inserted_slots) {
    for (std::vector<int>::const_iterator it = inserted_slots.begin();
         it != inserted_slots.end();) {
        const int slot = *it++;
        const int item_id = *it++;
        const int unique_id = *it++;
        packet.put_short(slot);
        packet.put_int(item_id);
        packet.put_int(unique_id);

        const CItem* const item = G_CDataManager()->find_item(item_id);
        const int inventory_type =
            item && item->vtable_->is_avatar_item(item) ? 2 : 1;
        const Inven_Item inserted = inventory.GetInvenSlot(inventory_type,
                                                            slot);
        packet.put_byte(inserted.amplify_option_.getAbilityType());
        packet.put_short(inserted.amplify_option_.getAbilityValue());
        packet.put_int(item && item->is_stackable()
                           ? inserted.avatar_select_
                           : 0);
    }
}

}  // namespace

// IDA_EA: 0x866E3E2
// IDA_NAME: _ZN5CUser30compute_quest_basic_reward_expEP5Quest
// DEMANGLED: CUser::compute_quest_basic_reward_exp(Quest *)
unsigned int CUser::compute_quest_basic_reward_exp(Quest* quest) {
    if (quest->quest_grade == 4) {
        return 0;
    }

    const int level_difference = get_charac_level() - quest->minimum_level;
    int level_penalty = 100;
    if (quest->value_130 != 1) {
        level_penalty = compute_level_penalty(level_difference);
    }

    QuestParameterScript* const parameters =
        G_CDataManager()->get_quest_parameter_script();
    const int reward_weight = parameters->getRewardWeight(
        static_cast<char>(quest->difficulty));
    const int reward_level = (quest->event || quest->value_130)
        ? get_charac_level()
        : quest->minimum_level;
    unsigned int reward_exp = static_cast<unsigned int>(
        parameters->getRewardExp(reward_level));
    reward_exp = static_cast<unsigned int>(
        level_penalty * (reward_weight * static_cast<int>(reward_exp) / 100) /
        100);

    int premium_bonus_exp = 0;
    if (isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(79))) {
        premium_bonus_exp = setPremiumBonusExp(
            reward_exp,
            getAddExpRateFromPremium(static_cast<ENUM_PREMIUM_TYPE>(79)));
    }

    const STExpReward growth_power_reward = get_growth_power_exp_reward();
    const int seria_buff_bonus_exp = static_cast<int>(
        static_cast<long double>(reward_exp) *
        (static_cast<long double>(
             growth_power_reward.gained_exp_bonus_rate) /
         100.0L));
    set_charac_seria_buf_bonus_exp(seria_buff_bonus_exp);
    return static_cast<unsigned int>(
        seria_buff_bonus_exp + premium_bonus_exp + reward_exp);
}

// IDA_EA: 0x866E5E6
// IDA_NAME: _ZN5CUser31compute_quest_basic_reward_goldEP5Quest
// DEMANGLED: CUser::compute_quest_basic_reward_gold(Quest *)
int CUser::compute_quest_basic_reward_gold(Quest* quest) {
    if (quest->reward_type != 0 && quest->reward_type != 25) {
        return 0;
    }

    bool has_gold_reward = false;
    for (std::vector<RewardItemInfo>::const_iterator it =
             quest->reward_items.begin();
         it != quest->reward_items.end();
         ++it) {
        if (it->item_id == 0) {
            has_gold_reward = true;
            break;
        }
    }
    if (!has_gold_reward) {
        return 0;
    }

    const int level_difference = get_charac_level() - quest->minimum_level;
    int level_penalty = 100;
    int reward_level = get_charac_level();
    if (quest->value_130 != 1) {
        level_penalty = compute_level_penalty(level_difference);
        reward_level = quest->minimum_level;
    }

    QuestParameterScript* const parameters =
        G_CDataManager()->get_quest_parameter_script();
    const int reward_gold = parameters->getRewardGold(reward_level);
    return quest->gold_multiple * (level_penalty * reward_gold / 100) / 100;
}

// IDA_EA: 0x866E7A8
// IDA_NAME: _ZN5CUser18quest_basic_rewardEP5QuestRiS2_S2_S2_b
// DEMANGLED: CUser::quest_basic_reward(Quest *,int &,int &,int &,int &,bool)
int CUser::quest_basic_reward(Quest* quest,
                              int& reward_exp,
                              int& reward_gold,
                              int& reward_quest_point,
                              int& reward_quest_piece,
                              bool suppress_packets) {
    reward_exp = static_cast<int>(compute_quest_basic_reward_exp(quest));
    reward_gold = compute_quest_basic_reward_gold(quest);

    int abuse_penalty = static_cast<int>(
        static_cast<unsigned int>(getCurCharacHelpAbuseComputedRatio()) /
        10000u);
    if (abuse_penalty > 100) {
        abuse_penalty = 100;
    }

    const QuestParameterScript* const parameters =
        G_CDataManager()->get_quest_parameter_script();
    if (parameters->bal_kun_penalty < abuse_penalty) {
        reward_exp = (100 - abuse_penalty) * reward_exp / 100;
        reward_gold = (100 - abuse_penalty) * reward_gold / 100;
    }

    reward_quest_point = quest->quest_point;
    if (reward_quest_point > 0) {
        incQuestPoint(reward_quest_point, quest->quest_index, suppress_packets);
    }

    reward_quest_piece = quest->quest_piece;
    if (reward_quest_piece > 0) {
        incQuestPiece(reward_quest_piece,
                      quest->quest_index,
                      suppress_packets);
    }
    return reward_quest_piece;
}

// IDA_EA: 0x866E914
// IDA_NAME: _ZN5CUser22getUserQuestRewardItemEPK5QuestPKS_
// DEMANGLED: CUser::getUserQuestRewardItem(Quest const*,CUser const*)
const std::vector<RewardItemInfo>* CUser::getUserQuestRewardItem(
    const Quest* quest,
    const CUser* user) {
    if (!quest->clear_reward_items.empty()) {
        if (quest->value_e8 == 1) {
            for (std::vector<int>::const_iterator it =
                     quest->clear_reward_items.begin();
                 it != quest->clear_reward_items.end();
                 ++it) {
                if (user->getCurCharacQuestR()->isClearQuest(
                        static_cast<unsigned int>(*it))) {
                    return &quest->reward_select_items;
                }
            }
        } else {
            std::vector<int>::const_iterator it =
                quest->clear_reward_items.begin();
            for (; it != quest->clear_reward_items.end(); ++it) {
                if (!user->getCurCharacQuestR()->isClearQuest(
                        static_cast<unsigned int>(*it))) {
                    break;
                }
            }
            if (it == quest->clear_reward_items.end()) {
                return &quest->reward_select_items;
            }
        }
    }
    return &quest->reward_items;
}

// IDA_EA: 0x86645A4
// IDA_NAME: _ZN5CUser23eventDeliveryQuestClearEiiRSt6vectorISt4pairIiiESaIS2_EEiii
// DEMANGLED: CUser::eventDeliveryQuestClear(int,int,std::vector<std::pair<int,int> > &,int,int,int)
int CUser::eventDeliveryQuestClear(
    int quest_index,
    int /*trigger_value*/,
    std::vector<std::pair<int, int> >& reward_items,
    int reward_exp,
    int reward_gold,
    int reward_quest_point) {
    QuestClearDeliveryMsg message;
    message.setBaseData(1,
                        static_cast<unsigned int>(get_acc_id()),
                        getCurCharacNo(),
                        GetServerGroupToPvP());
    message.quest_index = quest_index;
    message.reward_exp = reward_exp;
    message.reward_gold = reward_gold;
    message.reward_quest_point = reward_quest_point;
    message.items = &reward_items;
    return GlobalData::s_event_manager->eventDeliveryNotify(
        message.delivery_type,
        message);
}

// IDA_EA: 0x866EA92
// IDA_NAME: _ZN5CUser12quest_rewardEiii
// DEMANGLED: CUser::quest_reward(int,int,int)
int CUser::quest_reward(int quest_index,
                        int selected_reward_index,
                        int trigger_value) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_FINISH_QUEST);
    packet.put_byte(1);
    packet.put_short(quest_index);

    UserQuest* const user_quest = getCurCharacQuestR();
    if (!user_quest->check_cond_clear(quest_index)) {
        return 22;
    }

    Quest* const quest = G_CDataManager()->find_quest(quest_index);
    if (!quest) {
        return 21;
    }
    if (!IsGameMasterMode() && !quest->isRepeatableQuest() &&
        user_quest->isClearQuest(quest_index)) {
        return 22;
    }
    if (quest->quest_grade != 4 && trigger_value != 1) {
        return 22;
    }

    std::vector<std::pair<int, int> > give_items;
    if (quest->trigger_type == 0 || quest->trigger_type == 25) {
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 quest->reward_select_pairs.begin();
             it != quest->reward_select_pairs.end();
             ++it) {
            give_items.push_back(std::make_pair(
                it->first, trigger_value * it->second));
        }
        if (!getCurCharacInvenR()->CheckNeedItemList(give_items)) {
            return 17;
        }
    }

    const bool needs_inventory_preflight =
        !((quest->trigger_type != 0 && quest->reward_type != 0 &&
           quest->trigger_type != 25) ||
          quest->reward_type == 25);
    if (needs_inventory_preflight) {
        const int preflight_result = CanDeliverQuestReward(
            *this,
            *quest,
            selected_reward_index,
            trigger_value,
            give_items);
        if (preflight_result) {
            return preflight_result;
        }
    }

    int reward_exp = 0;
    int reward_gold = 0;
    int reward_quest_point = 0;
    int reward_quest_piece = 0;
    quest_basic_reward(quest,
                       reward_exp,
                       reward_gold,
                       reward_quest_point,
                       reward_quest_piece,
                       false);

    if (reward_exp) {
        int reward_sp = 0;
        int reward_sfp = 0;
        set_charac_party_bonus_exp(0);
        set_charac_member_bonus_exp(0);
        set_charac_fatigue_buf_bonus_exp(0);
        const bool level_up = gain_exp_sp(reward_exp,
                                          reward_sp,
                                          reward_sfp,
                                          static_cast<eExpAddReason>(13),
                                          0,
                                          false);
        if (level_up) {
            if (getCurCharacStamina() <= 99u) {
                setCurCharacStamina(100u);
            }
            if (get_state() == 3) {
                SendNotiPacket(static_cast<eSendTarget>(0),
                               static_cast<ENUM_NOTIPACKET>(2),
                               0);
                SendNotiPacket(static_cast<eSendTarget>(1),
                               static_cast<ENUM_NOTIPACKET>(2),
                               1);
            } else if (get_state() > 4) {
                SendNotiPacket(static_cast<eSendTarget>(2),
                               static_cast<ENUM_NOTIPACKET>(2),
                               1);
            }
        }
        if (get_state() == 3) {
            SendNotiPacket(static_cast<eSendTarget>(1),
                           static_cast<ENUM_NOTIPACKET>(37),
                           0);
            if (level_up) {
                decide_growth_power_reward_system();
            }
        }
    }

    bool equipment_changed = false;
    std::vector<int> deleted_slots;
    std::vector<int> inserted_slots;
    int deleted_count = 0;
    int inserted_count = 0;
    unsigned long requested_money = 0;
    unsigned long inserted_money = 0;

    if ((quest->trigger_type == 0 || quest->trigger_type == 25) &&
        quest->reward_type != 25) {
        deleted_count = getCurCharacInvenW()->delete_event_items(
            give_items,
            deleted_slots,
            equipment_changed,
            static_cast<eMoneySubReason>(4),
            static_cast<eItemDelReason>(4),
            false);
        if (!deleted_count) {
            cMyTrace trace("int CUser::quest_reward(int, int, int)",
                           23023,
                           5);
            trace("CUser::quest_reward, QuestID : %d delete_object_count == 0 ",
                  quest_index);
            return 17;
        }
        if (getGmQuestFlag()) {
            deleted_count = 0;
        }
    }

    std::vector<std::pair<int, int> > reward_items;
    if (quest->reward_type == 0) {
        const int money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
            get_charac_level(), get_acc_name());
        int current_money = getCurCharacInvenR()->get_money();
        AddScaledRewardItems(*quest, *this, trigger_value, reward_items);
        if (selected_reward_index >= 0 &&
            static_cast<int>(quest->light_reward_items.size()) >
                selected_reward_index) {
            const RewardItemInfo& selected =
                quest->light_reward_items[selected_reward_index];
            reward_items.push_back(std::make_pair(
                selected.item_id, trigger_value * selected.item_count));
        }
        for (std::vector<std::pair<int, int> >::iterator it =
                 reward_items.begin();
             it != reward_items.end();
             ++it) {
            if (it->first != 0) {
                SendAradEventItemLog(static_cast<unsigned long>(it->first),
                                     it->second);
                continue;
            }

            requested_money = static_cast<unsigned long>(
                trigger_value * reward_gold);
            if (money_limit - static_cast<int>(requested_money) <
                current_money) {
                if (current_money >= money_limit) {
                    it->second = 0;
                } else {
                    const int available = money_limit - current_money;
                    it->second = available;
                    inserted_money += static_cast<unsigned long>(available);
                    current_money = money_limit;
                }
            } else {
                current_money += static_cast<int>(requested_money);
                inserted_money += requested_money;
                it->second = static_cast<int>(requested_money);
            }
        }

        inserted_count = getCurCharacInvenW()->insert_event_items(
            reward_items,
            inserted_slots,
            static_cast<eMoneyAddReason>(3),
            static_cast<eItemAddReason>(8),
            false,
            0,
            0);
        eventDeliveryQuestClear(quest_index,
                                trigger_value,
                                reward_items,
                                reward_exp,
                                reward_gold,
                                reward_quest_point);
    }

    packet.put_byte(quest->trigger_type);
    packet.put_int(reward_exp);
    if (quest->trigger_type == 0 || quest->trigger_type == 25) {
        if (quest->reward_type == 25) {
            packet.put_byte(1);
            packet.put_byte(7);
            packet.put_short(0);
            packet.put_int(0);
        } else {
            packet.put_byte(deleted_count);
            if (deleted_count > 0) {
                PutDeletedItemData(packet, deleted_slots);
            }
        }
    }

    packet.put_byte(quest->reward_type);
    if (quest->reward_type == 0) {
        packet.put_byte(inserted_count);
        if (inserted_count > 0) {
            PutInsertedItemData(packet,
                                *getCurCharacInvenR(),
                                inserted_slots);
        }
    }

    if (quest->reward_type == 1) {
        packet.put_byte(quest->value_c0);
        set_grow_type(static_cast<unsigned char>(quest->value_c0),
                      getCurCharSecondGrowType(),
                      reinterpret_cast<char*>(&packet),
                      static_cast<eChangeGrowTypeReason>(0));

        PacketGuard basic_info;
        basic_info.put_header(0, 2);
        basic_info.put_byte(0);
        basic_info.put_short(1);
        make_basic_info(basic_info, 0);
        basic_info.finalize(true);
        G_GameWorld()->send_all(basic_info, this);

        CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
            "user.cpp", 23277), true);
        guard << 36;
        guard << GetUID();
        SIG_LOAD_SKILL* const skill = guard.GetInBuffer<SIG_LOAD_SKILL>();
        std::memset(skill, 0, sizeof(*skill));
        skill->charac_no = static_cast<unsigned int>(getCurCharacNo());
        skill->skill_table_index = 1;
        skill->script_version =
            G_CDataManager()->get_pvp_skill_tree_parameter_script()->version;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }

    if (quest->reward_type == 2) {
        packet.put_byte(quest->value_c0);
        set_grow_type(getCurCharFirstGrowType(),
                      static_cast<unsigned char>(quest->value_c0),
                      reinterpret_cast<char*>(&packet),
                      static_cast<eChangeGrowTypeReason>(0));

        PacketGuard basic_info;
        basic_info.put_header(0, 2);
        basic_info.put_byte(0);
        basic_info.put_short(1);
        make_basic_info(basic_info, 0);
        basic_info.finalize(true);
        G_GameWorld()->send_all(basic_info, this);
    }

    if (quest->reward_type == 10 && quest->value_c0 > 0 &&
        !getCurCharacInvenW()->GetCreatureMgrW()->Evolute(
            quest->value_c0, quest->value_fc, false)) {
        return 22;
    }

    if (quest->reward_type == 20) {
        if (get_charac_level() < quest->minimum_level) {
            return 22;
        }
        SetCurCharacExpertJobType(quest->value_c0);
        packet.put_byte(quest->value_c0);
        APSystem::CUserProc::ClearActionAndSendtoUser(
            this,
            static_cast<APSystem::_ActionGroupIndex>(0),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
        LogHistory("ExpertJob+,%d,%d", get_charac_level(), quest->value_c0);

        int skill_count_position = packet.get_index();
        int skill_count = 0;
        packet.put_byte(0);
        const STExpertJobScript* const script =
            G_CDataManager()->GetExpertJobScript(quest->value_c0);
        if (script) {
            for (std::vector<std::pair<int, int> >::const_iterator it =
                     script->unknown_exp_pairs.begin();
                 it != script->unknown_exp_pairs.end();
                 ++it) {
                if (!getCurCharacR()) {
                    continue;
                }
                const int changed = getCurCharacSkillW()->growtype_skill(
                    get_charac_job(), it->first, it->second,
                    ENUM_SKILL_TREE_0);
                if (changed > 0) {
                    packet.put_byte(changed);
                    packet.put_byte(it->first);
                    packet.put_byte(it->second);
                    ++skill_count;
                }
            }
            packet.put_byte(skill_count_position, skill_count);

            int second_skill_count_position = packet.get_index();
            skill_count = 0;
            packet.put_byte(0);
            for (std::vector<std::pair<int, int> >::const_iterator it =
                     script->unknown_exp_pairs.begin();
                 it != script->unknown_exp_pairs.end();
                 ++it) {
                if (!getCurCharacR()) {
                    continue;
                }
                const int changed = getCurCharacSkillW()->growtype_skill(
                    get_charac_job(), it->first, it->second,
                    ENUM_SKILL_TREE_1);
                if (changed > 0) {
                    packet.put_byte(changed);
                    packet.put_byte(it->first);
                    packet.put_byte(it->second);
                    ++skill_count;
                }
            }
            packet.put_byte(second_skill_count_position, skill_count);
            static_cast<expert_job::CCharacExpertJob*>(
                GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)))->InitScript(
                        const_cast<STExpertJobScript*>(script));
            SetCurCharacExpertJobExp(0);
        }

        PacketGuard basic_info;
        basic_info.put_header(0, 2);
        basic_info.put_byte(0);
        basic_info.put_short(1);
        make_basic_info(basic_info, 0);
        basic_info.finalize(true);
        G_GameWorld()->send_all(basic_info, this);
    }

    if (quest->reward_type == 21) {
        packet.put_short(quest->value_c0);
        AddDungeonClear(quest->value_c0, 3);

        PacketGuard dungeon_packet;
        dungeon_packet.put_header(0, 5);
        dungeon_packet.put_short(1);
        dungeon_packet.put_short(quest->value_c0);
        dungeon_packet.put_byte(4);
        dungeon_packet.finalize(true);
        Send(dungeon_packet);
    }

    if (quest->reward_type == 22) {
        const int requested_slot = quest->value_c0;
        const int current_slots = GetCurExEquipSlotStat();
        // 0828 PVF uses 21 for the auxiliary-equipment (left) slot and 22
        // for the magic-stone (right) slot.  value_c0 is the raw
        // [reward int data] value; neither QuestScript nor Quest subtracts 1.
        if (requested_slot == 21) {
            if (current_slots == 0 || current_slots == 2) {
                SetCurExpEquipSlotStat(
                    static_cast<char>(current_slots + 1));
            }
        } else if (requested_slot == 22 && current_slots <= 1) {
            SetCurExpEquipSlotStat(static_cast<char>(current_slots + 2));
        }
        packet.put_byte(GetCurExEquipSlotStat());
        packet.put_byte(requested_slot);
    }

    if (quest->reward_type == 25) {
        std::vector<std::pair<int, int> > creature_rewards;
        AddScaledRewardItems(*quest,
                             *this,
                             trigger_value,
                             creature_rewards);
        if (selected_reward_index >= 0 &&
            static_cast<int>(quest->light_reward_items.size()) >
                selected_reward_index) {
            const RewardItemInfo& selected =
                quest->light_reward_items[selected_reward_index];
            creature_rewards.push_back(std::make_pair(
                selected.item_id, trigger_value * selected.item_count));
        }
        if (creature_rewards.size() != 1) {
            return 22;
        }
        const std::pair<int, int>& creature_reward = creature_rewards.front();
        const CItem* const creature_item =
            G_CDataManager()->find_item(creature_reward.first);
        if (!creature_item) {
            return 22;
        }
        if (creature_item->vtable_->get_item_type(creature_item) ==
                oldsrv::config::kEquipmentTypeCreature &&
            !getCurCharacInvenW()->GetCreatureMgrW()->EventEvoluteCreature(
                creature_reward.first,
                quest->value_fc,
                creature_reward.second)) {
            return 22;
        }
    }

    if (quest->reward_type == 26 && quest->value_c0 > 0) {
        // 0828 reward enum 26 is [title].  putTitleOnly performs the official
        // title-book slot resolution, prototype construction and dirty-state
        // update; the finish-quest packet already carries reward_type 26.
        CTitleBook::putTitleOnly(this, quest->value_c0);
    }

    packet.finalize(true);
    Send(packet);
    if (equipment_changed) {
        send_equip(oldsrv::config::kEquipmentTypeCreature);
    }
    if (inserted_money < requested_money) {
        SendMoneyFullReason(static_cast<ENUM_MONEY_FULL_REASON>(4),
                            requested_money,
                            inserted_money);
    }
    return 0;
}
