#ifndef OLDSRV_DB_DB_GET_PREMIUM_SELECTED_CUBE_INFO_H_
#define OLDSRV_DB_DB_GET_PREMIUM_SELECTED_CUBE_INFO_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// Wire records shared by CUser::requestCubePremiumInfoIfContracted
// (0x86467EE) and the DB dispatcher (0x844C4E2).  The request contains the
// selected character number only; the reply appends the two one-byte cube
// selection fields and retains the two-byte physical Stream tail.
#pragma pack(push, 1)
struct SIG_GET_PREMIUM_SELECTED_CUBE_INFO {
    uint32_t charac_no;  // +0x00
};

struct SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES {
    uint32_t charac_no;  // +0x00
    uint8_t selected;    // +0x04
    uint8_t cube_type;   // +0x05
    uint8_t reserved_06[2]; // +0x06, old typed Stream extent
};
#pragma pack(pop)

static_assert(sizeof(SIG_GET_PREMIUM_SELECTED_CUBE_INFO) == 0x04u,
              "premium selected-cube request ABI must remain four bytes");
static_assert(sizeof(SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES) == 0x08u,
              "premium selected-cube reply ABI must remain eight bytes");


// IDA unit: 13_database_dispatch/DB_GetPremiumSelectedCubeInfo.
// IDA_EA: dispatch @0x844C4E2; ctor/dtor
// @0x844FF2E/@0x845A8C2/@0x845A8F2.
class DB_GetPremiumSelectedCubeInfo : public DBDispatcher {
public:
    DB_GetPremiumSelectedCubeInfo();
    virtual ~DB_GetPremiumSelectedCubeInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_GET_PREMIUM_SELECTED_CUBE_INFO_H_
