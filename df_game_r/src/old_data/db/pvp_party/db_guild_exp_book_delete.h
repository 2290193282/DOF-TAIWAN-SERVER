#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_GUILD_EXP_BOOK_DELETE_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_GUILD_EXP_BOOK_DELETE_H_

#include "old_data/db/core/db_conn_info.h"
#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_GuildExpBookDelete.
// IDA_EA: dispatch @0x843C35A; makeRequest @0x843C408; ctor/dtor
// @0x844F4CA/@0x845C3C4/@0x845C3F4. DBTask slot 521.
class DB_GuildExpBookDelete : public DBDispatcher {
public:
    DB_GuildExpBookDelete();
    virtual ~DB_GuildExpBookDelete();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int charac_no,
                            int guild_secede,
                            ENUM_SERVER_GROUP server_group);
};

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_GUILD_EXP_BOOK_DELETE_H_
