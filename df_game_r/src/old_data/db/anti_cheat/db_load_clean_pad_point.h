#ifndef OLDSRV_DB_DB_LOAD_CLEAN_PAD_POINT_H_
#define OLDSRV_DB_DB_LOAD_CLEAN_PAD_POINT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


enum {
    CLEANPAD_HACK_TYPE_MIN = 101,
    CLEANPAD_HACK_TYPE_MAX = 4000,
    CLEANPAD_HACK_TYPE_COUNT =
        CLEANPAD_HACK_TYPE_MAX - CLEANPAD_HACK_TYPE_MIN + 1,
};

// IDA_EA: 0x8452A04
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_LOAD_CLEANPAD_POINTEEPT_v
// The DB hack_type key 101..4000 maps directly to this 3900-entry value table.
struct SIG_LOAD_CLEANPAD_POINT {
    int32_t value_by_hack_type[CLEANPAD_HACK_TYPE_COUNT];
};

static_assert(sizeof(SIG_LOAD_CLEANPAD_POINT) == 0x3CF0,
              "SIG_LOAD_CLEANPAD_POINT ABI drift");


// IDA_EA: 0x842ECF4/0x842EFA0/0x844EB62/0x845DE94/0x845DEC4.
// IDA unit: 13_database_dispatch/DB_LoadCleanPadPoint.
class DB_LoadCleanPadPoint : public DBDispatcher {
public:
    DB_LoadCleanPadPoint();
    virtual ~DB_LoadCleanPadPoint();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest();
};

#endif  // OLDSRV_DB_DB_LOAD_CLEAN_PAD_POINT_H_
