#ifndef OLDSRV_DB_DB_UPDATE_CREATURE_ITEM_LIST_H_
#define OLDSRV_DB_DB_UPDATE_CREATURE_ITEM_LIST_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x841FD12/0x844E366/0x845F3E8/0x845F418.
// IDA unit: 06_userinfo_stat_inventory/DB_UpdateCreatureItemList.
class DB_UpdateCreatureItemList : public DBDispatcher {
public:
    DB_UpdateCreatureItemList();
    virtual ~DB_UpdateCreatureItemList();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_CREATURE_ITEM_LIST_H_
