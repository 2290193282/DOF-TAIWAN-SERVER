#include "old_game/social/party/party.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x85B407C
// IDA_NAME: _ZN6CParty24_checkGoldCardSelectableEP5CUser22eClearRewardCardType_ti
// DEMANGLED: CParty::_checkGoldCardSelectable(CUser *,eClearRewardCardType_t,int)
int CParty::_checkGoldCardSelectable(CUser* user,
                                     eClearRewardCardType_t card_type,
                                     int member_slot) {
    const unsigned int type = static_cast<unsigned int>(card_type);
    if (!user || type >= 2u || static_cast<unsigned int>(member_slot) >= 4u) {
        return 0;
    }

    const int cost = play_result_values_[type][member_slot];
    if (cost == 0) {
        return 1;
    }
    if (user->getCurCharacInvenW()->use_money(
            cost, static_cast<eMoneySubReason>(20), true) == 0) {
        return 0;
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             0);
    return 1;
}

// Shared extraction of the verified type-1 post-selection block at
// CParty::SetCardNumber+0x8d.  Keeping this as one owner prevents the free
// ChampionBreak row from bypassing capacity/AP behavior.
void CParty::ApplyGoldCardSelectionSideEffectsForRuntime(CUser* user,
                                                         int member_slot) {
    CEventBase* const reward_event =
        GlobalData::s_event_manager->GetRepeatEvent(65);
    Inven_Item& gold_reward =
        battle_data_.event_reward_item(member_slot, 1);
    if (reward_event->IsEventing(0) != 1) {
        if (user->getCurCharacInvenR()->tryInsertItemIntoInventory(
                gold_reward) == -1) {
            gold_reward.item_id_ = 0;
            gold_reward.set_add_info(0);
        } else {
            CEventBase* const space_event =
                GlobalData::s_event_manager->GetRepeatEvent(64);
            if ((space_event->IsEventing(0) || IsReturnUserParty() ||
                 IsEventCharacParty()) &&
                gold_reward.item_category_ == 1u &&
                user->getCurCharacInvenW()->check_empty_count(
                    static_cast<Inven_Item::ITEM_TYPE>(1), 2) != 1) {
                gold_reward.item_id_ = 0;
                gold_reward.set_add_info(0);
            }
        }
    }

    APSystem::CUserProc::ClearActionAndSendtoUser(
        user,
        static_cast<APSystem::_ActionGroupIndex>(8),
        0,
        ENUM_PACKETCLASS_NOTIPACKET);
    const int item_id = gold_reward.getKey();
    if (item_id != 0) {
        CItem* const item = G_CDataManager()->find_item(item_id);
        if (item && static_cast<unsigned int>(item->get_rarity()) <= 4u) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                user,
                static_cast<APSystem::_ActionGroupIndex>(21),
                0,
                ENUM_PACKETCLASS_NOTIPACKET);
        }
    }
}

// IDA_EA: 0x85B415A
// IDA_NAME: _ZN6CParty13SetCardNumberEP5CUserc22eClearRewardCardType_t
// DEMANGLED: CParty::SetCardNumber(CUser *,char,eClearRewardCardType_t)
int CParty::SetCardNumber(CUser* user,
                          char card,
                          eClearRewardCardType_t card_type) {
    const int member_slot = GetMemberSlotNo(user);
    if (member_slot == -1 || card > 3) {
        return 0;
    }
    if (HaveAlreadySelectedCard(static_cast<char>(member_slot), card_type)) {
        return 0;
    }

    const int type = static_cast<int>(card_type);
    if (selected_cards_[type][card] != 0xffu) {
        return 0;
    }
    if (_checkGoldCardSelectable(user, card_type, member_slot) != 1) {
        return 0;
    }
    selected_cards_[type][card] = static_cast<uint8_t>(member_slot);

    if (type == 1) {
        ApplyGoldCardSelectionSideEffectsForRuntime(user, member_slot);
    }
    return 1;
}

// IDA_EA: 0x85B44D6
// IDA_NAME: _ZN6CParty16SendEachCardInfoEv
// DEMANGLED: CParty::SendEachCardInfo(void)
void CParty::SendEachCardInfo() {
    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "void CParty::SendEachCardInfo()",
        0,
        "[CARD_FLOW][CMD71_BUILD] party=%d state=%d normal=%u,%u,%u,%u gold=%u,%u,%u,%u",
        GetPartyIndex(),
        get_dungeon_clear_state(),
        static_cast<unsigned int>(selected_cards_[0][0]),
        static_cast<unsigned int>(selected_cards_[0][1]),
        static_cast<unsigned int>(selected_cards_[0][2]),
        static_cast<unsigned int>(selected_cards_[0][3]),
        static_cast<unsigned int>(selected_cards_[1][0]),
        static_cast<unsigned int>(selected_cards_[1][1]),
        static_cast<unsigned int>(selected_cards_[1][2]),
        static_cast<unsigned int>(selected_cards_[1][3]));
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_SELECT_CARD);
    packet.put_byte(1);

    for (int card = 0; card <= 3; ++card) {
        packet.put_byte(static_cast<signed char>(selected_cards_[0][card]));
        const uint8_t gold_member = selected_cards_[1][card];
        packet.put_byte(static_cast<signed char>(gold_member));
        if (gold_member == 0xffu) {
            packet.put_byte(0);
        } else {
            int packet_index = packet.get_index();
            packet.put_byte(1);
            packet.put_int(0);

            const Inven_Item& normal_reward =
                battle_data_.event_reward_item(gold_member, 0);
            const Inven_Item& gold_reward =
                battle_data_.event_reward_item(gold_member, 1);
            packet.put_int(normal_reward.get_add_info());
            const int item_id = gold_reward.getKey();
            if (item_id != 0) {
                packet.put_int(item_id);
                packet.put_int(gold_reward.get_add_info());
                packet.put_byte(packet_index, 2);
            }
        }
        packet.put_byte(battle_data_.card_info_member_state(card));
    }

    // CLIENT_IDA_EVIDENCE: CMD71 handler sub_557000 @ 0x557026..0x55713F
    // consumes eight card rows even though the official CParty still owns four
    // real member seats.  The remaining rows are fixed client UI slots.
    for (int card = 4; card < 8; ++card) {
        packet.put_byte(static_cast<signed char>(-1));
        packet.put_byte(static_cast<signed char>(-1));
        packet.put_byte(0);
        packet.put_byte(0);
    }

    packet.finalize(true);
    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "void CParty::SendEachCardInfo()",
        0,
        "[CARD_FLOW][CMD71_READY] party=%d state=%d len=%d",
        GetPartyIndex(),
        get_dungeon_clear_state(),
        packet.get()->get_len());
    send_to_party(packet);
}

int CParty::GetSelectedCardOwnerForRuntime(int card_type, int card) const {
    if (card_type < 0 || card_type >= 2 || card < 0 || card >= 4) {
        return -1;
    }
    return static_cast<signed char>(selected_cards_[card_type][card]);
}

int CParty::SetChampionCardNumberForRuntime(CUser* user, char card) {
    const int member_slot = GetMemberSlotNo(user);
    if (member_slot < 0 || member_slot >= 4 || card < 0 || card >= 4) {
        return 0;
    }
    const eClearRewardCardType_t champion_type =
        static_cast<eClearRewardCardType_t>(1);
    if (HaveAlreadySelectedCard(static_cast<char>(member_slot),
                                champion_type) != 0 ||
        selected_cards_[1][static_cast<unsigned char>(card)] != 0xffu) {
        return 0;
    }

    // CLIENT_IDA_EVIDENCE: state-9 type 1 is the ChampionBreak lower row.
    // It retains the ordinary gold-card contents, but the ChampionBreak right
    // is free; deliberately do not call _checkGoldCardSelectable here.
    selected_cards_[1][static_cast<unsigned char>(card)] =
        static_cast<uint8_t>(member_slot);
    // ChampionBreak removes only the paid-card fee.  The ordinary type-1
    // capacity preflight, reward clearing and AP notifications still own the
    // selected lower-row reward, so reuse them after recording the owner.
    ApplyGoldCardSelectionSideEffectsForRuntime(user, member_slot);
    return 1;
}
