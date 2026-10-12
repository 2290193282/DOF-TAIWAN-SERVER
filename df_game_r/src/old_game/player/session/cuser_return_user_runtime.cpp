#include "old_game/player/core/cuser.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/level_reward_scripts.h"
#include "old_data/db/account/db_update_return_user_first_login.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8691EC4
// IDA_NAME: _ZN5CUser17processReturnUserEv
// DEMANGLED: CUser::processReturnUser(void)
void CUser::processReturnUser() {
    if (!IsReturnUser()) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 301);
    const int expire_time = GetReturnUserExpireTime();
    const int current_time = GlobalData::s_systemTime_.getCurSec();
    packet.put_int(expire_time - current_time);
    packet.finalize(true);
    Send(packet);

    if (IsReturnUserFirstLogin()) {
        GiveItemsReturnUser();
        GiveEquipItemsReturnUser();

        // The DB dispatcher decodes this request as m_id, first_login,
        // expire_time.  Capture first_login again after the two reward
        // transactions, then clear the local flag only after queueing it.
        const int first_login = IsReturnUserFirstLogin();
        const int persisted_expire_time = GetReturnUserExpireTime();
        DB_UpdateReturnUserFirstLogin::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            first_login,
            persisted_expire_time);
        SetReturnUserFirstUserLogin(false);
    }
}

// IDA_EA: 0x8692000
// IDA_NAME: _ZN5CUser19GiveItemsReturnUserEv
// DEMANGLED: CUser::GiveItemsReturnUser(void)
int CUser::GiveItemsReturnUser() {
    const int item_id = static_cast<int>(
        G_CDataManager()->get_return_user_reward_script()->first_dword);
    Inven_Item item;
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (!prototype) {
        return 0;
    }

    item.item_id_ = item_id;
    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(1);

    const char* const text =
        g_scriptStringManager_.findString(4, "return_user_msg_1", 0);
    const int server_group = GetServerGroup();
    const int text_length = static_cast<int>(std::strlen(text));
    const unsigned int charac_no = static_cast<unsigned int>(getCurCharacNo());
    const char* const title =
        g_scriptStringManager_.findString(4, "return_user_msg_title", 0);
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        100000,
        charac_no,
        text,
        text_length,
        7,
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false,
        false);
}

// IDA_EA: 0x8692140
// IDA_NAME: _ZN5CUser24GiveEquipItemsReturnUserEv
// DEMANGLED: CUser::GiveEquipItemsReturnUser(void)
int CUser::GiveEquipItemsReturnUser() {
    const int first_grow_type_for_key = getCurCharFirstGrowType();
    const int charac_level = get_charac_level();
    const int return_user_level_key = getReturnUserLevelKey(
        charac_level, first_grow_type_for_key);
    const int first_grow_type = getCurCharFirstGrowType();
    const int charac_job = get_charac_job();
    LevelUpRewardItemTableKey table_key(
        return_user_level_key, charac_job, first_grow_type);
    LevelUpRewardItemData reward;

    int result = G_CDataManager()->get_return_user_reward_script()
                     ->level_up_rewards.get_data(table_key, reward) ^
                 1;
    if (result) {
        return result;
    }

    Inven_Item item;
    CItem* const prototype = G_CDataManager()->find_item(reward.item_id);
    if (!prototype) {
        return 0;
    }

    item.item_id_ = reward.item_id;
    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(reward.count);

    const char* const text =
        g_scriptStringManager_.findString(4, "return_user_msg_2", 0);
    const int server_group = GetServerGroup();
    const int text_length = static_cast<int>(std::strlen(text));
    const unsigned int charac_no = static_cast<unsigned int>(getCurCharacNo());
    const char* const title =
        g_scriptStringManager_.findString(4, "return_user_msg_title", 0);
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        charac_no,
        text,
        text_length,
        7,
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false,
        false);
}

// IDA_EA: 0x869230A
// IDA_NAME: _ZN5CUser21getReturnUserLevelKeyEii
// DEMANGLED: CUser::getReturnUserLevelKey(int,int)
int CUser::getReturnUserLevelKey(int charac_level, int first_grow_type) {
    if (first_grow_type) {
        if (charac_level <= 19) {
            return 15;
        }
    } else {
        if (charac_level <= 9) {
            return 5;
        }
        if (charac_level <= 14) {
            return 10;
        }
        if (charac_level <= 19) {
            return 15;
        }
    }

    if (charac_level <= 29) {
        return 20;
    }
    if (charac_level <= 39) {
        return 30;
    }
    if (charac_level <= 49) {
        return 40;
    }
    if (charac_level <= 59) {
        return 50;
    }
    if (charac_level > 69) {
        return 70;
    }
    return 60;
}
