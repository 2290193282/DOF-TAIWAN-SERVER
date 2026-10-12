#ifndef OLDSRV_DB_DB_SWAP_CREATURE_ITEM_H_
#define OLDSRV_DB_DB_SWAP_CREATURE_ITEM_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x841FE0C/0x841FEF2/0x844E39E/0x845F39A/0x845F3CA.
// IDA unit: 06_userinfo_stat_inventory/DB_SwapCreatureItem.
class DB_SwapCreatureItem : public DBDispatcher {
public:
    DB_SwapCreatureItem();
    virtual ~DB_SwapCreatureItem();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int ui_id, int slot);
};

#endif  // OLDSRV_DB_DB_SWAP_CREATURE_ITEM_H_
