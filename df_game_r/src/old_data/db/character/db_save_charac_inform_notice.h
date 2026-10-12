#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_DB_SAVE_CHARAC_INFORM_NOTICE_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_DB_SAVE_CHARAC_INFORM_NOTICE_H_

#include "old_data/db/core/db_dispatcher.h"

class DB_SaveCharacInformNotice : public DBDispatcher {
public:
    DB_SaveCharacInformNotice();
    virtual ~DB_SaveCharacInformNotice();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_DB_SAVE_CHARAC_INFORM_NOTICE_H_
