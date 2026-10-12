#ifndef OLDSRV_DB_DB_INSERT_UNLIMIT_SUPPORT_LOG_H_
#define OLDSRV_DB_DB_INSERT_UNLIMIT_SUPPORT_LOG_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


struct SIG_INSERT_UNLIMIT_SUPPORT_LOG {
    uint32_t account_id;
    uint32_t server_id;
    uint32_t charac_no;
    char charac_name[29];
};

static_assert(sizeof(SIG_INSERT_UNLIMIT_SUPPORT_LOG) == 44,
              "SIG_INSERT_UNLIMIT_SUPPORT_LOG IDA ABI size must stay 44");


// IDA_EA: 0x842EB2E/0x842EBD0/0x844EB46/0x845DEE2/0x845DF12.
// IDA unit: 13_database_dispatch/DB_InsertUnlimitSupportLog.
class DB_InsertUnlimitSupportLog : public DBDispatcher {
public:
    DB_InsertUnlimitSupportLog();
    virtual ~DB_InsertUnlimitSupportLog();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int charac_no,
                            const char* charac_name);
};

#endif  // OLDSRV_DB_DB_INSERT_UNLIMIT_SUPPORT_LOG_H_
