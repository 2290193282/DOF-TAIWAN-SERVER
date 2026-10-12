#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_ONTIME_REWARD_USER_LOG_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_ONTIME_REWARD_USER_LOG_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 11_event_runtime/DB_InsertOnTimeRewardUserLog.
// IDA_EA: dispatch @0x8442A56; makeRequest @0x8442B7A; ctor/dtor
// @0x844F962/@0x845B968/@0x845B998. DBTask slot 664.
class DB_InsertOnTimeRewardUserLog : public DBDispatcher {
public:
    DB_InsertOnTimeRewardUserLog();
    virtual ~DB_InsertOnTimeRewardUserLog();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            int receive_no);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_ONTIME_REWARD_USER_LOG_H_
