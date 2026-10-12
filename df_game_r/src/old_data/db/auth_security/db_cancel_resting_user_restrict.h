#ifndef OLDSRV_DB_DB_CANCEL_RESTING_USER_RESTRICT_H_
#define OLDSRV_DB_DB_CANCEL_RESTING_USER_RESTRICT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


struct SIG_RESTING_USER_RESTRICT {
    uint32_t account_id;
};

static_assert(sizeof(SIG_RESTING_USER_RESTRICT) == 4,
              "SIG_RESTING_USER_RESTRICT IDA ABI size must stay 4");

// IDA unit: 13_database_dispatch/DB_CancelRestingUserRestrict.
// DBTask::RegistDispatcher @0x83FBFEE stores this at old slot +406; active
// signal index is 405. The request payload is old SIG_RESTING_USER_RESTRICT.
class DB_CancelRestingUserRestrict : public DBDispatcher {
public:
    DB_CancelRestingUserRestrict();
    virtual ~DB_CancelRestingUserRestrict();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, int user_idx);
};

#endif  // OLDSRV_DB_DB_CANCEL_RESTING_USER_RESTRICT_H_
