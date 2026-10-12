#ifndef OLDSRV_DB_DB_WRITE_GEO_REJECT_H_
#define OLDSRV_DB_DB_WRITE_GEO_REJECT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844BB70/0x844EB2A/0x845AB16/0x845AB46.
// IDA unit: 13_database_dispatch/DB_WriteGeoReject.
class DB_WriteGeoReject : public DBDispatcher {
public:
    DB_WriteGeoReject();
    virtual ~DB_WriteGeoReject();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_WRITE_GEO_REJECT_H_
