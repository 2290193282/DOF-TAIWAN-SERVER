#ifndef OLDSRV_DB_DB_UPDATE_PURCHASE_CASH_ITEM_BONUS_H_
#define OLDSRV_DB_DB_UPDATE_PURCHASE_CASH_ITEM_BONUS_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/auction_cashshop/purchase_cash_item_bonus_payload.h"


// IDA unit: 10_mail_trade_auction_cash/DB_UpdatePurchaseCashItemBonus.
// IDA_EA: dispatch @0x843E252; makeRequest @0x843E560; ctor/dtor
// @0x844F61A/@0x845C0B8/@0x845C0E8.
class DB_UpdatePurchaseCashItemBonus : public DBDispatcher {
public:
    DB_UpdatePurchaseCashItemBonus();
    virtual ~DB_UpdatePurchaseCashItemBonus();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int uid, unsigned int account_id);
};

#endif  // OLDSRV_DB_DB_UPDATE_PURCHASE_CASH_ITEM_BONUS_H_
