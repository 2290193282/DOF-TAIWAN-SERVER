#ifndef OLDSRV_DB_DB_CHECK_3RD_PARTY_CONSENT_H_
#define OLDSRV_DB_DB_CHECK_3RD_PARTY_CONSENT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 09_dungeon_pvp_party/DB_Check3rdPartyConcent.
// Signal 695 is the DB/Inter queue record used by the CMD451 consent path.
class DB_Check3rdPartyConcent : public DBDispatcher {
public:
    DB_Check3rdPartyConcent();
    virtual ~DB_Check3rdPartyConcent();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int account_id);
};

#endif  // OLDSRV_DB_DB_CHECK_3RD_PARTY_CONSENT_H_
