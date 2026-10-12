#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SAVE_OBJECT_BRING_UP_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SAVE_OBJECT_BRING_UP_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_SaveObjectBringUp.
class DB_SaveObjectBringUp : public DBDispatcher {
public:
    DB_SaveObjectBringUp();
    virtual ~DB_SaveObjectBringUp();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_SAVE_OBJECT_BRING_UP_H_
