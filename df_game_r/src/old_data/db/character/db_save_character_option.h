#ifndef OLDSRV_DB_DB_SAVE_CHARACTER_OPTION_H_
#define OLDSRV_DB_DB_SAVE_CHARACTER_OPTION_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8430144/0x844EC42/0x845DC72/0x845DCA2.
// IDA unit: 04_character_select_create/DB_SaveCharacterOption.
class DB_SaveCharacterOption : public DBDispatcher {
public:
    DB_SaveCharacterOption();
    virtual ~DB_SaveCharacterOption();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_CHARACTER_OPTION_H_
