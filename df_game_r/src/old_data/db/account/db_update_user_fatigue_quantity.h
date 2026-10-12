#ifndef OLDSRV_DB_DB_UPDATE_USER_FATIGUE_QUANTITY_H_
#define OLDSRV_DB_DB_UPDATE_USER_FATIGUE_QUANTITY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8441A8E/0x8441C2A/0x844F866/0x845BB3C/0x845BB6C.
// IDA unit: 13_database_dispatch/DB_UpdateUserFatigueQuantity.
class DB_UpdateUserFatigueQuantity : public DBDispatcher {
public:
    DB_UpdateUserFatigueQuantity();
    virtual ~DB_UpdateUserFatigueQuantity();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    // Returns true only after the fixed request is accepted by DB_Q.  The
    // caller keeps its in-memory delta when the queue is unavailable.
    static bool makeRequest(unsigned int account_id,
                            unsigned int fatigue_quantity);
};

#endif  // OLDSRV_DB_DB_UPDATE_USER_FATIGUE_QUANTITY_H_
