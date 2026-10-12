#ifndef OLDSRV_DB_DB_UPDATE_ONTIME_LAST_RECV_IDX_H_
#define OLDSRV_DB_DB_UPDATE_ONTIME_LAST_RECV_IDX_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateOntimeLastRecvIdx.
// IDA_EA: dispatch @0x844280A; makeRequest @0x8442966; ctor/dtor
// @0x844F946/@0x845B9B6/@0x845B9E6. makeRequest emits signal 0x297.
class DB_UpdateOntimeLastRecvIdx : public DBDispatcher {
public:
    DB_UpdateOntimeLastRecvIdx();
    virtual ~DB_UpdateOntimeLastRecvIdx();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            int last_recv_idx);
};

#endif  // OLDSRV_DB_DB_UPDATE_ONTIME_LAST_RECV_IDX_H_
