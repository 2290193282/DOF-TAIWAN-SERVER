#include "old_game/player/core/cuser.h"

#include <cstring>
#include <stdint.h>
#include <utility>
#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/db_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8656CAA
// IDA_NAME: _ZN5CUser12AddDailyItemEv
// DEMANGLED: CUser::AddDailyItem(void)
void CUser::AddDailyItem() {
    // The old daily reset first restores the three-count stack for users who
    // have passed the level gate.  The same item transaction is also used by
    // the level-up-to-20 path, but this is a separate daily invocation.
    if (get_charac_level() > 19) {
        CItem* const prototype = G_CDataManager()->find_item(4183);
        if (prototype) {
            Inven_Item item;
            int slot = getCurCharacInvenR()->GetInvenData(4183, item);
            if (slot >= 0) {
                if (item.get_add_info() <= 2) {
                    const int old_count = item.get_add_info();
                    history_log().ItemAdd(
                        static_cast<INVEN_TYPE>(1),
                        3 - old_count,
                        old_count,
                        item,
                        static_cast<eItemAddReason>(13));
                    item.set_add_info(3);
                    getCurCharacInvenW()->update_item(
                        static_cast<INVEN_TYPE>(1), slot, item);
                }
            } else {
                prototype->vtable_->make_item(prototype, item);
                item.set_add_info(3);
                item.item_id_ = 4183;
                slot = getCurCharacInvenW()->insertItemIntoInventory(
                    item, static_cast<eItemAddReason>(13), true, false);
            }

            if (slot >= 0) {
                SendUpdateItemList(static_cast<eSendTarget>(1),
                                   static_cast<ENUM_ITEMSPACE>(0),
                                   slot);
            }
        }
    }

    std::vector<std::pair<int, int> > reset_items;
    std::vector<std::pair<int, int> > changed_slots;
    ResetItemByScript(reset_items);
    RefillItemByScript(reset_items);
    deleteSpecificItem(reset_items, changed_slots);

    for (std::vector<std::pair<int, int> >::const_iterator it =
             changed_slots.begin();
         it != changed_slots.end(); ++it) {
        if (it->first >= 0 && it->first <= 27 && it->second >= 0) {
            SendUpdateItemList(static_cast<eSendTarget>(1),
                               static_cast<ENUM_ITEMSPACE>(it->first),
                               it->second);
        }
    }

    advance_altar_manager_.refillTicketFree();

    // This legacy one-time item branch is intentionally date-gated exactly
    // as in the old server.  It either writes the inventory delta or sends
    // the same item through the DB system-mail transaction when inventory
    // insertion returns -1.
    if (GlobalData::s_systemTime_.getCurSec() <= 1209135599) {
        CItem* const prototype = G_CDataManager()->find_item(7339);
        if (prototype) {
            Inven_Item item;
            prototype->vtable_->make_item(prototype, item);
            item.set_add_info(1);
            item.item_id_ = 7339;

            const int slot = getCurCharacInvenW()->insertItemIntoInventory(
                item, static_cast<eItemAddReason>(13), true, true);
            if (slot == -1) {
                char title[21] = {};
                char contents[256] = {};
                std::strncpy(title,
                             g_scriptStringManager_.findString(
                                 4, "game_server_msg_24", 0),
                             0x14u);
                std::strncpy(contents,
                             g_scriptStringManager_.findString(
                                 4, "game_server_msg_25", 0),
                             0xffu);
                WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                    title,
                    item,
                    0,
                    static_cast<unsigned int>(getCurCharacNo()),
                    contents,
                    static_cast<int>(std::strlen(contents)),
                    0,
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                    false,
                    false);
            } else {
                PacketGuard packet;
                packet.put_header(0, 14);
                packet.put_byte(0);
                packet.put_short(1);
                if (getCurCharacInvenR()->MakeItemPacket(
                        static_cast<INVEN_TYPE>(1), slot, packet) == 1) {
                    packet.finalize(true);
                    Send(packet);
                }
            }
        }
    }
}

// IDA_EA: 0x86792D6
// IDA_NAME: _ZN5CUser19processAwakenRewardEc
// DEMANGLED: CUser::processAwakenReward(char)
int CUser::processAwakenReward(char reward_type) {
    const char* contents_key = 0;
    const char* title_key = 0;
    switch (reward_type) {
    case 0:
        contents_key = "awaken_mail_msg_1";
        title_key = "awaken_mail_title_1";
        break;
    case 1:
    case 7:
        contents_key = "awaken_mail_msg_2";
        title_key = "awaken_mail_title_2";
        break;
    case 2:
    case 5:
        contents_key = "awaken_mail_msg_3";
        title_key = "awaken_mail_title_3";
        break;
    case 3:
    case 8:
        contents_key = "awaken_mail_msg_4";
        title_key = "awaken_mail_title_4";
        break;
    case 4:
        contents_key = "awaken_mail_msg_5";
        title_key = "awaken_mail_title_5";
        break;
    case 6:
        contents_key = "awaken_mail_msg_6";
        title_key = "awaken_mail_title_6";
        break;
    default:
        return static_cast<int>(reward_type);
    }

    char contents[256] = {};
    char title[21] = {};
    std::strncpy(contents,
                 g_scriptStringManager_.findString(4, contents_key, 0),
                 0xffu);
    std::strncpy(title,
                 g_scriptStringManager_.findString(4, title_key, 0),
                 0x14u);

    const int reward_item_id = 960;
    CItem* const prototype = G_CDataManager()->find_item(reward_item_id);
    if (!prototype) {
        return static_cast<int>(reinterpret_cast<uintptr_t>(prototype));
    }

    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(1);
    item.item_id_ = reward_item_id;
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(getCurCharacNo()),
        contents,
        static_cast<int>(std::strlen(contents)),
        12,
        static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
        false,
        false);
}
