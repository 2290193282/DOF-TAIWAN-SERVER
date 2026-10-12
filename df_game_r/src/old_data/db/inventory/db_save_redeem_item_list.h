#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_REDEEM_ITEM_LIST_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_REDEEM_ITEM_LIST_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SaveRedeemItemList.
// DBTask old slot +501 maps to dispatcher index 500.
class DB_SaveRedeemItemList : public DBDispatcher {
public:
    DB_SaveRedeemItemList();
    virtual ~DB_SaveRedeemItemList();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(DB_SaveRedeemItemList) == 0x04u,
              "DB_SaveRedeemItemList IDA ABI size must stay 0x04");
#endif

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_REDEEM_ITEM_LIST_H_
