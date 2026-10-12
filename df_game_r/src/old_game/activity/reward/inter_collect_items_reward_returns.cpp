#include "old_game/activity/reward/inter_collect_items_reward_returns.h"

#include <cstdio>
#include <cstring>

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

namespace {

struct RewardMailSpec {
    int item_id;
    const char* text_key;
};

const RewardMailSpec kRewardMailSpecs[] = {
    {8663, "game_server_msg_258"},
    {8663, "game_server_msg_257"},
    {26824, "game_server_msg_259"},
};

const char* FindStringOrEmpty(const char* key) {
    const char* const text = g_scriptStringManager_.findString(4, key, 0);
    return text ? text : "";
}

}  // namespace

// IDA_EA: 0x84E71EE
// IDA_NAME: _ZN24Inter_CollectItemsReward12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CollectItemsReward::dispatch_sig(CUser *,char *,int)
int Inter_CollectItemsReward::dispatch_sig(CUser* user, char* payload,
                                           int payload_size) {
    if (!user || !payload || payload_size < 19 || !user->getCurCharacR()) {
        return 0;
    }

    const unsigned char reward_type =
        static_cast<unsigned char>(payload[18]);
    if (reward_type >= sizeof(kRewardMailSpecs) / sizeof(kRewardMailSpecs[0])) {
        return 0;
    }
    const RewardMailSpec& spec = kRewardMailSpecs[reward_type];

    CItem* const item_data = G_CDataManager()->find_item(spec.item_id);
    if (!item_data || !item_data->vtable_ || !item_data->vtable_->make_item) {
        return 0;
    }

    Inven_Item item;
    item.item_id_ = spec.item_id;
    item_data->vtable_->make_item(item_data, item);

    char title[30] = {};
    char text[256] = {};
    std::strncpy(title, FindStringOrEmpty("game_server_msg_256"),
                 sizeof(title) - 1);
    std::snprintf(text, sizeof(text), FindStringOrEmpty(spec.text_key),
                  user->getCurCharacName());

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        true,
        false);
    return 0;
}

// IDA_EA: 0x84EEDC0 / 0x84F5C70 / 0x84F5CA0
Inter_CollectItemsReward::Inter_CollectItemsReward() : ISigDispatcher() {}
Inter_CollectItemsReward::~Inter_CollectItemsReward() {}
