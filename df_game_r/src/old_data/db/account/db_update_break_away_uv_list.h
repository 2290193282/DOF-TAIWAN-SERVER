#ifndef OLDSRV_DB_DB_UPDATE_BREAK_AWAY_UV_LIST_H_
#define OLDSRV_DB_DB_UPDATE_BREAK_AWAY_UV_LIST_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843873A/0x8438842/0x844ED76/0x845CE20/0x845CE50.
// IDA unit: 13_database_dispatch/DB_UpdateBreakAwayUvList.
class DB_UpdateBreakAwayUvList : public DBDispatcher {
public:
    DB_UpdateBreakAwayUvList();
    virtual ~DB_UpdateBreakAwayUvList();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, char* play_info);
};

#endif  // OLDSRV_DB_DB_UPDATE_BREAK_AWAY_UV_LIST_H_
