#ifndef OLDSRV_DB_DB_CONN_INFO_H_
#define OLDSRV_DB_DB_CONN_INFO_H_

#include "old_data/db/core/db_types.h"

// IDA: STDBConnInfo constructor @0x80ECD38; total copied size is 0x168.
// The structure is the DB connection record used by CEnvironment,
// CDBConnector, and DBMgr. Offsets are fixed by old df_game_r code.
struct STDBConnInfo {
    STDBConnInfo();

    char db_ip[17];       // offset 0: DB host/IP string, copied with length 0x10/17.
    char pad_17[3];       // offset 17..19: alignment padding before db_port.
    unsigned int db_port; // offset 20: TCP port.
    char db_userid[21];   // offset 24: DB account, copied with length 0x14/21.
    char db_passwd[21];   // offset 45: decrypted DB password, copied with length 0x14/21.
    char db_name[31];     // offset 66: schema name, copied with length 0x1E/31.
    char reserved[256];   // offset 97: unused/copied tail in the old structure.
    unsigned int server_group; // offset 356: server group attached after lookup.
};

#endif  // OLDSRV_DB_DB_CONN_INFO_H_
