#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_STATISTIC_ADVANCE_ALTAR_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_STATISTIC_ADVANCE_ALTAR_H_

#include "old_data/db/core/db_dispatcher.h"

namespace advancealtar {

class DB_StatisticAdvanceAltar : public DBDispatcher {
public:
    DB_StatisticAdvanceAltar();
    virtual ~DB_StatisticAdvanceAltar();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

}  // namespace advancealtar

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_STATISTIC_ADVANCE_ALTAR_H_
