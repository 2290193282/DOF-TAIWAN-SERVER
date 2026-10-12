#ifndef OLDSRV_DB_DB_UPDATE_QUEST_SHOP_BUYING_STATUS_LIST_H_
#define OLDSRV_DB_DB_UPDATE_QUEST_SHOP_BUYING_STATUS_LIST_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x840568A/0x844F7BE/0x845FB86/0x845FBB6.
// IDA unit: 07_npc_shop_dialog/DB_UpdateQuestShopBuyingStatusList.
class DB_UpdateQuestShopBuyingStatusList : public DBDispatcher {
public:
    DB_UpdateQuestShopBuyingStatusList();
    virtual ~DB_UpdateQuestShopBuyingStatusList();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_QUEST_SHOP_BUYING_STATUS_LIST_H_
