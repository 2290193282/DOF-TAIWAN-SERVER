#ifndef OLDSRV_DB_DB_UPDATE_LOGOUT_H_
#define OLDSRV_DB_DB_UPDATE_LOGOUT_H_

#include "old_data/db/core/db_dispatcher.h"
#include "infra/network/packet/logout_payload.h"

// IDA unit: 13_database_dispatch/DB_UpdateLogout.
class DB_UpdateLogout : public DBDispatcher {
public:
    DB_UpdateLogout();
    virtual ~DB_UpdateLogout();

    int UpdateAccountMemberBonusFatigue(unsigned int account_id,
                                        short bonus_fatigue);
    bool SendResult(int signal, int user_idx, SIG_UPDATE_LOGOUT* logout);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool UpdateLogout(SIG_UPDATE_LOGOUT* logout);
};

#endif  // OLDSRV_DB_DB_UPDATE_LOGOUT_H_
