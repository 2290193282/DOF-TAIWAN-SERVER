#ifndef OLDSRV_DB_DB_SAVE_CLIENT_HACK_LOG_H_
#define OLDSRV_DB_DB_SAVE_CLIENT_HACK_LOG_H_

#include "old_data/db/core/db_dispatcher.h"


class MySQL;

class DB_SaveClientHackLog : public DBDispatcher {
public:
    DB_SaveClientHackLog();
    virtual ~DB_SaveClientHackLog();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

private:
    int _getTodaysHackCount(MySQL* db_handle,
                            unsigned int account_id,
                            unsigned short hack_type);
    void _sendResult(unsigned int account_id);
    void _sendHackTypeResult(unsigned int account_id,
                             int hack_type,
                             int todays_hack_count);
};

#endif  // OLDSRV_DB_DB_SAVE_CLIENT_HACK_LOG_H_
