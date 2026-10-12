#ifndef OLDSRV_DB_DB_UPDATE_BREAK_AWAY_CLEAR_ADD_INFO_H_
#define OLDSRV_DB_DB_UPDATE_BREAK_AWAY_CLEAR_ADD_INFO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843848C/0x844EDAE/0x845CEBC/0x845CEEC.
// IDA unit: 13_database_dispatch/DB_UpdateBreakAwayClearAddInfo.
class DB_UpdateBreakAwayClearAddInfo : public DBDispatcher {
public:
    DB_UpdateBreakAwayClearAddInfo();
    virtual ~DB_UpdateBreakAwayClearAddInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_BREAK_AWAY_CLEAR_ADD_INFO_H_
