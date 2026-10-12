#ifndef OLDSRV_DB_DB_LOAD_CHARAC_H_
#define OLDSRV_DB_DB_LOAD_CHARAC_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/character/load_charac_payload.h"


class MySQL;

// IDA_EA: 0x8408A3C/0x8408C0A/0x8408C30/0x8408CF4/0x84093F6/0x8409D2E/0x8409FA6/0x840A4CE/0x844D8C6/0x844D8E2/0x844D912.
// IDA unit: 13_database_dispatch/DB_LoadCharac.
class DB_LoadCharac : public DBDispatcher {
public:
    DB_LoadCharac();
    virtual ~DB_LoadCharac();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    MySQL* _getGameDB();
    bool _UpdateSchoolCharac(SIG_LOAD_CHARAC* payload);
    bool _getCharacInfo(SIG_LOAD_CHARAC* payload);
    bool _getCharacStat(SIG_LOAD_CHARAC* payload);
    bool _LoadEventBringUpInfo(SIG_LOAD_CHARAC* payload);
    static bool _GetTagCharacInfo(unsigned int charac_no,
                                  TAG_CHARAC_DATA* tag_payload);
    void _sendResult(int signal, int user_idx, SIG_LOAD_CHARAC* payload);
};

#endif  // OLDSRV_DB_DB_LOAD_CHARAC_H_
