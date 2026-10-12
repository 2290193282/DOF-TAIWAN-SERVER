#ifndef OLDSRV_DB_DB_UPDATE_AURA_AVATAR_OPTION_H_
#define OLDSRV_DB_DB_UPDATE_AURA_AVATAR_OPTION_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8444E1E/0x8444F26/0x844FA26/0x845B5C0/0x845B5F0.
// IDA unit: 06_userinfo_stat_inventory/DB_UpdateAuraAvatarOption.
class DB_UpdateAuraAvatarOption : public DBDispatcher {
public:
    DB_UpdateAuraAvatarOption();
    virtual ~DB_UpdateAuraAvatarOption();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int charac_no,
                            int option_type,
                            int value);
};

#endif  // OLDSRV_DB_DB_UPDATE_AURA_AVATAR_OPTION_H_
