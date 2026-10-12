#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_UPDATE_PVP_GRADE_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_UPDATE_PVP_GRADE_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 09_dungeon_pvp_party/DB_UpdatePvPGrade.
// IDA_EA: dispatch @0x8428B26; makeRequest @0x84293A0; ctor/dtor
// @0x844E7AA/@0x845E76A/@0x845E79A. DBTask slot 212.
class DB_UpdatePvPGrade : public DBDispatcher {
public:
    DB_UpdatePvPGrade();
    virtual ~DB_UpdatePvPGrade();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest();
};

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_UPDATE_PVP_GRADE_H_
