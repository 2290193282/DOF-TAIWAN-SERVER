#ifndef OLDSRV_DB_DB_AVATAR_OPTION_CHANGE_H_
#define OLDSRV_DB_DB_AVATAR_OPTION_CHANGE_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844687E/0x844FB22/0x845B302/0x845B332.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarOptionChange.
class DB_AvatarOptionChange : public DBDispatcher {
public:
    DB_AvatarOptionChange();
    virtual ~DB_AvatarOptionChange();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_AVATAR_OPTION_CHANGE_H_
