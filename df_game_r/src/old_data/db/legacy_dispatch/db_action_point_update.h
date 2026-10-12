#ifndef OLDSRV_DB_DB_ACTION_POINT_UPDATE_H_
#define OLDSRV_DB_DB_ACTION_POINT_UPDATE_H_

#include "old_data/db/legacy_dispatch/action_point_payload.h"
#include "old_data/db/core/db_dispatcher.h"

#include <string>

namespace APSystem {

// IDA_EA: 0x812431C/0x812432A/0x8124386/0x81246D6/0x844DD96.
// IDA unit: 08_quest_skill_growth/APSystem/DB_UpdateActionPoint, registered
// from 99_misc_global/DBTask as DB signal 19.
class DB_UpdateActionPoint : public DBDispatcher {
public:
    DB_UpdateActionPoint();
    virtual ~DB_UpdateActionPoint();

    static const _SIG_LOAD_ACTION_POINT* makeRequest(
                                            int user_idx,
                                            const _SIG_LOAD_ACTION_POINT& payload,
                                            bool previous_day);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool Update(bool previous_day, const _SIG_LOAD_ACTION_POINT& payload);
    void MakeActionClearStateString(std::string& text,
                                    const _ActionPoint* action_points,
                                    unsigned int count);
};

}  // namespace APSystem
#endif  // OLDSRV_DB_DB_ACTION_POINT_UPDATE_H_
