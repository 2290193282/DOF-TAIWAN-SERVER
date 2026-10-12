#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_FATIGUE_ACCELERATION_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_FATIGUE_ACCELERATION_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


namespace fatigue_acceleration_db {

enum {
    kDbSignal = 849,
    kOperationLoad = 1,
    kOperationSave = 2,
    kResultOk = 0,
    kResultFailed = 1,
};

// Fixed internal DB_Q / DISPATCH_Q message.  The token and character number
// let DispatchThread reject a result belonging to an earlier pooled CUser
// session before any client state is changed.
struct Message {
    uint32_t charac_no;
    uint32_t token;
    int32_t server_group;
    uint8_t operation;
    uint8_t requested_enabled;
    uint8_t result;
    uint8_t reserved;
};

static_assert(sizeof(Message) == 16u,
              "CMD556 internal DB message must remain a fixed 16-byte ABI");

class DB_FatigueAcceleration : public DBDispatcher {
public:
    DB_FatigueAcceleration();
    virtual ~DB_FatigueAcceleration();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool Queue(int user_idx, const Message& request);
};

}  // namespace fatigue_acceleration_db

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_FATIGUE_ACCELERATION_0828_H_
