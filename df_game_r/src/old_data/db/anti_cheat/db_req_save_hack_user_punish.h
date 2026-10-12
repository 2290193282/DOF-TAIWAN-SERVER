#ifndef OLDSRV_DB_DB_REQ_SAVE_HACK_USER_PUNISH_H_
#define OLDSRV_DB_DB_REQ_SAVE_HACK_USER_PUNISH_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844E57A/0x845ED82/0x845EDB2/0x8425374/0x8425494.
// IDA unit: 02_login_account/DB_ReqSaveHackUserPunish.
class DB_ReqSaveHackUserPunish : public DBDispatcher {
public:
    DB_ReqSaveHackUserPunish();
    virtual ~DB_ReqSaveHackUserPunish();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int m_id, unsigned int value,
                            const char* src);
};

#endif  // OLDSRV_DB_DB_REQ_SAVE_HACK_USER_PUNISH_H_
