#ifndef OLDSRV_DB_DB_LOAD_QUEST_H_
#define OLDSRV_DB_DB_LOAD_QUEST_H_

#include "old_data/db/core/db_dispatcher.h"

struct SIG_LOAD_QUEST;


// IDA_EA: 0x840E236/0x840E39A/0x844D930/0x844D94C/0x844D97C.
// IDA unit: 13_database_dispatch/DB_LoadQuest.
class DB_LoadQuest : public DBDispatcher {
public:
    DB_LoadQuest();
    virtual ~DB_LoadQuest();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool GetUserQuest(SIG_LOAD_QUEST* payload);
};

#endif  // OLDSRV_DB_DB_LOAD_QUEST_H_
