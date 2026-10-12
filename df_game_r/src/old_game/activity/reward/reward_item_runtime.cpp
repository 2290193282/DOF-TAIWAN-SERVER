#include "old_game/activity/reward/reward_item_runtime.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x84BFB84
// IDA_NAME: _Z10RewardItemP5CUserllljPKcS2_j
// DEMANGLED: RewardItem(
//     CUser *,long,long,long,unsigned int,char const*,char const*,unsigned int)
int RewardItem(CUser* user,
               long start_time,
               long end_time,
               long expiration_time,
               unsigned int item_id,
               const char* title_string_key,
               const char* body_string_key,
               unsigned int occurrence_time) {
    const long current_time =
        static_cast<long>(GlobalData::s_systemTime_.getCurSec());
    long result = end_time;

    if (end_time <= current_time) {
        if (expiration_time == 0 || expiration_time >= current_time) {
            result = start_time;
            if (start_time <= end_time) {
                Inven_Item item;
                CItem* const prototype = G_CDataManager()->find_item(
                    static_cast<int>(item_id));
                if (prototype) {
                    // The old stack record receives the supplied id before
                    // the CItem virtual factory fills its remaining fields.
                    item.item_id_ = static_cast<int32_t>(item_id);
                    prototype->vtable_->make_item(prototype, item);
                    item.set_add_info(1);

                    // Deliberately retain the original bounded copies without
                    // pre-zeroing: the source then measures the body exactly
                    // as its original mail request did.
                    char title[21];
                    char body[256];
                    std::strncpy(title,
                                 g_scriptStringManager_.findString(
                                     4, title_string_key, 0),
                                 0x14u);
                    std::strncpy(body,
                                 g_scriptStringManager_.findString(
                                     4, body_string_key, 0),
                                 0xffu);

                    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                        title,
                        item,
                        0,
                        static_cast<unsigned int>(user->getCurCharacNo()),
                        body,
                        static_cast<int>(std::strlen(body)),
                        occurrence_time,
                        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
                        false,
                        false);
                }
                return 0;
            }
        } else {
            result = expiration_time;
        }
    }

    return static_cast<int>(result);
}
