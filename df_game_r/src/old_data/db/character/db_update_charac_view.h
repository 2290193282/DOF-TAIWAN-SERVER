#ifndef OLDSRV_DB_DB_UPDATE_CHARAC_VIEW_H_
#define OLDSRV_DB_DB_UPDATE_CHARAC_VIEW_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8408848/0x844D85C/0x844D878/0x844D8A8.
// IDA unit: 13_database_dispatch/DB_UpdateCharacView.
class DB_UpdateCharacView : public DBDispatcher {
public:
    DB_UpdateCharacView();
    virtual ~DB_UpdateCharacView();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_CHARAC_VIEW_H_
