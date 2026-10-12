#include "old_game/player/core/cuser.h"

#include <utility>
#include <vector>

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/in_game_advertisement_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cbooster_gage_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const int kMultiBoxErrorInvalidItem = 17;
const int kMultiBoxErrorNoCapacity = 19;
const int kMultiBoxErrorRewardCapacity = 4;
const int kInventoryStackable = 1;
const int kInventoryAvatar = 2;
const int kInventoryCreature = 5;
const int kItemDeleteLottery = 3;
const int kItemAddLottery = 3;
const int kAvatarAddLottery = 7;
const int kEventCoinHistoryLottery = 2;
const int kValueStatisticLottery = 7;

bool IsAvatarItem(const CItem& item) {
    return item.vtable_->is_avatar_item(&item);
}

bool IsCreatureItem(const CItem& item) {
    return item.vtable_->is_creature_item(&item);
}

int ItemType(const CItem& item) {
    return item.vtable_->get_item_type(&item);
}

CBoosterGage* GetBoosterGage(const CUser& user) {
    return static_cast<CBoosterGage*>(user.GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(17)));
}

}  // namespace

// IDA_EA: 0x8675EDC
// IDA_NAME: _ZN5CUser20check_multibox_errorEi
// DEMANGLED: CUser::check_multibox_error(int)
int CUser::check_multibox_error(int required_empty_slots) {
    const CInventory* const inventory = getCurCharacInvenR();
    const Inven_Item::ITEM_TYPE checked_types[] = {
        static_cast<Inven_Item::ITEM_TYPE>(1),
        static_cast<Inven_Item::ITEM_TYPE>(2),
        static_cast<Inven_Item::ITEM_TYPE>(3),
        static_cast<Inven_Item::ITEM_TYPE>(10),
        static_cast<Inven_Item::ITEM_TYPE>(5),
    };

    for (unsigned int index = 0;
         index != sizeof(checked_types) / sizeof(checked_types[0]); ++index) {
        if (inventory->check_empty_count(checked_types[index],
                                         required_empty_slots) != 1) {
            return kMultiBoxErrorNoCapacity;
        }
    }
    return 0;
}

// IDA_EA: 0x8675FD2
// IDA_NAME: _ZN5CUser27check_locked_multibox_errorEPK5CItemib
// DEMANGLED: CUser::check_locked_multibox_error(CItem const*,int,bool)
int CUser::check_locked_multibox_error(const CItem* box,
                                       int material_slot,
                                       bool booster_is_full) {
    const CInventory* const inventory = getCurCharacInvenR();
    const Inven_Item material = inventory->GetInvenSlot(
        kInventoryStackable, material_slot);
    CItem* const material_data = G_CDataManager()->find_item(material.item_id_);

    if (!box || !material_data || box->is_stackable() != 1 ||
        ItemType(*box) != 26 || material_data->is_stackable() != 1 ||
        ItemType(*material_data) != 1) {
        return kMultiBoxErrorInvalidItem;
    }

    const MultiBoxLotteryInfo& lottery_info =
        static_cast<const CStackableItem*>(box)->multibox_lottery_info();
    if (lottery_info.required_material_item_id() <= 0 ||
        lottery_info.required_material_count() == 0u ||
        material_data->get_index() !=
            lottery_info.required_material_item_id() ||
        material.get_add_info() <
            static_cast<int>(lottery_info.required_material_count())) {
        return kMultiBoxErrorInvalidItem;
    }

    int required_empty_slots = 2;
    if (booster_is_full) {
        required_empty_slots *= G_CDataManager()->GetCeraShopEventMultiplier();
    }
    return check_multibox_error(required_empty_slots);
}

// IDA_EA: 0x86761E0
// IDA_NAME: _ZN5CUser29check_unlocked_multibox_errorEPK5CItemi
// DEMANGLED: CUser::check_unlocked_multibox_error(CItem const*,int)
int CUser::check_unlocked_multibox_error(const CItem* box,
                                         int expected_count) {
    if (!box || box->is_stackable() != 1 ||
        (ItemType(*box) != 26 && ItemType(*box) != 27) ||
        expected_count != 0xffff) {
        return kMultiBoxErrorInvalidItem;
    }
    return check_multibox_error(2);
}

// IDA_EA: 0x8676296
// IDA_NAME: _ZN5CUser27check_insert_lottery_rewardEPK5CItemRSt6vectorISt4pairI10Inven_ItembESaIS6_EES9_
// DEMANGLED: CUser::check_insert_lottery_reward(CItem const*,std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> &,std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> &)
int CUser::check_insert_lottery_reward(
    const CItem* box,
    std::vector<std::pair<Inven_Item, bool> >& candidates,
    std::vector<std::pair<Inven_Item, bool> >& accepted) {
    const CInventory* const inventory = getCurCharacInvenR();
    for (std::vector<std::pair<Inven_Item, bool> >::const_iterator it =
             candidates.begin();
         it != candidates.end(); ++it) {
        if (inventory->check_empty_count(
                static_cast<Inven_Item::ITEM_TYPE>(it->first.item_category_),
                1) != 1) {
            return kMultiBoxErrorRewardCapacity;
        }
    }

    for (std::vector<std::pair<Inven_Item, bool> >::iterator it =
             candidates.begin();
         it != candidates.end(); ++it) {
        Inven_Item reward = it->first;
        if (reward.item_id_ == 1) {
            accepted.push_back(std::make_pair(reward, it->second));
            continue;
        }

        CItem* const item = G_CDataManager()->find_item(reward.item_id_);
        if (!item) {
            return kMultiBoxErrorInvalidItem;
        }

        if (IsAvatarItem(*item)) {
            if (inventory->GetRemainCapacity(
                    static_cast<INVEN_TYPE>(kInventoryAvatar),
                    static_cast<Inven_Item::ITEM_TYPE>(0)) <= 0) {
                return kMultiBoxErrorRewardCapacity;
            }
            accepted.push_back(std::make_pair(reward, it->second));
            continue;
        }

        if (IsCreatureItem(*item) &&
            ItemType(*item) == oldsrv::config::kEquipmentTypeCreature) {
            if (reward.item_category_ != kInventoryCreature ||
                !user_creature::CCreatureItemConverter::SetEggItem(&reward) ||
                inventory->tryInsertItemIntoInventory(reward) == -1) {
                return kMultiBoxErrorRewardCapacity;
            }
            accepted.push_back(std::make_pair(reward, it->second));
            continue;
        }

        if (inventory->tryInsertItemIntoInventory(reward) == -1) {
            const_cast<CStackableItem*>(
                static_cast<const CStackableItem*>(box))
                ->get_multibox_default_item(&accepted);
            continue;
        }
        accepted.push_back(std::make_pair(reward, it->second));
    }
    return 0;
}

// IDA_EA: 0x8675AC6
// IDA_NAME: _ZN5CUser27process_use_locked_multiboxEiiRSt6vectorISt4pairI10Inven_ItembESaIS3_EE
// DEMANGLED: CUser::process_use_locked_multibox(int,int,std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> &)
int CUser::process_use_locked_multibox(
    int box_slot,
    int material_slot,
    std::vector<std::pair<Inven_Item, bool> >& rewards) {
    const Inven_Item box_item =
        getCurCharacInvenR()->GetInvenSlot(kInventoryStackable, box_slot);
    CItem* const box = G_CDataManager()->find_item(box_item.item_id_);
    CBoosterGage* const booster = GetBoosterGage(*this);
    const int error = check_locked_multibox_error(
        box, material_slot, booster->check_max_booster_gage(box_item.item_id_));
    if (error != 0) {
        return error;
    }

    CStackableItem* const stackable_box = static_cast<CStackableItem*>(box);
    const MultiBoxLotteryInfo& lottery_info =
        stackable_box->multibox_lottery_info();
    std::vector<std::pair<Inven_Item, bool> > candidates;
    stackable_box->get_multibox_lottery_item(&candidates);
    const int reward_error =
        check_insert_lottery_reward(box, candidates, rewards);
    if (reward_error != 0) {
        return reward_error;
    }

    // Reward construction can allocate and call back into inventory helpers.
    // Revalidate both exact slots immediately before the two-item commit so a
    // stale slot cannot consume a different stack.  The dispatcher serializes
    // this user transaction; after this full preflight neither deletion has a
    // remaining data-dependent failure condition.
    if (box_slot == material_slot) {
        return kMultiBoxErrorInvalidItem;
    }
    const CInventory* const inventory_read = getCurCharacInvenR();
    const Inven_Item current_box = inventory_read->GetInvenSlot(
        kInventoryStackable, box_slot);
    const Inven_Item current_material = inventory_read->GetInvenSlot(
        kInventoryStackable, material_slot);
    const int material_count =
        static_cast<int>(lottery_info.required_material_count());
    if (current_box.item_id_ != box_item.item_id_ ||
        current_box.get_add_info() < 1 ||
        current_material.item_id_ !=
            lottery_info.required_material_item_id() ||
        current_material.get_add_info() < material_count) {
        return kMultiBoxErrorInvalidItem;
    }

    InventoryMemory preview_memory;
    CInventory preview;
    preview.SetInventoryMemory(&preview_memory);
    if (!preview.setCopy(*inventory_read) ||
        preview.delete_item(
            static_cast<INVEN_TYPE>(kInventoryStackable), box_slot, 1,
            static_cast<eItemDelReason>(kItemDeleteLottery), false) != 1 ||
        preview.delete_item(
            static_cast<INVEN_TYPE>(kInventoryStackable), material_slot,
            material_count,
            static_cast<eItemDelReason>(kItemDeleteLottery), false) != 1) {
        return kMultiBoxErrorInvalidItem;
    }

    CInventory* const inventory = getCurCharacInvenW();
    if (inventory->delete_item(
            static_cast<INVEN_TYPE>(kInventoryStackable), box_slot, 1,
            static_cast<eItemDelReason>(kItemDeleteLottery), true) != 1) {
        // The live implementation is expected to fail before mutation, but
        // restore both exact snapshots so that invariant remains true even if
        // a later delete_item implementation changes its failure ordering.
        inventory->update_item(static_cast<INVEN_TYPE>(kInventoryStackable),
                               box_slot, current_box);
        inventory->update_item(static_cast<INVEN_TYPE>(kInventoryStackable),
                               material_slot, current_material);
        return kMultiBoxErrorInvalidItem;
    }
    if (inventory->delete_item(
        static_cast<INVEN_TYPE>(kInventoryStackable),
        material_slot,
        material_count,
        static_cast<eItemDelReason>(kItemDeleteLottery),
        true) != 1) {
        inventory->update_item(static_cast<INVEN_TYPE>(kInventoryStackable),
                               box_slot, current_box);
        inventory->update_item(static_cast<INVEN_TYPE>(kInventoryStackable),
                               material_slot, current_material);
        return kMultiBoxErrorInvalidItem;
    }
    return 0;
}

// IDA_EA: 0x8675D0C
// IDA_NAME: _ZN5CUser29process_use_unlocked_multiboxEiiRSt6vectorISt4pairI10Inven_ItembESaIS3_EE
// DEMANGLED: CUser::process_use_unlocked_multibox(int,int,std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> &)
int CUser::process_use_unlocked_multibox(
    int box_slot,
    int expected_count,
    std::vector<std::pair<Inven_Item, bool> >& rewards) {
    const Inven_Item box_item =
        getCurCharacInvenR()->GetInvenSlot(kInventoryStackable, box_slot);
    CItem* const box = G_CDataManager()->find_item(box_item.item_id_);
    const int error = check_unlocked_multibox_error(box, expected_count);
    if (error != 0) {
        return error;
    }

    CStackableItem* const stackable_box = static_cast<CStackableItem*>(box);
    std::vector<std::pair<Inven_Item, bool> > candidates;
    stackable_box->get_multibox_lottery_item(&candidates);
    const int reward_error =
        check_insert_lottery_reward(box, candidates, rewards);
    if (reward_error != 0) {
        return reward_error;
    }

    const Inven_Item current_box = getCurCharacInvenR()->GetInvenSlot(
        kInventoryStackable, box_slot);
    if (current_box.item_id_ != box_item.item_id_ ||
        current_box.get_add_info() < 1 ||
        getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(kInventoryStackable), box_slot, 1,
            static_cast<eItemDelReason>(kItemDeleteLottery), true) != 1) {
        return kMultiBoxErrorInvalidItem;
    }
    return 0;
}

// IDA_EA: 0x86767D2
// IDA_NAME: _ZN5CUser22use_multibox_statisticEj
// DEMANGLED: CUser::use_multibox_statistic(unsigned int)
void CUser::use_multibox_statistic(unsigned int box_item_id) {
    int box_category = 0;
    switch (box_item_id) {
        case 0x289749u:
            box_category = 2;
            break;
        case 0x28982du:
            SendRandomBoxStatistic(1,
                                   static_cast<ENUM_RANDOMBOX_ACTION_KIND>(1));
            return;
        case 0x380u:
            SendRandomBoxStatistic(0,
                                   static_cast<ENUM_RANDOMBOX_ACTION_KIND>(1));
            return;
    }
    SendRandomBoxStatistic(box_category,
                           static_cast<ENUM_RANDOMBOX_ACTION_KIND>(1));
}

// IDA_EA: 0x8674968
// IDA_NAME: _ZN5CUser21multibox_lottery_itemEii
// DEMANGLED: CUser::multibox_lottery_item(int,int)
int CUser::multibox_lottery_item(int box_slot, int requested_count) {
    const Inven_Item box_item =
        getCurCharacInvenR()->GetInvenSlot(kInventoryStackable, box_slot);
    std::vector<std::pair<Inven_Item, bool> > rewards;

    int error = 0;
    if (box_item.item_id_ == 8289 || box_item.item_id_ == 2660419) {
        error = process_use_unlocked_multibox(box_slot, requested_count, rewards);
    } else {
        error = process_use_locked_multibox(box_slot, requested_count, rewards);
    }
    if (error != 0) {
        if (error == kMultiBoxErrorNoCapacity) {
            GetBoosterGage(*this)->send_error_state_packet(this,
                                                            box_item.item_id_);
            return -1;
        }
        return error;
    }

    CBoosterGage* const booster = GetBoosterGage(*this);
    const bool booster_bonus =
        booster->cal_booster_gage(box_item.item_id_, this) != 0;
    if (booster_bonus) {
        booster->log_booster_gage_bonus_reward(this, rewards);
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_RANDOMBOX_ITEM);
    packet.put_byte(1);
    packet.put_short(box_slot);
    packet.put_short(requested_count);
    packet.put_short(static_cast<int>(rewards.size()));

    BoosterInsertedSlotList inserted_slots;
    for (std::vector<std::pair<Inven_Item, bool> >::iterator it =
             rewards.begin();
         it != rewards.end(); ++it) {
        Inven_Item& reward = it->first;
        int inserted_slot = -1;
        int reported_add_info = 0;

        if (reward.item_id_ == 1) {
            if (reward.add_info_ > 0 && reward.add_info_ <= 999) {
                int actual_added_coin = reward.add_info_;
                if (booster_bonus) {
                    actual_added_coin *=
                        G_CDataManager()->GetCeraShopEventMultiplier();
                }
                CInventory* const inventory = getCurCharacInvenW();
                inventory->SetEventCoin(inventory->GetEventCoin() +
                                        actual_added_coin);
                history_log().EventCoinAdd(
                    inventory->GetEventCoin(),
                    actual_added_coin,
                    static_cast<eCoinAddReason>(kEventCoinHistoryLottery));
                SaveMoneyCoin();
                SendCoin();
                packet.put_short(-1);
                packet.put_int(reward.item_id_);
                packet.put_int(reward.add_info_);
                packet.put_short(reward.avatar_select_);
                packet.put_byte(reward.GetItemAttr());
                packet.put_packet(reward);
            }
            continue;
        }

        CItem* const item = G_CDataManager()->find_item(reward.item_id_);
        if (!item) {
            return kMultiBoxErrorInvalidItem;
        }

        if (IsAvatarItem(*item)) {
            char agency_no[24] = {};
            reward.SetUpgrade(0);
            WongWork::CGenUniqueNo::genIPGNo(
                static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(1),
                static_cast<unsigned int>(getCurCharacNo()), agency_no);
            inserted_slot = booster->AddAvatarItem(
                inserted_slots,
                booster_bonus,
                this,
                reward,
                item->getUsablePeriod(),
                false,
                0,
                static_cast<char>(-1),
                agency_no,
                static_cast<eAvatarItemAddReason>(kAvatarAddLottery),
                reported_add_info);
        } else if (IsCreatureItem(*item) &&
                   ItemType(*item) ==
                       oldsrv::config::kEquipmentTypeCreature) {
            if (reward.item_category_ == kInventoryCreature &&
                user_creature::CCreatureItemConverter::SetEggItem(&reward)) {
                char agency_no[24] = {};
                WongWork::CGenUniqueNo::genIPGNo(
                    static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(1),
                    static_cast<unsigned int>(getCurCharacNo()), agency_no);
                inserted_slot = booster->InsertCreatureItem(
                    inserted_slots,
                    booster_bonus,
                    this,
                    reward,
                    static_cast<eItemAddReason>(kItemAddLottery),
                    true,
                    false,
                    item->getUsablePeriod(),
                    item->getExpirationDate(),
                    agency_no);
            }
        } else {
            int special_type = 0;
            int special_value = 0;
            bool unique = false;
            if (GlobalData::s_pCeraShop->IsSpecialItem(
                    reward.item_id_, special_type, special_value, unique) != 0 &&
                ItemType(*item) != 16 && ItemType(*item) != 34) {
                int processing_result = 0;
                const int restrict_code =
                    G_CDataManager()->get_item_list()->GetRestrictCode(
                        reward.item_id_);
                const long current_seconds =
                    GlobalData::system_time()->getCurSec();
                std::vector<Inven_Item> output_items;
                WongWork::CCeraShop::stCeraShopItemParam_t item_param;
                if (GlobalData::s_pCeraShop->ProcessSpecialItem(
                        this,
                        reward.item_id_,
                        restrict_code,
                        processing_result,
                        current_seconds,
                        true,
                        output_items,
                        item_param,
                        0,
                        0,
                        booster_bonus,
                        1) != 0) {
                    if (processing_result != 0) {
                        inserted_slot = -1;
                    }
                    if (restrict_code != 0 &&
                        !IsRestrictedGoods(restrict_code)) {
                        SetRestrictedGoods(restrict_code);
                        GlobalData::s_pCeraShop->SaveFeaturedIdx(
                            this, restrict_code, current_seconds, 0);
                        SendRestrictedGoods(true, restrict_code);
                    }
                }
                inserted_slot = 0xffff;
            } else {
                inserted_slot = booster->insertItemIntoInventory(
                    inserted_slots,
                    booster_bonus,
                    this,
                    reward,
                    static_cast<eItemAddReason>(kItemAddLottery),
                    true);
            }

            if (inserted_slot != -1) {
                reported_add_info = reward.get_add_info();
                int sell_price = 0;
                G_Store()->GetSellItemPrice(reward,
                                             item,
                                             reward.avatar_select_,
                                             false,
                                             sell_price);
                if (booster_bonus) {
                    sell_price *= G_CDataManager()->GetCeraShopEventMultiplier();
                }
                GetInstanceValueStatistic()->AddValueStatistic(
                    static_cast<VALUE_STATISTIC_FIELD>(kValueStatisticLottery),
                    this,
                    static_cast<unsigned int>(sell_price));
            }
        }

        if (inserted_slot == -1) {
            packet.put_short(-1);
            packet.put_int(reward.item_id_);
            packet.put_int(reward.get_add_info());
        } else {
            packet.put_short(inserted_slot);
            packet.put_int(reward.item_id_);
            packet.put_int(reported_add_info);
        }
        packet.put_short(reward.avatar_select_);
        packet.put_byte(reward.GetItemAttr());
        packet.put_packet(reward);
    }

    packet.put_byte(booster_bonus ? 1 : 0);
    packet.finalize(true);
    Send(packet);

    for (BoosterInsertedSlotList::const_iterator it = inserted_slots.begin();
         it != inserted_slots.end(); ++it) {
        SendUpdateItem(static_cast<eSendTarget>(1),
                       static_cast<ENUM_ITEMSPACE>(it->second),
                       it->first);
    }
    InGameADManagerInstance()->CheckFailMultiBoxLottery(this, rewards);

    for (std::vector<std::pair<Inven_Item, bool> >::const_iterator it =
             rewards.begin();
         it != rewards.end(); ++it) {
        if (!it->second) {
            continue;
        }
        PacketGuard broadcast;
        broadcast.put_header(0, 86);
        broadcast.put_byte(4);
        broadcast.put_byte(1);
        broadcast.put_short(get_unique_id());
        broadcast.put_int(it->first.item_id_);
        broadcast.put_int(box_item.item_id_);
        broadcast.put_byte(it->first.GetUpgrade());
        broadcast.finalize(true);
        G_GameWorld()->send_all(broadcast);
    }

    use_multibox_statistic(box_item.item_id_);
    return 0;
}
