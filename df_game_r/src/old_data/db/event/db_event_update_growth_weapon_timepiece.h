#ifndef OLDSRV_DB_DB_EVENT_UPDATE_GROWTH_WEAPON_TIMEPIECE_H_
#define OLDSRV_DB_DB_EVENT_UPDATE_GROWTH_WEAPON_TIMEPIECE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 08_quest_skill_growth/DB_EventUpdateGrowthWeaponTimepiece.
class DB_EventUpdateGrowthWeaponTimepiece : public DBDispatcher {
public:
    DB_EventUpdateGrowthWeaponTimepiece();
    virtual ~DB_EventUpdateGrowthWeaponTimepiece();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            short timepiece,
                            int infinity_weapon_id);
};

#endif  // OLDSRV_DB_DB_EVENT_UPDATE_GROWTH_WEAPON_TIMEPIECE_H_
