#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/etc/delete_invalid_item_data.h"
#include "old_game/economy/mail/mail_trade_runtime.h"

#include <cstddef>
#include <cstring>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

namespace {

const char kDefaultDeleteInvalidItemMailText[] = "Nexon CS";

struct RewardItemPrototypeVTable {
    void (*destructor)(CItem* item);
    void (*deleting_destructor)(CItem* item);
    void (*make_item)(const CItem* item, Inven_Item& inventory_item);
};

#if defined(__i386__)
static_assert(offsetof(RewardItemPrototypeVTable, make_item) == 0x08u,
              "CItem make-item vtable offset must stay 0x08");
#endif

const RewardItemPrototypeVTable* RewardItemVTable(const CItem* item) {
    const RewardItemPrototypeVTable* table = 0;
    std::memcpy(&table, item, sizeof(table));
    return table;
}

const char* ResolveMailText(const std::string& key) {
    if (key.empty()) {
        return kDefaultDeleteInvalidItemMailText;
    }
    return g_scriptStringManager_.findString(4, toTString(key), 0);
}

void SendRewardMailBatch(CUser* user,
                         const char* title,
                         const char* content,
                         const Inven_Item* items,
                         unsigned int item_count) {
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(
        title,
        items,
        item_count,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        content,
        static_cast<int>(std::strlen(content)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false);
}

}  // namespace

// IDA_EA: 0x8686F1E
// IDA_NAME: _ZN15cUserHistoryLog17DeleteInvaildItemERKSsii
// DEMANGLED: cUserHistoryLog::DeleteInvaildItem(std::string const&,int,int)
int cUserHistoryLog::DeleteInvaildItem(const std::string& source,
                                       int item_id,
                                       int add_info) {
    return owner_->LogHistory("DeleteInvaildItem, %s, %d, %d",
                              source.c_str(),
                              item_id,
                              add_info);
}

// IDA_EA: 0x86931C4
// IDA_NAME: _ZN5CUser28RewardItem2DeleteInvalidItemERKSsRKSt6vectorISt4pairIiiESaIS4_EE
// DEMANGLED: CUser::RewardItem2DeleteInvalidItem(std::string const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
void CUser::RewardItem2DeleteInvalidItem(
    const std::string& source,
    const std::vector<std::pair<int, int> >& invalid_items) {
    if (invalid_items.empty() || !getCurCharacR()) {
        return;
    }

    std::vector<Inven_Item> rewards;
    for (std::vector<std::pair<int, int> >::const_iterator it =
             invalid_items.begin();
         it != invalid_items.end();
         ++it) {
        const int invalid_item_id = it->first;
        CUser_history_log(this)->DeleteInvaildItem(
            source, invalid_item_id, it->second);

        if (!G_CDataManager()->find_item(invalid_item_id)) {
            continue;
        }

        int reward_item_id = 0;
        int reward_count = 0;
        if (static_cast<unsigned char>(
                G_CDataManager()->get_delete_invalid_item_script()
                    ->GetRewardItemInfo(
                    invalid_item_id, reward_item_id, reward_count)) != 1u ||
            reward_count <= 0) {
            continue;
        }

        const CItem* const prototype =
            G_CDataManager()->find_item(reward_item_id);
        if (!prototype) {
            continue;
        }

        Inven_Item reward;
        reward.item_id_ = reward_item_id;
        RewardItemVTable(prototype)->make_item(prototype, reward);
        if (static_cast<unsigned char>(prototype->is_stackable()) != 0u) {
            reward.set_add_info(reward_count);
        }
        rewards.push_back(reward);
    }

    if (rewards.empty()) {
        return;
    }

    std::string title_key;
    std::string content_key;
    G_CDataManager()->get_delete_invalid_item_script()->GetMailInfo(
        title_key, content_key);

    char title[30] = {};
    char content[256] = {};
    std::strncpy(title, ResolveMailText(title_key), 29u);
    std::strncpy(content, ResolveMailText(content_key), 255u);

    Inven_Item batch[10];
    unsigned int batch_count = 0;
    for (std::vector<Inven_Item>::const_iterator it = rewards.begin();
         it != rewards.end();
         ++it) {
        batch[batch_count++].setCopy(*it);
        if (batch_count == 10u) {
            SendRewardMailBatch(this,
                                title,
                                content,
                                batch,
                                batch_count);
            batch_count = 0;
        }
    }
    if (batch_count) {
        SendRewardMailBatch(this,
                            title,
                            content,
                            batch,
                            batch_count);
    }
}
