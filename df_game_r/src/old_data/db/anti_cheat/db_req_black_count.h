#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REQ_BLACK_COUNT_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REQ_BLACK_COUNT_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"

// IDA_EA: Stream::GetInBuffer<SIG_REQ_BLACK_COUNT> @0x8239A18,
// CStreamGuard::GetInBuffer<SIG_REQ_BLACK_COUNT> @0x8236DCA, and
// Stream::GetOutBuffer<SIG_REQ_BLACK_COUNT> @0x8451EEE all reserve exactly
// 16 bytes.  RequestBlackCount @0x81C79F2 zeroes the whole record and writes
// charac_no; CUser::GetBlackListCount @0x867F1D4 subsequently writes result
// at +0x04 and the current character's blacklist count at +0x08.  The final
// dword has no semantic writer in the recovered path and remains reserved.
struct SIG_REQ_BLACK_COUNT {
    uint32_t charac_no;       // +0x00
    uint32_t result;          // +0x04
    uint32_t black_count;     // +0x08
    uint32_t reserved_0c;     // +0x0C
};

static_assert(sizeof(SIG_REQ_BLACK_COUNT) == 0x10u,
              "SIG_REQ_BLACK_COUNT IDA ABI size must stay 0x10");
static_assert(offsetof(SIG_REQ_BLACK_COUNT, charac_no) == 0x00u,
              "SIG_REQ_BLACK_COUNT charac_no offset must stay 0x00");
static_assert(offsetof(SIG_REQ_BLACK_COUNT, result) == 0x04u,
              "SIG_REQ_BLACK_COUNT result offset must stay 0x04");
static_assert(offsetof(SIG_REQ_BLACK_COUNT, black_count) == 0x08u,
              "SIG_REQ_BLACK_COUNT black-count offset must stay 0x08");
static_assert(offsetof(SIG_REQ_BLACK_COUNT, reserved_0c) == 0x0cu,
              "SIG_REQ_BLACK_COUNT reserved tail offset must stay 0x0C");

// IDA_EA: 0x844E542/0x845EE1E/0x845EE4E/0x8424CE6.
// IDA unit: 13_database_dispatch/DB_ReqBlackCount.
class DB_ReqBlackCount : public DBDispatcher {
public:
    DB_ReqBlackCount();
    virtual ~DB_ReqBlackCount();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REQ_BLACK_COUNT_H_
