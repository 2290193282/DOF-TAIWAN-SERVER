#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_2_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_2_H_

#include "old_data/db/character/db_load_charac_hotkey_option.h"

// IDA unit: 13_database_dispatch/DB_SaveGameOption_2.
class DB_SaveGameOption_2 : public DBDispatcher {
public:
    DB_SaveGameOption_2();
    virtual ~DB_SaveGameOption_2();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_SAVE_GAME_OPTION_2_H_
