#ifndef OLDSRV_DB_DB_ACTION_POINT_LOAD_H_
#define OLDSRV_DB_DB_ACTION_POINT_LOAD_H_

#include "old_data/db/legacy_dispatch/action_point_payload.h"
#include "old_data/db/core/db_dispatcher.h"

namespace APSystem {

// IDA_EA: 0x8123B10/0x8123B9E/0x8123F50/0x8124156/0x844DD7A.
// IDA unit: 08_quest_skill_growth/APSystem/DB_LoadActionPoint, registered
// from 99_misc_global/DBTask as DB signal 17.
class DB_LoadActionPoint : public DBDispatcher {
public:
    DB_LoadActionPoint();
    virtual ~DB_LoadActionPoint();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool Select(_SIG_LOAD_ACTION_POINT& payload);
    bool ParseActionClearState(char* text,
                               _ActionPoint* action_points,
                               unsigned int count);
    bool SendResult(int signal,
                    int user_idx,
                    _SIG_LOAD_ACTION_POINT& payload,
                    bool result);
};

}  // namespace APSystem
#endif  // OLDSRV_DB_DB_ACTION_POINT_LOAD_H_
