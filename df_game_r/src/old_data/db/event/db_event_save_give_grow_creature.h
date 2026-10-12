#ifndef OLDSRV_DB_EVENT_SAVE_GIVE_GROW_CREATURE_H_
#define OLDSRV_DB_EVENT_SAVE_GIVE_GROW_CREATURE_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_game/activity/schedule/character_event_records.h"

// IDA unit: 06_userinfo_stat_inventory/DB_EventSaveGiveGrowCreature.
class DB_EventSaveGiveGrowCreature : public DBDispatcher {
public:
    // IDA_EA: 0x844FDFA
    DB_EventSaveGiveGrowCreature();
    // IDA_EA: 0x845AD38 / 0x845AD68
    virtual ~DB_EventSaveGiveGrowCreature();

    // IDA_EA: 0x8449A58
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x8449B32
    // IDA_NAME: _ZN28DB_EventSaveGiveGrowCreature11makeRequestEjRN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoE
    static void makeRequest(
        unsigned int account_id,
        SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo& event_info);
};

#endif  // OLDSRV_DB_EVENT_SAVE_GIVE_GROW_CREATURE_H_
