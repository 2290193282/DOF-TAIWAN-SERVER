#include "old_data/Login/packet_dispatch/compound_equipment_upgrade_card_dispatcher.h"

#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/event_upgrade_jar_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {
const char kDispatchSignature[] =
    "virtual int Dispatcher_Compound_Equipment_Upgrade_Card::dispatch_sig(CUser*, PacketBuf&)";
}

// IDA_EA: 0x8597068
// IDA_NAME: _ZN42Dispatcher_Compound_Equipment_Upgrade_CardC2Ev
// DEMANGLED: Dispatcher_Compound_Equipment_Upgrade_Card::Dispatcher_Compound_Equipment_Upgrade_Card(void)
Dispatcher_Compound_Equipment_Upgrade_Card::Dispatcher_Compound_Equipment_Upgrade_Card()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240138
// IDA_NAME: _ZN42Dispatcher_Compound_Equipment_Upgrade_CardD2Ev
// DEMANGLED: Dispatcher_Compound_Equipment_Upgrade_Card::~Dispatcher_Compound_Equipment_Upgrade_Card()
// IDA_EA: 0x8240168
// IDA_NAME: _ZN42Dispatcher_Compound_Equipment_Upgrade_CardD0Ev
// DEMANGLED: Dispatcher_Compound_Equipment_Upgrade_Card::~Dispatcher_Compound_Equipment_Upgrade_Card()
Dispatcher_Compound_Equipment_Upgrade_Card::~Dispatcher_Compound_Equipment_Upgrade_Card() {}

// IDA_EA: 0x8228A2E
// IDA_NAME: _ZNK42Dispatcher_Compound_Equipment_Upgrade_Card20set_ReturnSendPacketER11PacketGuardti
// DEMANGLED: Dispatcher_Compound_Equipment_Upgrade_Card::set_ReturnSendPacket(PacketGuard &,unsigned short,int)const
int Dispatcher_Compound_Equipment_Upgrade_Card::set_ReturnSendPacket(
    PacketGuard& packet, unsigned short slot, int remaining_count) const {
    packet.put_byte(0);
    packet.put_short(slot);
    return packet.put_int(remaining_count);
}

// IDA_EA: 0x8227E00
// IDA_NAME: _ZN42Dispatcher_Compound_Equipment_Upgrade_Card12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Compound_Equipment_Upgrade_Card::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Compound_Equipment_Upgrade_Card::dispatch_sig(CUser* user,
                                                              PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) return 0;
    if (G_GameWorld()->GetChannelType() == 7)
        return LineFunc(58563, kDispatchSignature, 0, 0);

    unsigned short first_slot;
    unsigned short second_slot;
    unsigned short recipe_slot;
    packet.get_short(first_slot);
    packet.get_short(second_slot);
    packet.get_short(recipe_slot);

    const Inven_Item first = user->getCurCharacInvenR()->GetInvenSlot(1, first_slot);
    const Inven_Item second = user->getCurCharacInvenR()->GetInvenSlot(1, second_slot);
    unsigned long first_card = static_cast<unsigned long>(static_cast<unsigned int>(first.item_id_));
    unsigned long second_card = static_cast<unsigned long>(static_cast<unsigned int>(second.item_id_));
    const int booster_id = GlobalData::s_pEvent_UpGrade_Jar->
        getBoosterIndex_byUpgradeCardIndex(first_card, second_card);

    if (!booster_id || first.add_info_ <= 0 || second.add_info_ <= 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_COMPOUND_EQUIPMENT_UPGRADE_CARD, 19);
        return 0;
    }

    const Inven_Item recipe = user->getCurCharacInvenR()->GetInvenSlot(1, recipe_slot);
    if (!GlobalData::s_pEvent_UpGrade_Jar->check_requiredRecipe(recipe)) return 0;
    CItem* const booster = G_CDataManager()->find_item(booster_id);
    if (!booster || booster->vtable_->get_item_type(booster) != 15) return 0;

    std::vector<stStackableBoosterItemInfo_t> rewards;
    static_cast<const CStackableItem*>(booster)->getBoosterItem(rewards);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_COMPOUND_EQUIPMENT_UPGRADE_CARD);
    std::vector<int> inserted_slots;
    if (rewards.empty() || rewards.size() > 1u) {
        response.put_byte(0);
        response.put_byte(19);
    } else {
        user->getCurCharacInvenW()->delete_item(static_cast<INVEN_TYPE>(1), recipe_slot, 1,
                                                static_cast<eItemDelReason>(41), true);
        user->getCurCharacInvenW()->delete_item(static_cast<INVEN_TYPE>(1), first_slot, 1,
                                                static_cast<eItemDelReason>(41), true);
        user->getCurCharacInvenW()->delete_item(static_cast<INVEN_TYPE>(1), second_slot, 1,
                                                static_cast<eItemDelReason>(41), true);
        const Inven_Item deleted_recipe = user->getCurCharacInvenR()->GetInvenSlot(1, recipe_slot);
        const Inven_Item deleted_first = user->getCurCharacInvenR()->GetInvenSlot(1, first_slot);
        const Inven_Item deleted_second = user->getCurCharacInvenR()->GetInvenSlot(1, second_slot);
        response.put_byte(1);
        // The original reads Inven_Item +0x07 after each deletion: this is
        // the post-transaction remaining stack count, not the item index.
        set_ReturnSendPacket(response, recipe_slot, deleted_recipe.add_info_);
        set_ReturnSendPacket(response, first_slot, deleted_first.add_info_);
        set_ReturnSendPacket(response, second_slot, deleted_second.add_info_);
        for (std::vector<stStackableBoosterItemInfo_t>::const_iterator it = rewards.begin(); it != rewards.end(); ++it) {
            CItem* const reward_prototype = G_CDataManager()->find_item(it->item_index);
            if (!reward_prototype) continue;
            Inven_Item reward;
            reward.item_id_ = it->item_index;
            reward_prototype->vtable_->make_item(reward_prototype, reward);
            reward.set_add_info(it->item_count);
            const int inserted = user->getCurCharacInvenW()->insertItemIntoInventory(
                reward, static_cast<eItemAddReason>(43), true, true);
            if (inserted < 0) {
                WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                    0, reward, 0, static_cast<unsigned int>(user->getCurCharacNo()),
                    0, 0, 0, static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()), false, false);
            } else {
                inserted_slots.push_back(inserted);
            }
            response.put_item_idx(static_cast<unsigned long>(reward.item_id_));
        }
    }
    response.finalize(true);
    user->Send(response);
    for (std::vector<int>::const_iterator it = inserted_slots.begin(); it != inserted_slots.end(); ++it)
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1), static_cast<ENUM_ITEMSPACE>(0), *it);
    return 0;
}

// IDA_EA: 0x82342EE
// IDA_NAME: _ZN42Dispatcher_Compound_Equipment_Upgrade_Card4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Compound_Equipment_Upgrade_Card::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Compound_Equipment_Upgrade_Card::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}

// IDA_EA: 0x82342F8
// IDA_NAME: _ZN42Dispatcher_Compound_Equipment_Upgrade_Card7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Compound_Equipment_Upgrade_Card::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Compound_Equipment_Upgrade_Card::process(CUser* user, MSG_BASE& message, ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}
