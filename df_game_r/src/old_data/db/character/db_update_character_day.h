#ifndef OLDSRV_DB_DB_UPDATE_CHARACTER_DAY_H_
#define OLDSRV_DB_DB_UPDATE_CHARACTER_DAY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 04_character_select_create/DB_UpdateCharacterDay.
// IDA_EA: dispatch @0x8435D12; ctor/dtor
// @0x844EF52/@0x845D4D4/@0x845D504. DBTask active slot 366.
class DB_UpdateCharacterDay : public DBDispatcher {
public:
    DB_UpdateCharacterDay();
    virtual ~DB_UpdateCharacterDay();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_CHARACTER_DAY_H_
