#ifndef DNF_DB_UPDATE_CERASHOP_PURCHASE_COUNT_H_
#define DNF_DB_UPDATE_CERASHOP_PURCHASE_COUNT_H_

#include "old_data/db/auction_cashshop/cera_shop_purchase_count_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 07_npc_shop_dialog/DB_UpdateCeraShopPurchaseCount.
class DB_UpdateCeraShopPurchaseCount : public DBDispatcher {
public:
    DB_UpdateCeraShopPurchaseCount();
    virtual ~DB_UpdateCeraShopPurchaseCount();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int member_id,
                            unsigned long item_index,
                            int server_id,
                            int user_idx);

private:
    int _selectPurchaseCount(unsigned int member_id,
                             unsigned short& purchase_count);
    int _updatePurchaseCount(unsigned int member_id,
                             unsigned short purchase_count);
    int _insertPurchaseCount(unsigned int member_id);
};

#endif  // DNF_DB_UPDATE_CERASHOP_PURCHASE_COUNT_H_
