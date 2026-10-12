#ifndef OLDSRV_DB_DB_SAVE_USER_HANDICAP_H_
#define OLDSRV_DB_DB_SAVE_USER_HANDICAP_H_

#include "old_data/db/core/db_dispatcher.h"


class MySQL;

// IDA unit: 13_database_dispatch/DB_SaveUserHandicap.
// IDA_EA: _getDBHandle @0x8427DFC; _getDBHandleForDevideAccountDB @0x8427E28;
// dispatch @0x8427E54; makeRequest @0x842801A; ctor/dtor
// @0x844E702/@0x845E8F0/@0x845E920.
class DB_SaveUserHandicap : public DBDispatcher {
public:
    DB_SaveUserHandicap();
    virtual ~DB_SaveUserHandicap();

    static MySQL* _getDBHandle();
    static MySQL* _getDBHandleForDevideAccountDB();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            int event_id,
                            int punish_type,
                            int punish_value,
                            int duration_days,
                            int src);
};

#endif  // OLDSRV_DB_DB_SAVE_USER_HANDICAP_H_
