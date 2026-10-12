#ifndef OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_DB_LOAD_CERASHOP_ADD_RESTRICT_TYPE_H_
#define OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_DB_LOAD_CERASHOP_ADD_RESTRICT_TYPE_H_

#include "old_data/db/core/db_dispatcher.h"

class CUser;

// IDA unit: DB_LoadCerashopAddRestrictType.  The class contains no state
// beyond DBDispatcher's vptr on i386.
class DB_LoadCerashopAddRestrictType : public DBDispatcher {
public:
    // IDA_EA: 0x844FF82
    DB_LoadCerashopAddRestrictType();

    // IDA_EA: 0x80E46DE / 0x80E470E
    virtual ~DB_LoadCerashopAddRestrictType();

    // IDA_EA: 0x80E1ACA
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x80E1FF6
    static void makeRequest(CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(DB_LoadCerashopAddRestrictType) == 0x04u,
              "DB_LoadCerashopAddRestrictType must be one DBDispatcher vptr");
#endif

#endif  // OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_DB_LOAD_CERASHOP_ADD_RESTRICT_TYPE_H_
