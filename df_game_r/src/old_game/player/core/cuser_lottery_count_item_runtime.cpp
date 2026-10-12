#include "old_game/player/core/cuser.h"

#include <cstring>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x869154A
// IDA_NAME: _ZN5CUser20giveLotteryCountItemEib
// DEMANGLED: CUser::giveLotteryCountItem(int,bool)
void CUser::giveLotteryCountItem(int item_index, bool match) {
    if (!getCurCharacR()) {
        LogManager::logFormat(
            1,
            "user.cpp",
            "void CUser::giveLotteryCountItem(int, bool)",
            0xB45F,
            "giveLotteryCountItem curchar = null id(%s) item_no(%u) match(%d)",
            NumberToString(static_cast<unsigned int>(get_acc_id()), 0),
            static_cast<unsigned int>(item_index),
            match);
        return;
    }

    CItem* const source = G_CDataManager()->find_item(item_index);
    if (!source) {
        return;
    }
    if (!verifyStackableItemProcess(static_cast<StackableAction::Type>(54),
                                    source)) {
        return;
    }

    const CStackableItem* const stackable =
        static_cast<const CStackableItem*>(source);
    const std::vector<int>& values = stackable->action_values();
    std::vector<Inven_Item> generated_items;

    for (size_t index = 0; index < values.size();) {
        // Retail reads the values as mandatory id/count pairs.  at() retains
        // the old malformed-PVF failure instead of silently dropping a half
        // configured reward row.
        Inven_Item reward;
        reward.item_id_ = values.at(index++);
        reward.set_add_info(values.at(index++));
        generated_items.push_back(reward);

        if (reward.item_id_ == 1 && reward.get_add_info() >= 0) {
            const unsigned int new_coin = static_cast<unsigned int>(
                getCurCharacInvenR()->GetEventCoin() + reward.get_add_info());
            getCurCharacInvenW()->SetEventCoin(new_coin);
            history_log().EventCoinAdd(
                getCurCharacInvenR()->GetEventCoin(),
                reward.get_add_info(),
                static_cast<eCoinAddReason>(2));
            SaveMoneyCoin();
            SendCoin();
            continue;
        }

        G_CDataManager()->get_item_list()->create_item(
            static_cast<ENUM_ITEM_CREATE_TYPE>(2), reward, reward.get_add_info());
        if (reward.item_id_ == 0 && reward.get_add_info() == 0) {
            continue;
        }

        const int inserted = getCurCharacInvenW()->insertItemIntoInventory(
            reward, static_cast<eItemAddReason>(13), true, true);
        if (inserted >= 0) {
            SendUpdateItem(static_cast<eSendTarget>(1),
                           static_cast<ENUM_ITEMSPACE>(0),
                           inserted);
            continue;
        }

        char content[256];
        const char* const script_content =
            g_scriptStringManager_.findString(4, "game_server_msg_104", 0);
        std::strncpy(content, script_content, 0xffu);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            getCurCharacName(),
            reward,
            0,
            static_cast<unsigned int>(getCurCharacNo()),
            content,
            static_cast<int>(std::strlen(content)),
            0,
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
            false,
            false);
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_REWARD_ITEMS);
    packet.put_byte(static_cast<int>(generated_items.size()));
    for (std::vector<Inven_Item>::const_iterator it = generated_items.begin();
         it != generated_items.end(); ++it) {
        packet.put_int(it->item_id_);
        packet.put_int(it->get_add_info());
    }
    packet.finalize(true);
    Send(packet);
}
