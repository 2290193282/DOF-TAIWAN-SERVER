#ifndef OLDSRV_DB_DB_SAVE_PREMIUM_INFO_H_
#define OLDSRV_DB_DB_SAVE_PREMIUM_INFO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SavePremiumInfo.
// IDA_EA: dispatch @0x841B4BE; ctor/dtor @0x844E136/@0x845F916/@0x845F946.
// DBTask old slot +85 -> active dispatcher index 84.
class DB_SavePremiumInfo : public DBDispatcher {
public:
    DB_SavePremiumInfo();
    virtual ~DB_SavePremiumInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_PREMIUM_INFO_H_
