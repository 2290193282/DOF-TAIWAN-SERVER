#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_ITEM_DICTIONARY_FLAG_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_ITEM_DICTIONARY_FLAG_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_SaveItemDictionaryFlag.
class DB_SaveItemDictionaryFlag : public DBDispatcher {
public:
    // IDA_EA: 0x844FA7A
    // IDA_NAME: _ZN25DB_SaveItemDictionaryFlagC2Ev
    // DEMANGLED: DB_SaveItemDictionaryFlag::DB_SaveItemDictionaryFlag(void)
    DB_SaveItemDictionaryFlag();

    // IDA_EA: 0x845B4D6
    // IDA_NAME: _ZN25DB_SaveItemDictionaryFlagD2Ev
    // DEMANGLED: DB_SaveItemDictionaryFlag::~DB_SaveItemDictionaryFlag()
    // IDA_EA: 0x845B506
    // IDA_NAME: _ZN25DB_SaveItemDictionaryFlagD0Ev
    // DEMANGLED: DB_SaveItemDictionaryFlag::~DB_SaveItemDictionaryFlag()
    virtual ~DB_SaveItemDictionaryFlag();

    // IDA_EA: 0x844547E
    // IDA_NAME: _ZN25DB_SaveItemDictionaryFlag8dispatchEiiP6Stream
    // DEMANGLED: DB_SaveItemDictionaryFlag::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_ITEM_DICTIONARY_FLAG_H_
