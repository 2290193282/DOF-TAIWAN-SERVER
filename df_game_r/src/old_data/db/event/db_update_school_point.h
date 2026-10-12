#ifndef OLDSRV_DB_DB_UPDATE_SCHOOL_POINT_H_
#define OLDSRV_DB_DB_UPDATE_SCHOOL_POINT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// IDA wire record used by Stream/CStreamGuard GetOutBuffer/GetInBuffer at
// 0x8452A1A/0x8452A6E. Both fields are serialized as four-byte integers.
struct SIG_UPDATE_SCHOOL_POINT {
    uint32_t account_id;
    int32_t point;
};

static_assert(sizeof(SIG_UPDATE_SCHOOL_POINT) == 8u,
              "SIG_UPDATE_SCHOOL_POINT IDA ABI size must stay 8");


// IDA_EA: 0x842F05E/0x842F0F0/0x844EB7E/0x845DE46/0x845DE76.
// IDA unit: 13_database_dispatch/DB_UpdateSchoolPoint.
class DB_UpdateSchoolPoint : public DBDispatcher {
public:
    DB_UpdateSchoolPoint();
    virtual ~DB_UpdateSchoolPoint();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            int cur_charac_school_point,
                            int uid);
};

#endif  // OLDSRV_DB_DB_UPDATE_SCHOOL_POINT_H_
