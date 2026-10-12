#ifndef OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_DB_SAVE_CERASHOP_ADD_RESTRICT_TYPE_H_
#define OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_DB_SAVE_CERASHOP_ADD_RESTRICT_TYPE_H_

#include "old_data/db/core/db_dispatcher.h"

class CUser;
namespace CerashopAddRestrict {
struct DBInputData;
}

// IDA unit: DB_SaveCerashopAddRestrictType.  The class contains no state
// beyond DBDispatcher's vptr on i386.
class DB_SaveCerashopAddRestrictType : public DBDispatcher {
public:
    // IDA_EA: 0x844FF66
    DB_SaveCerashopAddRestrictType();

    // IDA_EA: 0x80E472C / 0x80E475C
    virtual ~DB_SaveCerashopAddRestrictType();

    // IDA_EA: 0x80E14F6
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x80E17BA
    static void makeRequest(CUser* user,
                            CerashopAddRestrict::DBInputData& input);
};

#if defined(__i386__)
static_assert(sizeof(DB_SaveCerashopAddRestrictType) == 0x04u,
              "DB_SaveCerashopAddRestrictType must be one DBDispatcher vptr");
#endif

#endif  // OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_DB_SAVE_CERASHOP_ADD_RESTRICT_TYPE_H_
