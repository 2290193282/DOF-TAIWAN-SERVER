#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REPORT_MANNERLESS_USER_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REPORT_MANNERLESS_USER_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>

// IDA_EA: Stream::GetOutBuffer<SIG_MANNERLESS_USER> @0x84521E6.  The old
// specialization calls enable_out(314) before exposing the record.  This DB
// leaf only consumes the three NUL-terminated SQL fragments at +0/+0x1d/+0x3a;
// the trailing bytes are intentionally opaque because the function does not
// inspect them.
#pragma pack(push, 1)
struct SIG_MANNERLESS_USER {
    char sql_fragment_00[29];  // +0x00
    char sql_fragment_1d[29];  // +0x1d
    char sql_fragment_3a[29];  // +0x3a
    uint8_t opaque_tail[0x13a - 0x57];
};
#pragma pack(pop)

static_assert(sizeof(SIG_MANNERLESS_USER) == 0x13au,
              "SIG_MANNERLESS_USER must match Stream::GetOutBuffer");
static_assert(offsetof(SIG_MANNERLESS_USER, sql_fragment_1d) == 0x1du &&
                  offsetof(SIG_MANNERLESS_USER, sql_fragment_3a) == 0x3au,
              "SIG_MANNERLESS_USER consumed field offsets must match IDA");

// IDA unit: 13_database_dispatch/DB_ReportMannerlessUser.
// DBTask original slot 203.
class DB_ReportMannerlessUser : public DBDispatcher {
public:
    // IDA_EA: 0x844E6AE
    // IDA_NAME: _ZN23DB_ReportMannerlessUserC2Ev
    // DEMANGLED: DB_ReportMannerlessUser::DB_ReportMannerlessUser(void)
    DB_ReportMannerlessUser();

    // IDA_EA: 0x845E9DA
    // IDA_NAME: _ZN23DB_ReportMannerlessUserD2Ev
    // DEMANGLED: DB_ReportMannerlessUser::~DB_ReportMannerlessUser()
    // IDA_EA: 0x845EA0A
    // IDA_NAME: _ZN23DB_ReportMannerlessUserD0Ev
    // DEMANGLED: DB_ReportMannerlessUser::~DB_ReportMannerlessUser()
    virtual ~DB_ReportMannerlessUser();

    // IDA_EA: 0x842796A
    // IDA_NAME: _ZN23DB_ReportMannerlessUser8dispatchEiiP6Stream
    // DEMANGLED: DB_ReportMannerlessUser::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REPORT_MANNERLESS_USER_H_
