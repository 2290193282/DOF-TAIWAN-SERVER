#ifndef OLDSRV_DB_DB_SAVE_SKILL_H_
#define OLDSRV_DB_DB_SAVE_SKILL_H_

#include "old_data/db/core/db_dispatcher.h"

struct SIG_LOAD_SKILL;


// IDA_EA: 0x8417770/0x841778C/0x84177BC/0x84177DA/0x841782A/0x8417A08.
// IDA unit: 13_database_dispatch/DB_SaveSkill.
class DB_SaveSkill : public DBDispatcher {
public:
    DB_SaveSkill();
    virtual ~DB_SaveSkill();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool SaveSkill(SIG_LOAD_SKILL* payload);
    bool SendResult(int signal, int user_idx, SIG_LOAD_SKILL* payload);
};

#endif  // OLDSRV_DB_DB_SAVE_SKILL_H_
