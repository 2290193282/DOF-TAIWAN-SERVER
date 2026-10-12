#ifndef OLDSRV_DB_DB_SAVE_CHARAC_SLOT_EXTEND_H_
#define OLDSRV_DB_DB_SAVE_CHARAC_SLOT_EXTEND_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x84420EA/0x844F89E/0x845BAA0/0x845BAD0.
// IDA unit: 04_character_select_create/DB_SaveCharacSlotExtend.
class DB_SaveCharacSlotExtend : public DBDispatcher {
public:
    DB_SaveCharacSlotExtend();
    virtual ~DB_SaveCharacSlotExtend();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_CHARAC_SLOT_EXTEND_H_
