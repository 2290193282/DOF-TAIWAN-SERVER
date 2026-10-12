#ifndef OLDSRV_DB_DB_HADES_PUNISH_H_
#define OLDSRV_DB_DB_HADES_PUNISH_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// The old typed Stream reserves 36 bytes: five dwords followed by one
// bounded, NUL-terminated IPv4/reason field.  Keep the physical record in a
// shared header so producers, consumers and StreamGuard derive one extent.
#pragma pack(push, 1)
struct SIG_HADES_PUNISH {
    uint32_t account_id;     // +0x00
    uint32_t punish_kind;    // +0x04
    uint32_t punish_days;    // +0x08
    uint32_t apply_switch;   // +0x0c
    uint32_t reserved;       // +0x10
    char reason[16];         // +0x14
};
#pragma pack(pop)

static_assert(sizeof(SIG_HADES_PUNISH) == 0x24u,
              "Hades DB request ABI must stay 36 bytes");


// IDA_EA: 0x8445AA6/0x8445DD4/0x844FAB2/0x845B43A/0x845B46A.
// IDA unit: 02_login_account/DB_HadesPunish.
class DB_HadesPunish : public DBDispatcher {
public:
    DB_HadesPunish();
    virtual ~DB_HadesPunish();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int punish_kind,
                            unsigned int punish_days,
                            unsigned int apply_switch,
                            unsigned int reserved,
                            char* reason);
};

#endif  // OLDSRV_DB_DB_HADES_PUNISH_H_
