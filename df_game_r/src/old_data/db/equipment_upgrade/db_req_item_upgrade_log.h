#ifndef DNF_DB_REQ_ITEM_UPGRADE_LOG_H_
#define DNF_DB_REQ_ITEM_UPGRADE_LOG_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_ReqItemUpgradeLog.
class DB_ReqItemUpgradeLog : public DBDispatcher {
public:
    DB_ReqItemUpgradeLog();
    virtual ~DB_ReqItemUpgradeLog();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int server_id,
                            unsigned int item_id,
                            unsigned int charac_no,
                            const char* charac_name,
                            unsigned int develop,
                            bool success);
};

#endif  // DNF_DB_REQ_ITEM_UPGRADE_LOG_H_
