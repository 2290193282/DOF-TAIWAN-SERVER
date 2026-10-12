#ifndef OLDSRV_DB_DB_UPDATE_USER_GIFT_FATIGUE_QUANTITY_H_
#define OLDSRV_DB_DB_UPDATE_USER_GIFT_FATIGUE_QUANTITY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8441D1C/0x8441FDE/0x844F882/0x845BAEE/0x845BB1E.
// IDA unit: 13_database_dispatch/DB_UpdateUserGiftFatigueQuantity.
class DB_UpdateUserGiftFatigueQuantity : public DBDispatcher {
public:
    DB_UpdateUserGiftFatigueQuantity();
    virtual ~DB_UpdateUserGiftFatigueQuantity();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int charac_no,
                            unsigned int fatigue_quantity);
};

#endif  // OLDSRV_DB_DB_UPDATE_USER_GIFT_FATIGUE_QUANTITY_H_
