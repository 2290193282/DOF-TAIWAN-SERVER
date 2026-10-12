#ifndef OLDSRV_DB_DB_EVENT_UPDATE_GROWTH_WEAPON_CHARAC_H_
#define OLDSRV_DB_DB_EVENT_UPDATE_GROWTH_WEAPON_CHARAC_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 04_character_select_create/DB_EventUpdateGrowthWeaponCharac.
class DB_EventUpdateGrowthWeaponCharac : public DBDispatcher {
public:
    DB_EventUpdateGrowthWeaponCharac();
    virtual ~DB_EventUpdateGrowthWeaponCharac();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int charac_no,
                            int growth_weapon_id,
                            short server_id);
};

#endif  // OLDSRV_DB_DB_EVENT_UPDATE_GROWTH_WEAPON_CHARAC_H_
