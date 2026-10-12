#ifndef OLDSRV_DB_DB_UPDATE_CREATURE_ITEM_H_
#define OLDSRV_DB_DB_UPDATE_CREATURE_ITEM_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/creature/creature_item_payload.h"


class MySQL;

// IDA_EA: 0x84200A8/0x84203E8/0x844E382/0x845F34C/0x845F37C.
// IDA unit: 06_userinfo_stat_inventory/DB_UpdateCreatureItem.
class DB_UpdateCreatureItem : public DBDispatcher {
public:
    DB_UpdateCreatureItem();
    virtual ~DB_UpdateCreatureItem();

    char* GetField(MySQL* db_handle, char* dest, SIG_CREATURE_ITEM* payload);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_CREATURE_ITEM_H_
