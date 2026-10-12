#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_ITEM_DROP_STATISTICS_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_ITEM_DROP_STATISTICS_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_UpdateItemDropStatistics.
// IDA_EA: dispatch @0x84285AE; makeRequest @0x842894C; ctor/dtor
// @0x844E78E/@0x845E7B8/@0x845E7E8. DBTask slot 210.
class DB_UpdateItemDropStatistics : public DBDispatcher {
public:
    enum { kGradeCount = 6 };

    DB_UpdateItemDropStatistics();
    virtual ~DB_UpdateItemDropStatistics();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int channel_no,
                            int drop_type,
                            unsigned int* stackable,
                            unsigned int* equipment,
                            unsigned int* recipe,
                            unsigned int* artifact);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_ITEM_DROP_STATISTICS_H_
