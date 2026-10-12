#ifndef OLDSRV_DB_EVENT_UPDATE_GIVE_GROW_CREATURE_H_
#define OLDSRV_DB_EVENT_UPDATE_GIVE_GROW_CREATURE_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_game/activity/schedule/character_event_records.h"

// IDA unit: 06_userinfo_stat_inventory/DB_EventUpdateGiveGrowCreature.
class DB_EventUpdateGiveGrowCreature : public DBDispatcher {
public:
    // IDA_EA: 0x844FE16
    DB_EventUpdateGiveGrowCreature();
    // IDA_EA: 0x845ACEA / 0x845AD1A
    virtual ~DB_EventUpdateGiveGrowCreature();

    // IDA_EA: 0x8449C54
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x8449D1C
    // IDA_NAME: _ZN30DB_EventUpdateGiveGrowCreature11makeRequestEjRN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoE
    static void makeRequest(
        unsigned int account_id,
        SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo& event_info);
};

#endif  // OLDSRV_DB_EVENT_UPDATE_GIVE_GROW_CREATURE_H_
