#ifndef OLDSRV_DB_DB_SAVE_EXPAND_EQUIPSLOT_H_
#define OLDSRV_DB_DB_SAVE_EXPAND_EQUIPSLOT_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/inventory/expand_equipslot_payload.h"


// IDA_EA: 0x840DA96/0x843FEBE/0x843FF48/0x844F6DE/0x845FB38/0x845FB68.
// IDA unit: 04_character_select_create/DB_SaveExpandEquipslot.
class DB_SaveExpandEquipslot : public DBDispatcher {
public:
    DB_SaveExpandEquipslot();
    virtual ~DB_SaveExpandEquipslot();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    bool _SaveCharacStat(unsigned int charac_no,
                         SIG_SAVE_EXPAND_CHARAC_STAT* stat_payload);
    bool _SaveExpandEquipslot(SIG_SAVE_EXPAND_EQUIPSLOT* payload);
};

#endif  // OLDSRV_DB_DB_SAVE_EXPAND_EQUIPSLOT_H_
