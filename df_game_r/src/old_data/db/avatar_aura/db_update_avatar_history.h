#ifndef OLDSRV_DB_DB_UPDATE_AVATAR_HISTORY_H_
#define OLDSRV_DB_DB_UPDATE_AVATAR_HISTORY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 06_userinfo_stat_inventory/DB_UpdateAvatarHistory.
class DB_UpdateAvatarHistory : public DBDispatcher {
public:
    // IDA_EA: 0x844FA5E.
    DB_UpdateAvatarHistory();
    // IDA_EA: 0x845B524 / 0x845B554.
    virtual ~DB_UpdateAvatarHistory();

    // IDA_EA: 0x8445246.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    // IDA_EA: 0x844536A.
    static void makeRequest(unsigned int charac_no, int avatar_ui_id,
                            char* ipg_agency_no);
};

#endif  // OLDSRV_DB_DB_UPDATE_AVATAR_HISTORY_H_
