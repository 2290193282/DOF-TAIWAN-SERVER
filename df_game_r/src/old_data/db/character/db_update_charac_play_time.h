#ifndef OLDSRV_DB_DB_UPDATE_CHARAC_PLAY_TIME_H_
#define OLDSRV_DB_DB_UPDATE_CHARAC_PLAY_TIME_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 04_character_select_create/DB_UpdateCharacPlayTime.
// IDA_EA: dispatch @0x843F75A; makeRequest @0x843F7EE; ctor/dtor
// @0x844F6C2/@0x845BE48/@0x845BE78. DBTask active slot 590.
class DB_UpdateCharacPlayTime : public DBDispatcher {
public:
    DB_UpdateCharacPlayTime();
    virtual ~DB_UpdateCharacPlayTime();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int charac_no, long play_time);
};

#endif  // OLDSRV_DB_DB_UPDATE_CHARAC_PLAY_TIME_H_
