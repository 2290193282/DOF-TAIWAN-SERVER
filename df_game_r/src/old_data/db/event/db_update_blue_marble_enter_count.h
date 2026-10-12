#ifndef OLDSRV_DB_DB_UPDATE_BLUE_MARBLE_ENTER_COUNT_H_
#define OLDSRV_DB_DB_UPDATE_BLUE_MARBLE_ENTER_COUNT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 11_event_runtime/DB_UpdateBlueMarbleEnterCount.
class DB_UpdateBlueMarbleEnterCount : public DBDispatcher {
public:
    DB_UpdateBlueMarbleEnterCount();
    virtual ~DB_UpdateBlueMarbleEnterCount();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int charac_no, unsigned char enter_count);
};

#endif  // OLDSRV_DB_DB_UPDATE_BLUE_MARBLE_ENTER_COUNT_H_
