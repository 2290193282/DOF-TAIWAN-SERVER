#ifndef OLDSRV_DB_DB_SAVE_GIVE_AVENGER_TITLE_FLAG_H_
#define OLDSRV_DB_DB_SAVE_GIVE_AVENGER_TITLE_FLAG_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8442226/0x84422DC/0x844F8BA/0x845BA52/0x845BA82.
// IDA unit: 13_database_dispatch/DB_SaveGiveAvengerTitleFlag.
class DB_SaveGiveAvengerTitleFlag : public DBDispatcher {
public:
    DB_SaveGiveAvengerTitleFlag();
    virtual ~DB_SaveGiveAvengerTitleFlag();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            unsigned int charac_no);
};

#endif  // OLDSRV_DB_DB_SAVE_GIVE_AVENGER_TITLE_FLAG_H_
