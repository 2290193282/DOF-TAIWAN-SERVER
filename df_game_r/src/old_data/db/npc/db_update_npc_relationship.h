#ifndef OLDSRV_DB_DB_UPDATE_NPC_RELATIONSHIP_H_
#define OLDSRV_DB_DB_UPDATE_NPC_RELATIONSHIP_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 07_npc_shop_dialog/DB_UpdateNPCRelationship.
// IDA_EA: dispatch @0x8435E1E; ctor/dtor @0x844EF6E/@0x845D486/@0x845D4B6.
class DB_UpdateNPCRelationship : public DBDispatcher {
public:
    DB_UpdateNPCRelationship();
    virtual ~DB_UpdateNPCRelationship();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_NPC_RELATIONSHIP_H_
