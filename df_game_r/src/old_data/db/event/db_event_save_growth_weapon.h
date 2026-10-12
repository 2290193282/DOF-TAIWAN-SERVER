#ifndef OLDSRV_DB_DB_EVENT_SAVE_GROWTH_WEAPON_H_
#define OLDSRV_DB_DB_EVENT_SAVE_GROWTH_WEAPON_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 08_quest_skill_growth/DB_EventSaveGrowthWeapon.
class DB_EventSaveGrowthWeapon : public DBDispatcher {
public:
    DB_EventSaveGrowthWeapon();
    virtual ~DB_EventSaveGrowthWeapon();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, short server_id);
};

#endif  // OLDSRV_DB_DB_EVENT_SAVE_GROWTH_WEAPON_H_
