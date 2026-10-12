#ifndef OLDSRV_DB_DB_INSERT_CREATURE_ITEM_H_
#define OLDSRV_DB_DB_INSERT_CREATURE_ITEM_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/creature/creature_item_payload.h"


class MySQL;

// IDA_EA: 0x841F8D4/0x841FA28/0x844E34A/0x845F436/0x845F466.
// IDA unit: 06_userinfo_stat_inventory/DB_InsertCreatureItem.
class DB_InsertCreatureItem : public DBDispatcher {
public:
    DB_InsertCreatureItem();
    virtual ~DB_InsertCreatureItem();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static bool insertCreatureItem(MySQL* db_handle, const SIG_CREATURE_ITEM* payload);
};

#endif  // OLDSRV_DB_DB_INSERT_CREATURE_ITEM_H_
