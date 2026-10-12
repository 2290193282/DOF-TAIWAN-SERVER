#include "old_game/social/party/party.h"

#include <cstring>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

namespace {
int InventoryItemId(const Inven_Item& item) {
    return item.item_id_;
}

}  // namespace

// IDA_EA: 0x859BC9A
// IDA_NAME: _ZN6CParty31increase_ancient_dungeon_ticketEv
// DEMANGLED: CParty::increase_ancient_dungeon_ticket(void)
char CParty::increase_ancient_dungeon_ticket() {
    CDungeon* const dungeon = battle_field_.dungeon_;
    if (!dungeon || dungeon->ancient_dungeon_ != 1u) {
        return dungeon ? 0 : 0;
    }

    const int item_id = dungeon->required_item_.item_id;
    const int item_count = dungeon->required_item_.item_count;
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager || !data_manager->find_item(item_id)) {
        return 0;
    }

    Inven_Item item;
    data_manager->get_item_list()->create_item(
        static_cast<ENUM_ITEM_CREATE_TYPE>(0), item, item_count);
    if (InventoryItemId(item) == -1 && item.get_add_info() == 0) {
        return 0;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }

        CUser* const user = members_[seat].user;
        const int inserted_slot = user->getCurCharacInvenW()->insertItemIntoInventory(
            item, static_cast<eItemAddReason>(56), true, true);
        if (inserted_slot >= 0) {
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 inserted_slot);
            continue;
        }

        char title[21];
        char text[256];
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_53", 0),
                     20u);
        std::strncpy(text,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_154", 0),
                     255u);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            item,
            0,
            static_cast<unsigned int>(user->getCurCharacNo()),
            text,
            static_cast<int>(std::strlen(text)),
            0,
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
            false,
            false);
        item.reset();
    }
    return 1;
}
