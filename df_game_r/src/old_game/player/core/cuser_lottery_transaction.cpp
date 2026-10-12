#include "old_game/player/core/cuser.h"

#include <stdint.h>

#include "common/config.h"
#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const int kLotteryInventoryStackable = 1;
const int kLotteryInventoryAvatar = 2;
const int kLotteryInventoryCreature = 5;
const int kLotteryCapacityFailure = 4;
const int kLotteryGoldInsufficient = 10;
const int kLotteryCharacterLevelFailure = 14;
const int kLotteryInvalidReward = 17;
const int kLotterySourceDeleteFailure = 17;
const int kLotterySourceNotFound = 21;
const int kLotteryMoneyLimitFailure = 22;
const int kLotteryDeleteReason = 3;
const int kLotteryAddReason = 3;
const int kLotteryAvatarAddReason = 7;
const int kLotteryMoneyAddReason = 23;
const int kLotteryMoneySubReason = 45;
const int kLotteryValueStatisticField = 7;

const unsigned int kLotteryFourBox = 2670497u;
const unsigned int kLotteryDelayedDeleteBox = 2670283u;

bool IsAvatarItem(const CItem& item) {
    return item.vtable_->is_avatar_item(&item);
}

bool IsCreatureItem(const CItem& item) {
    return item.vtable_->is_creature_item(&item);
}

int GetItemType(const CItem& item) {
    return item.vtable_->get_item_type(&item);
}

bool IsRandomOptionItem(const CItem& item) {
    return item.vtable_->is_random_option(&item) != 0;
}

int GetRandomOptionGrade(const CItem& item) {
    return item.vtable_->get_random_option_grade(&item);
}

int SelectLotteryUpgrade(const LotteryUpgradePolicy& policy) {
    const int roll = static_cast<int>(get_rand_int(10000));
    int upgrade = 0;

    if (policy.first_band_probability > roll) {
        upgrade = policy.first_upgrade_base +
                  static_cast<int>(get_rand_int(policy.first_band_roll_range)) /
                      1000;
    } else if (policy.second_band_probability +
                   policy.first_band_probability >
               roll) {
        upgrade = policy.second_upgrade_base +
                  static_cast<int>(get_rand_int(policy.second_band_roll_range)) /
                      1000;
    } else {
        upgrade = policy.third_upgrade_base +
                  static_cast<int>(get_rand_int(policy.third_band_roll_range)) /
                      1000;
    }

    return upgrade < 0x20 ? upgrade : 0;
}

bool NeedsLotteryItemBroadcast(const LotteryUpgradePolicy& policy,
                               const CItem& reward_data,
                               const Inven_Item& reward) {
    if (policy.broadcast_minimum_rarity <= reward_data.get_rarity()) {
        return true;
    }

    const int item_type = GetItemType(reward_data);
    if (IsAvatarItem(reward_data) && item_type >= 0 &&
        item_type < static_cast<int>(sizeof(policy.broadcast_avatar_item_types)) &&
        policy.broadcast_avatar_item_types[item_type] != 0) {
        return true;
    }

    return policy.broadcast_minimum_upgrade <= reward.GetUpgrade();
}

void SendLotteryRewardPacket(CUser& user,
                             int source_slot,
                             int inserted_slot,
                             const Inven_Item& reward,
                             int reported_add_info) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_LOTTERY_ITEM);
    packet.put_byte(1);
    packet.put_short(source_slot);
    packet.put_short(inserted_slot);
    packet.put_int(reward.item_id_);
    packet.put_int(reported_add_info);
    packet.put_short(reward.avatar_select_);
    packet.put_byte(reward.GetItemAttr());
    packet.put_byte(reward.amplify_option_.getAbilityType());
    packet.put_short(reward.amplify_option_.getAbilityValue());

    if (reward.isAvatarItemType()) {
        const CInventory* const inventory = user.getCurCharacInvenR();
        const WongWork::CAvatarItemMgr* const avatar_manager =
            inventory->GetAvatarItemMgrR();
        const int avatar_key = reward.get_add_info();

        packet.put_int(30);
        packet.put_binary(reinterpret_cast<const char*>(
                              avatar_manager->getJewelSocketData(avatar_key)),
                          30);
        packet.put_int(4);

        const stAvatarExpansionInfo_t* const expansion =
            avatar_manager->GetExpansionInfo(avatar_key);
        if (expansion) {
            packet.put_binary(reinterpret_cast<const char*>(expansion), 4);
        } else {
            stAvatarExpansionInfo_t empty_expansion;
            empty_expansion.init();
            packet.put_binary(reinterpret_cast<const char*>(&empty_expansion),
                              4);
        }
    }

    packet.put_packet(reward);
    packet.finalize(true);
    user.Send(packet);
}

void SendLotteryGoldPacket(CUser& user,
                           int source_slot,
                           int actual_money) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_LOTTERY_ITEM);
    packet.put_byte(1);
    packet.put_short(source_slot);
    packet.put_short(0);
    packet.put_int(0);
    packet.put_int(actual_money);
    packet.put_short(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_packet(g_emptySlot);
    packet.finalize(true);
    user.Send(packet);
}

}  // namespace

// IDA_EA: 0x868D1EC
// IDA_NAME: _ZN5CUser35send_broadcast_lottery_gold_messageEji
// DEMANGLED: CUser::send_broadcast_lottery_gold_message(unsigned int,int)
void CUser::send_broadcast_lottery_gold_message(unsigned int item_id,
                                                int reward_money) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item || item->is_stackable() != 1) {
        return;
    }

    const CStackableItem* const stackable =
        static_cast<const CStackableItem*>(item);
    if (stackable->lottery_gold_broadcast_minimum() <= 0 ||
        stackable->lottery_gold_broadcast_minimum() > reward_money) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 86);
    packet.put_byte(9);
    packet.put_short(get_unique_id());
    packet.put_int(static_cast<int>(item_id));
    packet.put_int(reward_money);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x86733A2
// IDA_NAME: _ZN5CUser12lottery_itemEi
// DEMANGLED: CUser::lottery_item(int)
int CUser::lottery_item(int source_slot) {
    const Inven_Item source = getCurCharacInvenR()->GetInvenSlot(
        kLotteryInventoryStackable, source_slot);
    const unsigned int source_item_id = static_cast<unsigned int>(source.item_id_);
    CItem* const source_data = G_CDataManager()->find_item(source_item_id);
    if (!source_data) {
        return kLotterySourceNotFound;
    }
    if (source_data->getUsableLevel() > get_charac_level()) {
        return kLotteryCharacterLevelFailure;
    }

    // Retail deliberately passes BYTE2 of the packed Inven_Item, i.e. the
    // low byte of item_id, into CLottery_NeedMoney.  Preserve that ABI-level
    // key rather than substituting a category or full item number.
    const unsigned int lottery_cost_key = source_item_id & 0xffu;
    int lottery_cost = G_CDataManager()->find_lottery_use_cost(lottery_cost_key);
    if (lottery_cost != 0 &&
        getCurCharacInvenR()->get_money() < lottery_cost) {
        return kLotteryGoldInsufficient;
    }

    Inven_Item reward;
    G_CDataManager()->get_item_list()->get_lotto_item(
        source.item_id_, reinterpret_cast<char*>(&reward));

    if (reward.item_id_ == 0 && reward.get_add_info() > 0) {
        const int reward_money = reward.get_add_info();
        const int money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
            get_charac_level(), get_acc_name());
        if (getCurCharacInvenR()->get_money() + reward_money > money_limit) {
            cMyTrace trace("int CUser::lottery_item(int)", 25017, 4);
            trace("lottery_gold_item:m_id(%s), char_no(%d), money(%d)",
                  NumberToString(static_cast<unsigned int>(get_acc_id()), 0),
                  getCurCharacNo(), reward_money);
            return kLotteryMoneyLimitFailure;
        }

        const int actual_money = getCurCharacInvenW()->gain_money(
            reward_money, static_cast<eMoneyAddReason>(kLotteryMoneyAddReason),
            true, 0);
        if (actual_money < reward_money) {
            SendMoneyFullReason(static_cast<ENUM_MONEY_FULL_REASON>(0),
                                reward_money, actual_money);
        }
        if (getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(kLotteryInventoryStackable),
                source_slot, 1,
                static_cast<eItemDelReason>(kLotteryDeleteReason), true) != 1) {
            return kLotterySourceDeleteFailure;
        }

        SendLotteryGoldPacket(*this, source_slot, actual_money);
        send_broadcast_lottery_gold_message(source_item_id, reward_money);
        return 0;
    }

    CItem* const reward_data = G_CDataManager()->find_item(reward.item_id_);
    if (!reward_data || reward.item_id_ == 0 || reward.item_id_ == -1) {
        return kLotteryInvalidReward;
    }

    if (source_data->is_stackable() == 1 && GetItemType(*source_data) == 13 &&
        reward.item_category_ == kLotteryInventoryStackable &&
        GetItemType(*reward_data) != 11) {
        reward.SetUpgrade(SelectLotteryUpgrade(
            G_CDataManager()->get_lottery_upgrade_policy()));
    }

    if (getCurCharacInvenR()->QuickEmptyCount() <= 0) {
        const Inven_Item::ITEM_TYPE required_spaces[] = {
            static_cast<Inven_Item::ITEM_TYPE>(1),
            static_cast<Inven_Item::ITEM_TYPE>(2),
            static_cast<Inven_Item::ITEM_TYPE>(3),
            static_cast<Inven_Item::ITEM_TYPE>(10),
        };
        if (getCurCharacInvenR()->check_empty_count(required_spaces[0], 1) != 1 ||
            (source.get_add_info() > 1 &&
             getCurCharacInvenR()->check_empty_count(required_spaces[1], 1) !=
                 1) ||
            getCurCharacInvenR()->check_empty_count(required_spaces[2], 1) != 1 ||
            getCurCharacInvenR()->check_empty_count(required_spaces[3], 1) != 1) {
            return kLotteryCapacityFailure;
        }
    }

    if (source_item_id == kLotteryFourBox) {
        const unsigned int monitored_items[] = {
            2670493u, 2670494u, 2670495u, 2670496u,
        };
        for (unsigned int index = 0;
             index != sizeof(monitored_items) / sizeof(monitored_items[0]);
             ++index) {
            Inven_Item monitored;
            getCurCharacInvenW()->GetInvenData(
                static_cast<int>(monitored_items[index]), monitored);
            if (monitored.get_add_info() > 9) {
                return kLotteryCapacityFailure;
            }
        }
    }

    if (source_item_id != kLotteryDelayedDeleteBox &&
        getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(kLotteryInventoryStackable), source_slot, 1,
            static_cast<eItemDelReason>(kLotteryDeleteReason), true) != 1) {
        return kLotterySourceDeleteFailure;
    }

    int inserted_slot = 0;
    int reported_add_info = 0;
    if (IsAvatarItem(*reward_data)) {
        char ipg_no[24] = {};
        WongWork::CGenUniqueNo::genIPGNo(
            static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(1),
            static_cast<unsigned int>(getCurCharacNo()), ipg_no);
        inserted_slot = getCurCharacInvenW()->AddAvatarItem(
            reward.item_id_, reward_data->getUsablePeriod(), false, 0,
            static_cast<char>(-1), ipg_no,
            static_cast<eAvatarItemAddReason>(kLotteryAvatarAddReason), false,
            0);
        if (inserted_slot == -1) {
            return kLotteryCapacityFailure;
        }

        const Inven_Item inserted_item = getCurCharacInvenR()->GetInvenSlot(
            kLotteryInventoryAvatar, inserted_slot);
        reported_add_info = getCurCharacInvenR()->GetAvatarItemMgrR()
                                ->GetRemainDate(
                                    inserted_item.get_add_info(),
                                    GlobalData::system_time()->getCurSec());
        reward.SetUpgrade(0);
    } else if (IsCreatureItem(*reward_data) &&
               GetItemType(*reward_data) ==
                   oldsrv::config::kEquipmentTypeCreature) {
        if (reward.item_category_ != kLotteryInventoryCreature ||
            !user_creature::CCreatureItemConverter::SetEggItem(&reward)) {
            return kLotteryInvalidReward;
        }
        inserted_slot = getCurCharacInvenW()->insertItemIntoInventory(
            reward, static_cast<eItemAddReason>(kLotteryAddReason), true, false);
        if (inserted_slot < 0) {
            return kLotteryCapacityFailure;
        }
        getCurCharacInvenW()->GetCreatureMgrW()->InsertCreatureItem(
            &reward, inserted_slot, 25, reward_data->getUsablePeriod(),
            reward_data->getExpirationDate());
    } else {
        if (IsRandomOptionItem(*reward_data) &&
            GlobalData::g_randomOptionHandle->give_option(
                static_cast<unsigned long>(reward.item_id_),
                static_cast<ENUM_RARITY>(reward_data->get_rarity()),
                reward_data->getUsableLevel(),
                reward_data->getItemGroupName(),
                GetRandomOptionGrade(*reward_data),
                &reward.chronicle_random_option_.random_option_) != 1) {
            return kLotteryInvalidReward;
        }

        inserted_slot = getCurCharacInvenW()->insertItemIntoInventory(
            reward, static_cast<eItemAddReason>(kLotteryAddReason), true, true);
        if (inserted_slot == -1) {
            return kLotteryCapacityFailure;
        }
        if (source_item_id == kLotteryDelayedDeleteBox &&
            getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(kLotteryInventoryStackable),
                source_slot, 1,
                static_cast<eItemDelReason>(kLotteryDeleteReason), true) != 1) {
            return kLotterySourceDeleteFailure;
        }

        reported_add_info = reward.get_add_info();
        int sell_price = 0;
        G_Store()->GetSellItemPrice(reward, reward_data, reward.avatar_select_,
                                    false, sell_price);
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(kLotteryValueStatisticField), this,
            static_cast<unsigned int>(sell_price));
    }

    if (IsOldLotteryItem(source_item_id) != 0) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            this, static_cast<APSystem::_ActionGroupIndex>(4), 0,
            static_cast<ENUM_PACKETCLASS>(0));
    }

    lottery_cost = G_CDataManager()->find_lottery_use_cost(lottery_cost_key);
    if (lottery_cost != 0) {
        getCurCharacInvenW()->use_money(
            lottery_cost, static_cast<eMoneySubReason>(kLotteryMoneySubReason),
            true);
        PacketGuard money_packet;
        money_packet.clear();
        money_packet.put_header(0, 14);
        money_packet.put_byte(0);
        money_packet.put_short(1);
        getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(kLotteryInventoryStackable), 0, money_packet);
        money_packet.finalize(true);
        Send(money_packet);
    }

    SendLotteryRewardPacket(*this, source_slot, inserted_slot, reward,
                            reported_add_info);

    if (reward.GetUpgrade() != 0) {
        history_log().UpgradeItem(reward.item_id_, 0, reward.GetUpgrade(), true);
    }

    if (source_data->is_stackable() == 1 && GetItemType(*source_data) == 13 &&
        NeedsLotteryItemBroadcast(G_CDataManager()->get_lottery_upgrade_policy(),
                                  *reward_data, reward) &&
        getMoveSpace() != 1 && !IsRandomOptionItem(*reward_data)) {
        PacketGuard broadcast;
        broadcast.put_header(0, 86);
        broadcast.put_byte(2);
        broadcast.put_byte(1);
        broadcast.put_short(get_unique_id());
        broadcast.put_int(reward.item_id_);
        broadcast.put_byte(reward.GetUpgrade());
        broadcast.finalize(true);
        G_GameWorld()->send_all(broadcast);
    }

    return 0;
}
