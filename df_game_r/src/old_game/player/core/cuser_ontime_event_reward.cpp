#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

#include <cstdio>
#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8681C30
// IDA_NAME: _ZN5CUser23InsertOnTimeEventRewardEjj
// DEMANGLED: CUser::InsertOnTimeEventReward(unsigned int,unsigned int)
int CUser::InsertOnTimeEventReward(unsigned int item_id,
                                   unsigned int item_count) {
    char log_prefix[256] = {};
    std::sprintf(log_prefix,
                 "./log/%s/OnTimeEvent",
                 G_CEnvironment()->get_file_name());
    const int charac_no = get_charac_no(-1);
    const char* const account_id =
        NumberToString(static_cast<unsigned int>(get_acc_id()), 0);
    CMyFileLog log(
        "bool CUser::InsertOnTimeEventReward(unsigned int, unsigned int)",
        34463);
    log(log_prefix,
        "On Time Event:InsertOnTimeEventReward(%s, %u)",
        account_id,
        charac_no);

    if (item_id == 0) {
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    CItemList* const item_list = data_manager ? data_manager->get_item_list()
                                              : 0;
    if (!data_manager || !item_list ||
        !data_manager->find_item(static_cast<int>(item_id))) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD);
    packet.put_byte(1);
    packet.put_byte(1);

    Inven_Item item;
    item.item_id_ = static_cast<int>(item_id);
    item_list->create_item(ENUM_ITEM_CREATE_TYPE_DEFAULT,
                           item,
                           static_cast<int>(item_count));

    if (item.item_id_ != 0 ||
        item.get_add_info() != 0) {
        CInventory* const inventory = getCurCharacInvenW();
        const int inserted =
            inventory
                ? inventory->insertItemIntoInventory(
                      item,
                      static_cast<eItemAddReason>(13),
                      true,
                      true)
                : -1;
        if (inserted >= 0) {
            SendUpdateItem(static_cast<eSendTarget>(1),
                           static_cast<ENUM_ITEMSPACE>(0),
                           inserted);
        } else {
            char title[21] = {};
            const char* const title_text =
                g_scriptStringManager_.findString(
                    4, "game_server_msg_132", 0);
            if (title_text) {
                std::strncpy(title, title_text, sizeof(title) - 1);
            }

            char body[256] = {};
            const char* const body_text =
                g_scriptStringManager_.findString(
                    4, "game_server_msg_133", 0);
            if (body_text) {
                std::strncpy(body, body_text, sizeof(body) - 1);
            }

            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title,
                item,
                0,
                static_cast<unsigned int>(get_charac_no(-1)),
                body,
                static_cast<int>(std::strlen(body)),
                0,
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                false,
                false);
            item.reset();
        }

        packet.put_int(static_cast<int>(item_id));
        packet.put_short(static_cast<int>(item_count));
    }

    packet.finalize(true);
    Send(packet);
    return 1;
}
