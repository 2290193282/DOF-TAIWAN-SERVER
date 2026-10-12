#ifndef DNF_DB_UPDATE_CHUSEOK_PACKAGE_PURCHASE_H_
#define DNF_DB_UPDATE_CHUSEOK_PACKAGE_PURCHASE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateChuseokPackagePurchase.
class DB_UpdateChuseokPackagePurchase : public DBDispatcher {
public:
    DB_UpdateChuseokPackagePurchase();
    virtual ~DB_UpdateChuseokPackagePurchase();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int member_id);
};

#endif  // DNF_DB_UPDATE_CHUSEOK_PACKAGE_PURCHASE_H_
