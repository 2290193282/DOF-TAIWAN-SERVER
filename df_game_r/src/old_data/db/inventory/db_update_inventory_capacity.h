#ifndef OLDSRV_DB_DB_UPDATE_INVENTORY_CAPACITY_H_
#define OLDSRV_DB_DB_UPDATE_INVENTORY_CAPACITY_H_

#include "old_data/db/core/db_conn_info.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 06_userinfo_stat_inventory/DB_UpdateInventoryCapacity.
// IDA_EA: dispatch @0x8444BEA; makeRequest @0x8444D12; ctor/dtor
// @0x844FA0A/@0x845B60E/@0x845B63E. DBTask slot 686.
class DB_UpdateInventoryCapacity : public DBDispatcher {
public:
    DB_UpdateInventoryCapacity();
    virtual ~DB_UpdateInventoryCapacity();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(ENUM_SERVER_GROUP server_group,
                            int user_idx,
                            unsigned int charac_no,
                            int inventory_capacity);
};

#endif  // OLDSRV_DB_DB_UPDATE_INVENTORY_CAPACITY_H_
