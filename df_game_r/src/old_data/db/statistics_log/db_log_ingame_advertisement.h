#ifndef OLDSRV_DB_DB_LOG_INGAME_ADVERTISEMENT_H_
#define OLDSRV_DB_DB_LOG_INGAME_ADVERTISEMENT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 02_login_account/DB_LogIngameAdvertisement.
class DB_LogIngameAdvertisement : public DBDispatcher {
public:
    DB_LogIngameAdvertisement();
    virtual ~DB_LogIngameAdvertisement();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_LOG_INGAME_ADVERTISEMENT_H_
