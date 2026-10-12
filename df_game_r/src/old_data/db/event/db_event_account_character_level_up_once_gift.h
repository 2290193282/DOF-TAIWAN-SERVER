#ifndef OLDSRV_DB_DB_EVENT_ACCOUNT_CHARACTER_LEVEL_UP_ONCE_GIFT_H_
#define OLDSRV_DB_DB_EVENT_ACCOUNT_CHARACTER_LEVEL_UP_ONCE_GIFT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 02_login_account/DB_EventAccountCharacterLevelUpOnceGift.
// IDA_EA: dispatch @0x8449E3E; makeRequest @0x8449F66; ctor/dtor
// @0x844FE32/@0x845AC9C/@0x845ACCC. makeRequest emits signal 0x319.
class DB_EventAccountCharacterLevelUpOnceGift : public DBDispatcher {
public:
    DB_EventAccountCharacterLevelUpOnceGift();
    virtual ~DB_EventAccountCharacterLevelUpOnceGift();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            short server_id,
                            int charac_no,
                            short charac_job,
                            short charac_level);
};

#endif  // OLDSRV_DB_DB_EVENT_ACCOUNT_CHARACTER_LEVEL_UP_ONCE_GIFT_H_
