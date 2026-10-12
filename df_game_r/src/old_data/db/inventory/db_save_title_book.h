#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_TITLE_BOOK_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_TITLE_BOOK_H_

#include "old_data/db/core/db_dispatcher.h"

class DB_SaveTitleBook : public DBDispatcher {
public:
    // IDA_EA: 0x844F82E
    // IDA_NAME: _ZN16DB_SaveTitleBookC2Ev
    // DEMANGLED: DB_SaveTitleBook::DB_SaveTitleBook(void)
    DB_SaveTitleBook();

    // IDA_EA: 0x845FAEA
    // IDA_NAME: _ZN16DB_SaveTitleBookD2Ev
    // DEMANGLED: DB_SaveTitleBook::~DB_SaveTitleBook()
    // IDA_EA: 0x845FB1A
    // IDA_NAME: _ZN16DB_SaveTitleBookD0Ev
    // DEMANGLED: DB_SaveTitleBook::~DB_SaveTitleBook()
    virtual ~DB_SaveTitleBook();

    // IDA_EA: 0x840DB00
    // IDA_NAME: _ZN16DB_SaveTitleBook8dispatchEiiP6Stream
    // DEMANGLED: DB_SaveTitleBook::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_TITLE_BOOK_H_
