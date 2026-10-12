#include "old_game/activity/schedule/blue_marble_runtime.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

#include <cstring>

// IDA_EA: 0x80D7DDE
// IDA_NAME: _ZN10BlueMarble12resultRewardEv
// DEMANGLED: BlueMarble::resultReward(void)
bool BlueMarble::resultReward() {
    Inven_Item item;
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        if (!users_[slot].getUser()) {
            continue;
        }
        item.reset();
        setResultItem(slot, item);
        if (item.item_id_ != 0) {
            insertItem(slot, item, static_cast<eItemAddReason>(66));
        }
    }
    return true;
}

// IDA_EA: 0x80D7E8E
// IDA_NAME: _ZN10BlueMarble13setResultItemEiR10Inven_Item
// DEMANGLED: BlueMarble::setResultItem(int,Inven_Item &)
void BlueMarble::setResultItem(int user_slot, Inven_Item& item) {
    const int grade = static_cast<int>(users_[user_slot].getGrade());
    const std::vector<BlueMarbleRewardInfo>& rewards =
        script_->info_script.reward_items;
    if (grade < 0 || grade > 3 ||
        static_cast<std::size_t>(grade) >= rewards.size()) {
        return;
    }

    const BlueMarbleRewardInfo& reward = rewards[grade];
    item.item_id_ = reward.first_dword;
    item.set_add_info(reward.second_dword);
}

// IDA_EA: 0x80D800E
// IDA_NAME: _ZN10BlueMarble10insertItemEiR10Inven_Item14eItemAddReason
// DEMANGLED: BlueMarble::insertItem(int,Inven_Item &,eItemAddReason)
void BlueMarble::insertItem(int user_slot,
                            Inven_Item& item,
                            eItemAddReason reason) {
    CUser* const user = users_[user_slot].getUser();
    CDataManager* const data_manager = G_CDataManager();
    CItemList* const item_list = data_manager ? data_manager->get_item_list()
                                              : 0;
    if (!user || !item_list) {
        return;
    }

    item_list->create_item(ENUM_ITEM_CREATE_TYPE_DEFAULT,
                           item,
                           item.get_add_info());
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        return;
    }

    const int inventory_slot =
        inventory->insertItemIntoInventory(item, reason, true, false);
    if (inventory_slot >= 0) {
        sendUpdateItemInfo(user_slot, inventory_slot);
        return;
    }

    char title[30] = {};
    const char* title_text = g_scriptStringManager_.findString(
        4, "game_server_msg_268", 0);
    if (title_text) {
        std::strncpy(title, title_text, sizeof(title) - 1);
    }

    char body[256] = {};
    const char* body_text = g_scriptStringManager_.findString(
        4, "game_server_msg_269", 0);
    if (body_text) {
        std::strncpy(body, body_text, sizeof(body) - 1);
    }

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(user->get_charac_no(-1)),
        body,
        static_cast<int>(std::strlen(body)),
        15,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}
