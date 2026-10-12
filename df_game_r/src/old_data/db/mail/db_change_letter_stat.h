#ifndef OLDSRV_DB_DB_CHANGE_LETTER_STAT_H_
#define OLDSRV_DB_DB_CHANGE_LETTER_STAT_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/mail/letter_stat_payload.h"


// IDA unit: 13_database_dispatch/DB_ChangeLetterStat.
// IDA_EA: dispatch @0x842602E; makeRequest @0x84261FC; ctor/dtor
// @0x844E622/@0x845EBAE/@0x845EBDE.
class DB_ChangeLetterStat : public DBDispatcher {
public:
    DB_ChangeLetterStat();
    virtual ~DB_ChangeLetterStat();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int signal,
                            int user_idx,
                            int charac_no,
                            int letter_id,
                            int letter_stat);
};

#endif  // OLDSRV_DB_DB_CHANGE_LETTER_STAT_H_
