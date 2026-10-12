#ifndef OLDSRV_DB_DB_EVENT_GROWTH_WEAPON_RESET_H_
#define OLDSRV_DB_DB_EVENT_GROWTH_WEAPON_RESET_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 08_quest_skill_growth/DB_EventGrowthWeaponReset.
class DB_EventGrowthWeaponReset : public DBDispatcher {
public:
    DB_EventGrowthWeaponReset();
    virtual ~DB_EventGrowthWeaponReset();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id);
};

#endif  // OLDSRV_DB_DB_EVENT_GROWTH_WEAPON_RESET_H_
