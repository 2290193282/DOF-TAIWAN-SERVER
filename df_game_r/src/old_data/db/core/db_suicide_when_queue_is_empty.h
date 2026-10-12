#ifndef OLDSRV_DB_DB_SUICIDE_WHEN_QUEUE_IS_EMPTY_H_
#define OLDSRV_DB_DB_SUICIDE_WHEN_QUEUE_IS_EMPTY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SuicideWhenQueueIsEmpty.
class DB_SuicideWhenQueueIsEmpty : public DBDispatcher {
public:
    DB_SuicideWhenQueueIsEmpty();
    virtual ~DB_SuicideWhenQueueIsEmpty();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SUICIDE_WHEN_QUEUE_IS_EMPTY_H_
