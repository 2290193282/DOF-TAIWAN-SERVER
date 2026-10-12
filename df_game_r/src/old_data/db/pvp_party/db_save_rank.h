#ifndef OLDSRV_DB_DB_SAVE_RANK_H_
#define OLDSRV_DB_DB_SAVE_RANK_H_

#include "old_data/db/core/db_dispatcher.h"


struct SIG_SAVE_RANK;

// IDA_EA: 0x844D99A/0x844D9B6/0x844D9E6/0x84174C0/0x8417510.
// IDA unit: 13_database_dispatch/DB_SaveRank.
class DB_SaveRank : public DBDispatcher {
public:
    DB_SaveRank();
    virtual ~DB_SaveRank();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool SaveRankAll(SIG_SAVE_RANK* rank);
};

#endif  // OLDSRV_DB_DB_SAVE_RANK_H_
