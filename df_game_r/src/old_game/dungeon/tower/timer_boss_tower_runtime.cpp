#include "old_game/dungeon/tower/timer_boss_tower_runtime.h"

#include <cstring>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kBossTowerClearTimerMessage = 163;
const int kBossTowerReturnTimerMessage = 164;
const int kBossTowerReadyTimerMessage = 165;

void SendUnclaimedBossTowerReward(CUser* user, const Inven_Item& reward) {
    char title[30] = {};
    char text[256] = {};
    char ipg_no[24] = {};

    CItem* const item = G_CDataManager()->find_item(reward.item_id_);
    if (!item) {
        return;
    }

    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "chn_game_server_msg_21", 0),
                 0x1du);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "chn_game_server_msg_22", 0),
                 0xffu);

    int expiration_time = 0;
    if (item->vtable_->is_avatar_item(item)) {
        const int usable_period = item->getUsablePeriod();
        if (usable_period && usable_period != 365) {
            expiration_time =
                86400 * usable_period + GlobalData::s_systemTime_.getCurSec();
        }
    }

    // The old body performs this vtable test twice. It preserves the same
    // avatar/system-mail split after deriving the expiration timestamp.
    if (item->vtable_->is_avatar_item(item)) {
        const char* const generated_ipg_no = WongWork::CGenUniqueNo::genIPGNo(
            static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(3),
            static_cast<unsigned int>(user->getCurCharacNo()), ipg_no);
        const char endurance = static_cast<char>(
            reinterpret_cast<const CEquipItem*>(item)->get_endurance());
        WongWork::CMailBoxHelper::ReqDBSendNewAvatarMail(
            user,
            user->getCurCharacNo(),
            reward.item_id_,
            expiration_time,
            endurance,
            0,
            generated_ipg_no,
            text,
            static_cast<int>(std::strlen(text)));
        return;
    }

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        reward,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        15,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

}  // namespace

// IDA_EA: 0x863D0F6
// IDA_NAME: _ZN17Timer_TowerOnTimeC2Ev
// DEMANGLED: Timer_TowerOnTime::Timer_TowerOnTime(void)
Timer_TowerOnTime::Timer_TowerOnTime() : TimerSig() {}

// IDA_EA: 0x863E4D8 (D2), 0x863E508 (D0)
// IDA_NAME: _ZN17Timer_TowerOnTimeD2Ev
// IDA_NAME: _ZN17Timer_TowerOnTimeD0Ev
// DEMANGLED: Timer_TowerOnTime::~Timer_TowerOnTime()
Timer_TowerOnTime::~Timer_TowerOnTime() {}

// IDA_EA: 0x863B8C4
// IDA_NAME: _ZN17Timer_TowerOnTime12dispatch_sigEiij
// DEMANGLED: Timer_TowerOnTime::dispatch_sig(int,int,unsigned int)
int Timer_TowerOnTime::dispatch_sig(int tower_index,
                                    int timer_key,
                                    unsigned int timer_message) {
    // IDA 0x863B8CF copies arg1 to getBossTower's explicit integer argument;
    // it is the static-pool object index, not a singleton allocation lookup.
    WongWork::CBossTower* const tower =
        G_CGameManager()->getBossTower(tower_index);
    if (!tower) {
        return 0;
    }

    tower->_onTimerProcess(static_cast<TIMER_MESSAGE>(timer_message),
                           static_cast<unsigned int>(timer_key));
    return 1;
}

namespace WongWork {

// IDA_EA: 0x8142CDE
// IDA_NAME: _ZN8WongWork10CBossTower15_onTimerProcessE13TIMER_MESSAGEj
// DEMANGLED: WongWork::CBossTower::_onTimerProcess(TIMER_MESSAGE,unsigned int)
int CBossTower::_onTimerProcess(TIMER_MESSAGE message, unsigned int) {
    switch (static_cast<int>(message)) {
        case kBossTowerClearTimerMessage:
            return onClearBossTower();
        case kBossTowerReturnTimerMessage:
            return onReturnVillage();
        case kBossTowerReadyTimerMessage:
            return onReadyTimerCheck();
        default:
            return static_cast<int>(message);
    }
}

// IDA_EA: 0x8144508
// IDA_NAME: _ZN8WongWork10CBossTower16onClearBossTowerEv
// DEMANGLED: WongWork::CBossTower::onClearBossTower(void)
int CBossTower::onClearBossTower() {
    if (party_->battle_field().instance_dungeon_mode() == 1) {
        PacketGuard packet;
        packet.put_header(0, 254);
        packet.finalize(true);
        party_->send_to_party(packet);
    } else {
        if (generateRewardExp() != 1 || generateRewardItem() != 1 ||
            SendRewardPacket() != 1 || InsertRewardExp() != 1 ||
            InsertRewardItems() != 1) {
            return 0;
        }
    }

    if (SendELPLPacket() != 1) {
        return 0;
    }
    party_->SetEPLPState(2);
    G_TimerQueue()->InsertTimer(7,
                                static_cast<int>(pool_index_),
                                kBossTowerReturnTimerMessage,
                                20,
                                kBossTowerReturnTimerMessage,
                                kBossTowerReturnTimerMessage);
    return 1;
}

// IDA_EA: 0x81447A0
// IDA_NAME: _ZN8WongWork10CBossTower17InsertRewardItemsEv
// DEMANGLED: WongWork::CBossTower::InsertRewardItems(void)
int CBossTower::InsertRewardItems() {
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (!user || party_->checkValidUser(member_slot) != 1) {
            continue;
        }

        stRewardItem_t& rewards = rewards_[member_slot];
        for (int reward_slot = 0;
             reward_slot < static_cast<int>(rewards.reward_value_);
             ++reward_slot) {
            Inven_Item& reward = rewards.items_[reward_slot];
            if (reward.item_id_ == -1) {
                continue;
            }

            int inserted_slot = -1;
            if (reward.item_id_ != 0) {
                inserted_slot = user->getCurCharacInvenW()->insertItemIntoInventory(
                    reward, static_cast<eItemAddReason>(11), true, true);
            } else {
                const int received_money =
                    user->getCurCharacInvenW()->gain_money(
                        reward.get_add_info(),
                        static_cast<eMoneyAddReason>(5),
                        true,
                        0);
                if (reward.get_add_info() > received_money) {
                    user->SendMoneyFullReason(
                        static_cast<ENUM_MONEY_FULL_REASON>(0),
                        reward.get_add_info(),
                        received_money);
                }
                inserted_slot = 0;
            }

            if (inserted_slot < 0) {
                SendUnclaimedBossTowerReward(user, reward);
                continue;
            }

            PacketGuard packet;
            packet.put_header(0, 14);
            const int item_space = reward.GetItemSpace();
            packet.put_byte(item_space);
            packet.put_short(1);
            user->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(GetInvenTypeFromItemSpace(
                    static_cast<ENUM_ITEMSPACE>(item_space))),
                inserted_slot,
                packet);
            packet.finalize(true);
            user->Send(packet);
        }
    }
    return 1;
}

// IDA_EA: 0x814512A
// IDA_NAME: _ZN8WongWork10CBossTower17onReadyTimerCheckEv
// DEMANGLED: WongWork::CBossTower::onReadyTimerCheck(void)
int CBossTower::onReadyTimerCheck() {
    if (checkAllMemberReady(party_->get_member_count())) {
        return 0;
    }
    if (!party_ || party_->get_member_count() <= 0) {
        _destroy();
        return 0;
    }

    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (user && party_->checkValidUser(member_slot) == 1 &&
            member_ready_[member_slot] != 1) {
            onChargeRequireItem(user);
            handleLeaveUser(user);
        }
    }

    if (party_->get_member_count() <= 0) {
        _destroy();
    } else {
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_FINISH_LOADING);
        packet.put_int(0);
        packet.put_byte(0);
        packet.finalize(true);
        party_->send_to_party(packet);
    }
    return 1;
}

// IDA_EA: 0x814530C
// IDA_NAME: _ZN8WongWork10CBossTower19onChargeRequireItemEP5CUser
// DEMANGLED: WongWork::CBossTower::onChargeRequireItem(CUser *)
int CBossTower::onChargeRequireItem(CUser* user) {
    if (!user) {
        return 0;
    }
    if (party_->battle_field().instance_dungeon_mode() == 1) {
        return 1;
    }
    if (!dungeon_->required_item_.delete_after_start) {
        return 1;
    }

    Inven_Item required_item;
    CItem* const item =
        G_CDataManager()->find_item(dungeon_->required_item_.item_id);
    if (!item) {
        return 0;
    }

    item->vtable_->make_item(item, required_item);
    required_item.set_add_info(dungeon_->required_item_.item_count);
    required_item.item_id_ = dungeon_->required_item_.item_id;
    const int inserted_slot = user->getCurCharacInvenW()->insertItemIntoInventory(
        required_item, static_cast<eItemAddReason>(4), true, true);
    if (inserted_slot >= 0) {
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 inserted_slot);
    }
    return 1;
}

}  // namespace WongWork
