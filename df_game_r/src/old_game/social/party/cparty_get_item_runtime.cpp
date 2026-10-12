#include "old_game/social/party/party.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "old_game/economy/inventory/special_item_routing_manager_abi.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_game/player/core/cuser.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

extern "C++" int GetRandomRange(int minimum, int maximum);

namespace {

void SendPickupError(CUser* user, int error) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
    packet.put_byte(0);
    packet.put_byte(static_cast<unsigned char>(error));
    packet.finalize(true);
    user->Send(packet);
}

int GetPickupItemCategory(const CItem& item) {
    if (item.is_stackable()) {
        return item.vtable_->get_item_type(&item) == 9 ? 7 : 0;
    }
    if (item.vtable_->is_creature_item(&item)) return 7;
    return item.vtable_->is_avatar_item(&item) ? 1 : 0;
}

int InsertPickupItem(CUser* user, const Inven_Item& item) {
    if (item.isAvatarItemType()) {
        char ipg_no[24] = {0};
        WongWork::CGenUniqueNo::genIPGNo(
            static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(5),
            static_cast<unsigned int>(user->getCurCharacNo()),
            ipg_no);
        return user->addAvatarItem(
            static_cast<unsigned long>(item.getKey()),
            0,
            static_cast<char>(item.avatar_select_),
            ipg_no,
            static_cast<eAvatarItemAddReason>(15));
    }
    return user->getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(4), true, true);
}

int GetPickupGoldEquipmentBonus(CUser* user,
                                bool generated_gold,
                                int base_money) {
    const Inven_Item equipment =
        user->getCurCharacInvenR()->GetInvenSlot(0, 0);
    // IDA_EA: CParty::get_item@0x85A3B98 applies the 24191 ten-percent
    // equipment bonus in both money branches (raw lines 5491..5503 and
    // 5716..5727); the later switch adds the remaining per-key rates.
    int rate = 0;
    switch (equipment.getKey()) {
    case 24191:
        return static_cast<int>(static_cast<long double>(base_money) * 0.1L);
    case 26115:
    case 26341:
    case 26626:
        rate = 3;
        break;
    case 26146:
    case 26342:
    case 26627:
    case 26786:
        rate = 4;
        break;
    case 2679971:
    case 2679974:
        rate = generated_gold ? 0 : 3;
        break;
    case 2679976:
        rate = generated_gold ? 0 : 5;
        break;
    default:
        break;
    }
    return rate * base_money / 100;
}

int ApplyPickupGoldBonuses(CParty* party,
                           CUser* user,
                           bool routing_eligible,
                           bool generated_gold,
                           int base_money,
                           int& manage_bonus) {
    manage_bonus = 0;
    if (!routing_eligible) {
        return 0;
    }

    int bonus = 0;
    const user_creature::CCreatureMgr* const creature =
        user->getCurCharacInvenR()->GetCreatureMgrR();
    if (creature) {
        bonus += creature->GetExtraGold(base_money);
        bonus += creature->GetOverSkillExtraGold(base_money);
    }
    if (base_money > 0) {
        bonus += GetPickupGoldEquipmentBonus(user, generated_gold, base_money);
    }
    manage_bonus = static_cast<int>(party->CalcurateManageBonus(
        ManageBonusType::Gold,
        user,
        static_cast<unsigned int>(base_money + bonus)));
    return bonus + manage_bonus;
}

int ApplyPickupGuildFund(CUser* user,
                         bool routing_eligible,
                         bool compare_upper_to_credited_money,
                         int base_money,
                         int& credited_money) {
    if (!routing_eligible || user->get_charac_guildkey() == 0 ||
        user->IsCompleteGuildFund() == 1) {
        return 0;
    }

    const GuildParameterScript* const parameters =
        G_CDataManager()->GetGuildParameterScriptForBuild();
    int guild_fund = 0;
    // IDA_EA: CParty::get_item@0x85A3B98 maps key 0's fixed-range check to
    // condition[0] <= base (n499999_10) and condition[1] >= post-bonus
    // credited money (n499999_9).  Key 1199 compares both bounds with its
    // generated base (n499999_2).
    const int upper_comparison = compare_upper_to_credited_money
                                     ? credited_money
                                     : base_money;
    if (parameters->guild_fund_condition[0] <= base_money &&
        parameters->guild_fund_condition[1] >= upper_comparison) {
        guild_fund = parameters->guild_fund_condition[2];
    } else if (parameters->guild_fund_condition[1] < base_money) {
        guild_fund = static_cast<int>(
            parameters->guild_fund_condition_weight *
            static_cast<double>(base_money));
    }

    if (credited_money >= guild_fund && guild_fund > 0) {
        credited_money -= guild_fund;
    }
    // IDA_EA: CParty::get_item@0x85A3B98 sends every positive computed
    // contribution even when a malformed parameter makes it exceed the
    // credited amount; only the deduction itself has the amount guard.
    if (guild_fund > 0) {
        CGuildServerProxy* const guild_proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        guild_proxy->SendAddGuildFund(
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<unsigned int>(user->get_charac_guildkey()),
            guild_fund,
            Packet_Add_Guild_Fund::kFundTypePartyReward);
    }
    return guild_fund;
}

}  // namespace

// IDA_EA: 0x85A3B98
// IDA_NAME: _ZN6CParty8get_itemEP5CUseribcRiRhRb
// DEMANGLED: CParty::get_item(CUser *,int,bool,char,int &,unsigned char &,bool &)
int CParty::get_item(CUser* user,
                     int unique_id,
                     bool client_flag,
                     char client_rarity,
                     int& item_id,
                     unsigned char& upgrade,
                     bool& stackable) {
    item_id = -1;
    upgrade = 0;
    stackable = false;
    if (!user) return 0;

    map_item dropped_item;
    if (battle_field().get_field_item(unique_id, dropped_item) != 1) {
        SendPickupError(user, client_rarity <= 3 ? 21 : 81);
        return 0;
    }

    PacketGuard packet;
    const int pickup_item_key = dropped_item.item.getKey();
    // CLIENT_IDA_EVIDENCE: sub_5A4A20@0x5A4C3E/0x5A4C61 routes
    // decoded keys 0 and 1199 to the eight-record money grammar.
    if (pickup_item_key == 0 || pickup_item_key == 1199) {
        const bool generated_gold = pickup_item_key == 1199;
        if (get_member_count() <= 0) {
            SendPickupError(user, 21);
            return 0;
        }

        int shared_money = 0;
        int picker_money = 0;
        WongWork::CGeneratorCommonData* generated_gold_data = 0;
        if (generated_gold) {
            generated_gold_data = G_CDataManager()
                                      ->getItemGenerator()
                                      ->getGenerator(
                                          WongWork::CItemGeneratorMgr::
                                              kClearReward)
                                      ->getGeneratorCommonData();
        } else {
            const int live_count = get_live_count_enter_map();
            if (live_count <= 0) {
                SendPickupError(user, 21);
                return 0;
            }
            const int total_money = dropped_item.item.get_add_info();
            shared_money = total_money / live_count;
            picker_money = shared_money + total_money % live_count;
        }

        packet.put_header(0, ENUM_NOTIPACKET_GET_ITEM);
        packet.put_short(static_cast<int16_t>(unique_id));
        packet.put_short(user->get_unique_id());
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const member = get_user(seat);
            if (!member || _checkValidUser(seat) != 1) {
                packet.put_byte(0);
                packet.put_int(0);
                continue;
            }

            packet.put_byte(1);
            int requested = 0;
            if (generated_gold) {
                WongWork::GoldGenerateRange* const gold_range =
                    generated_gold_data->getGoldGenTable(
                        static_cast<unsigned char>(
                            member->get_charac_level()));
                requested = gold_range->minimum +
                    GetRandomRange(-gold_range->maximum,
                                   gold_range->maximum) *
                        gold_range->minimum / 100;
            } else {
                requested = member == user ? picker_money : shared_money;
            }
            if (battle_data().member_live(seat) != 1) requested = 0;
            int manage_bonus = 0;
            const int bonus = ApplyPickupGoldBonuses(
                this,
                member,
                dropped_item.routing_eligible_ != 0,
                generated_gold,
                requested,
                manage_bonus);
            int credited_money = requested + bonus;
            const int guild_fund = ApplyPickupGuildFund(
                member,
                dropped_item.routing_eligible_ != 0,
                !generated_gold,
                requested,
                credited_money);
            const int gained = member->getCurCharacInvenW()->gain_money(
                credited_money,
                static_cast<eMoneyAddReason>(generated_gold ? 26 : 4),
                true,
                manage_bonus);
            // IDA_EA: CParty::get_item@0x85A3B98 calls gain_money first and
            // then saves the post-fund credited amount for upper-member tax
            // (raw lines 5550..5558 / 5775..5783).
            if (dropped_item.routing_eligible_) {
                member->saveTaxMoneyForUpperMember(credited_money);
            }
            const int actual = gained < 0 ? 0 : gained;
            if (actual < credited_money) {
                member->SendMoneyFullReason(
                    static_cast<ENUM_MONEY_FULL_REASON>(0),
                    static_cast<unsigned long>(credited_money),
                    static_cast<unsigned long>(actual));
            }
            // IDA_EA: raw key 0 writes gain_money's (possibly clamped) return
            // at line 5564, while key 1199 deliberately writes credited_money
            // at line 5789.  Both old branches then write count=1, one
            // aggregate bonus component, and the guild-fund contribution.
            packet.put_int(generated_gold ? credited_money : actual);
            packet.put_byte(1);
            packet.put_int(bonus);
            packet.put_int(guild_fund);
        }
        // CLIENT_IDA_EVIDENCE: sub_5A4A20@0x5A5060..0x5A528E loops
        // over eight wire records; ordinary CParty owns only slots 0..3.
        for (int seat = 4; seat < 8; ++seat) {
            packet.put_byte(0);
            packet.put_int(0);
        }
        const int pickup_error = battle_field().pickup_item(unique_id);
        if (pickup_error != 0) {
            SendPickupError(user, pickup_error);
            return 0;
        }
        packet.finalize(true);
        send_to_party(packet);
        if (dropped_item.routing_eligible_) item_id = dropped_item.item.getKey();
        return 1;
    }

    CItem* const item = G_CDataManager()->find_item(dropped_item.item.getKey());
    if (!item) return 0;
    stackable = item->is_stackable() != 0;

    const bool group_pickup = get_member_count() > 1 &&
                              get_live_count_enter_map() > 1 &&
                              dropped_item.routing_eligible_;
    if (group_pickup && (item->get_rarity() == 4 || item->IsRoutingItem())) {
        CSpecialItemRoutingManager* const routing =
            G_CGameManager()->GetSpecialItemRoutingManager();
        const int error = routing->StartRouting(user, dropped_item.item, unique_id);
        if (error != 0) {
            SendPickupError(user, error);
            return 0;
        }
        return 1;
    }

    CUser* winner = user;
    unsigned char dice[4] = {0, 0, 0, 0};
    if (group_pickup && item->get_rarity() > 0) {
        int highest_dice = 0;
        winner = 0;
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const member = get_user(seat);
            if (!member || _checkValidUser(seat) != 1 ||
                battle_data().member_live(seat) != 1 ||
                member->getCurCharacInvenR()->tryInsertItemIntoInventory(
                    dropped_item.item) == -1) {
                continue;
            }
            dice[seat] = static_cast<unsigned char>(get_rand_int(100) + 1);
            if (dice[seat] > highest_dice) {
                highest_dice = dice[seat];
                winner = member;
            }
        }
        if (!winner) {
            SendPickupError(user, 4);
            return 0;
        }
    }

    const int inserted = InsertPickupItem(winner, dropped_item.item);
    if (inserted == -1) {
        SendPickupError(user, 4);
        return 0;
    }
    const int pickup_error = battle_field().pickup_item(unique_id);
    if (pickup_error != 0) {
        SendPickupError(user, pickup_error);
        return 0;
    }

    // 0x85A6202..0x85A6228 calls this only after the map item has been
    // removed successfully.  The two physical Inven_Item offsets passed by
    // the old body are item_id (+0x02) and add_info (+0x07).
    _onGetItem(user,
               static_cast<unsigned int>(dropped_item.item.item_id_),
               static_cast<unsigned int>(dropped_item.item.add_info_));

    packet.put_header(0, ENUM_NOTIPACKET_GET_ITEM);
    packet.put_short(static_cast<int16_t>(unique_id));
    packet.put_short(user->get_unique_id());
    for (int seat = 0; seat <= 3; ++seat) packet.put_byte(dice[seat]);
    // CLIENT_IDA_EVIDENCE: sub_5A4A20@0x5A4C80..0x5A4CC0 consumes
    // eight wire rolls; ordinary CParty owns only slots 0..3.
    for (int seat = 4; seat < 8; ++seat) packet.put_byte(0);
    packet.put_short(winner->get_unique_id());
    packet.put_short(static_cast<int16_t>(inserted));
    packet.put_byte(GetPickupItemCategory(*item));
    packet.finalize(true);
    send_to_party(packet);

    if (dropped_item.routing_eligible_) {
        item_id = dropped_item.item.getKey();
        upgrade = dropped_item.item.GetUpgrade();
    }
    return 1;
}

// IDA_EA: 0x85B949C
// IDA_NAME: _ZN6CParty10_onGetItemEP5CUserjj
// DEMANGLED: CParty::_onGetItem(CUser *,unsigned int,unsigned int)
void CParty::_onGetItem(CUser* user,
                        unsigned int item_id,
                        unsigned int item_count) {
    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(item_id));
    if (!item) {
        return;
    }

    EventClassify::CEventScriptMng* const event_scripts =
        G_CDataManager()->get_event_script_mng();
    if (event_scripts->is_eventing(0x193)) {
        event_scripts->process_add_item_count_db(
            user, static_cast<int>(item_id), static_cast<int>(item_count));
    }

    if (item->is_stackable() == 0 ||
        item->vtable_->get_item_type(item) != 26) {
        return;
    }

    int box_category = 0;
    if (item_id == 0x289749u) {
        box_category = 2;
    } else if (item_id == 0x28982du) {
        box_category = 1;
    }
    CUser::SendRandomBoxStatistic(
        box_category, static_cast<ENUM_RANDOMBOX_ACTION_KIND>(0));
}
