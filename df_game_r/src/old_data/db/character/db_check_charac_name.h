#ifndef OLDSRV_DB_DB_CHECK_CHARAC_NAME_H_
#define OLDSRV_DB_DB_CHECK_CHARAC_NAME_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// IDA_EA: Stream::GetOutBuffer<SIG_CHECK_CHARAC_NAME> @0x84543C4 and
// CStreamGuard::GetInBuffer<SIG_CHECK_CHARAC_NAME> @0x8453EF2.
struct SIG_CHECK_CHARAC_NAME {
    uint32_t account_id;
    char charac_name[30];
    uint8_t flag;
};

// IDA_EA: CStreamGuard::GetInBuffer<SIG_GOLD_CONTROL_EXCEPTION_MID>
// @0x8453EDC.
struct SIG_GOLD_CONTROL_EXCEPTION_MID {
    uint8_t is_same_mid;
    uint8_t flag;
};

static_assert(sizeof(SIG_CHECK_CHARAC_NAME) == 36,
              "SIG_CHECK_CHARAC_NAME IDA ABI size must stay 36");
static_assert(sizeof(SIG_GOLD_CONTROL_EXCEPTION_MID) == 2,
              "SIG_GOLD_CONTROL_EXCEPTION_MID IDA ABI size must stay 2");


// IDA unit: 04_character_select_create/DB_CheckCharacName.
// IDA_EA: dispatch @0x844317E; makeRequest @0x8443352; ctor/dtor
// @0x844F97E/@0x845B87E/@0x845B8AE. DBTask slot 674.
class DB_CheckCharacName : public DBDispatcher {
public:
    DB_CheckCharacName();
    virtual ~DB_CheckCharacName();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            const char* charac_name,
                            bool request_flag);
};

#endif  // OLDSRV_DB_DB_CHECK_CHARAC_NAME_H_
