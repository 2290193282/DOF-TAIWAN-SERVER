#ifndef OLDSRV_DB_DB_INSERT_ATTENDANCE_EVENT_H_
#define OLDSRV_DB_DB_INSERT_ATTENDANCE_EVENT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 11_event_runtime/DB_InsertAttendanceEvent.
class DB_InsertAttendanceEvent : public DBDispatcher {
public:
    DB_InsertAttendanceEvent();
    virtual ~DB_InsertAttendanceEvent();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            unsigned int charac_no);
};

#endif  // OLDSRV_DB_DB_INSERT_ATTENDANCE_EVENT_H_
