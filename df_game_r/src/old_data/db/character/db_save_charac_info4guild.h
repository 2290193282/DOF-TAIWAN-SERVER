#ifndef OLDSRV_DB_DB_SAVE_CHARAC_INFO4GUILD_H_
#define OLDSRV_DB_DB_SAVE_CHARAC_INFO4GUILD_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 04_character_select_create/DB_SaveCharacInfo4Guild.
// IDA_EA: dispatch @0x841B624; ctor/dtor
// @0x844E152/@0x845F8C8/@0x845F8F8. DBTask active slot 99.
class DB_SaveCharacInfo4Guild : public DBDispatcher {
public:
    DB_SaveCharacInfo4Guild();
    virtual ~DB_SaveCharacInfo4Guild();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_CHARAC_INFO4GUILD_H_
