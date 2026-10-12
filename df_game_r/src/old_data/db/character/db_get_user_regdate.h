#ifndef OLDSRV_DB_DB_GET_USER_REGDATE_H_
#define OLDSRV_DB_DB_GET_USER_REGDATE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// IDA_EA: Stream::GetOutBuffer<SIG_GET_USER_REGDATE> @0x8442E82 and
// CStreamGuard::GetInBuffer<SIG_GET_USER_REGDATE> @0x8443025.
// The three dwords are written/read directly by dispatch/makeRequest.
struct SIG_GET_USER_REGDATE {
    uint32_t account_id;       // +0x00
    uint32_t regdate_hour;     // +0x04, YYYYMMDD after /100
    uint32_t regdate_seconds;  // +0x08, member_info.reg_date
};

static_assert(sizeof(SIG_GET_USER_REGDATE) == 0x0Cu,
              "SIG_GET_USER_REGDATE IDA ABI size must stay 0x0C");


// IDA unit: 13_database_dispatch/DB_GetUserRegdate.
// IDA_EA: dispatch @0x8442E68; makeRequest @0x8442FAE; SendResult
// @0x844309C; ctor/dtor @0x844F92A/@0x845B8CC/@0x845B8FC.
// NOTE: raw bytes at 0x8442FF5 are C7 44 24 04 A1 02 00 00, so the full
// request signal is 0x2A1 (673).  IDA's bad operator<< argument type displays
// only the low byte (161); DBTask stores this dispatcher at slot 673.
class DB_GetUserRegdate : public DBDispatcher {
public:
    DB_GetUserRegdate();
    virtual ~DB_GetUserRegdate();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int account_id);
    bool SendResult(int signal, int user_idx, SIG_GET_USER_REGDATE* payload);
};

#endif  // OLDSRV_DB_DB_GET_USER_REGDATE_H_
