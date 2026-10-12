#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ACCOUNT_FATIGUE_STAT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ACCOUNT_FATIGUE_STAT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


namespace event_account_fatigue_stat {

// Slot 851 is inside both original 852-entry routing arrays and was empty in
// the restored DBTask and InterDispatcher registries.  This is an internal
// 0828 compatibility signal; it is not claimed as an original 0627 symbol.
enum {
    kDbSignal = 851,
    kDbHandle = 9,
    kErrorOperationFailed = 1,
};

// Fixed DB_Q / DISPATCH_Q payload.  The account id is repeated on the reply
// so DispatchThread can reject a stale or cross-account result before sending
// any client packet.
struct Result {
    uint32_t account_id;
    uint32_t fatigue_quantity;
    uint8_t result;
    uint8_t reserved[3];
};

static_assert(sizeof(Result) == 12u,
              "CMD710 internal DB result must remain a fixed 12-byte ABI");

class DB_EventAccountFatigueStat : public DBDispatcher {
public:
    DB_EventAccountFatigueStat();
    virtual ~DB_EventAccountFatigueStat();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool makeRequest(int user_idx, uint32_t account_id);
};

}  // namespace event_account_fatigue_stat

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ACCOUNT_FATIGUE_STAT_H_
