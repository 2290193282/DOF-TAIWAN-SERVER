#ifndef OLDSRV_DB_DB_UPDATE_GROW_TYPE_H_
#define OLDSRV_DB_DB_UPDATE_GROW_TYPE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


#pragma pack(push, 1)
// IDA_EA: 0x8450DD0
// IDA_NAME: _ZN6Stream12GetOutBufferI19SIG_UPDATE_GROWTYPEEEPT_v
// Old SIG_UPDATE_GROWTYPE payload size is 5 bytes.
struct SIG_UPDATE_GROWTYPE {
    uint32_t charac_no;
    int8_t grow_type;
};
#pragma pack(pop)

// IDA unit: 13_database_dispatch/DB_UpdateGrowType.
class DB_UpdateGrowType : public DBDispatcher {
public:
    DB_UpdateGrowType();
    virtual ~DB_UpdateGrowType();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool UpdateGrowType(SIG_UPDATE_GROWTYPE* payload);
};

#endif  // OLDSRV_DB_DB_UPDATE_GROW_TYPE_H_
