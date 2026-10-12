#ifndef OLDSRV_DB_DB_SELECT_CHARAC_MID_H_
#define OLDSRV_DB_DB_SELECT_CHARAC_MID_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// IDA_EA: Stream::GetOutBuffer<SIG_CHECK_CHAR_MID> @0x84543DA and
// CStreamGuard::GetInBuffer<SIG_CHECK_CHAR_MID> @0x8454306.
struct SIG_CHECK_CHAR_MID {
    uint32_t request_id;
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
    char charac_name[30];
};

struct SIG_CHECK_CHAR_MID_RESULT {
    uint32_t request_id;
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
    uint32_t account_id;
};

static_assert(sizeof(SIG_CHECK_CHAR_MID) == 48,
              "SIG_CHECK_CHAR_MID IDA ABI size must stay 48");
static_assert(sizeof(SIG_CHECK_CHAR_MID_RESULT) == 20,
              "SIG_CHECK_CHAR_MID_RESULT IDA ABI size must stay 20");


// IDA unit: 04_character_select_create/DB_SelectCharacMid.
// IDA_EA: dispatch @0x8446516; makeRequest @0x844676C; ctor/dtor
// @0x844FB06/@0x845B350/@0x845B380. DBTask slot 697.
class DB_SelectCharacMid : public DBDispatcher {
public:
    DB_SelectCharacMid();
    virtual ~DB_SelectCharacMid();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int request_id,
                            const char* charac_name,
                            unsigned int value1,
                            unsigned int value2,
                            unsigned int value3);
};

#endif  // OLDSRV_DB_DB_SELECT_CHARAC_MID_H_
