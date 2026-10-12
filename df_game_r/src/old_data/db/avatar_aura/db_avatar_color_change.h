#ifndef OLDSRV_DB_DB_AVATAR_COLOR_CHANGE_H_
#define OLDSRV_DB_DB_AVATAR_COLOR_CHANGE_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8446944/0x844FE4E/0x845B2B4/0x845B2E4.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarColorChange.
class DB_AvatarColorChange : public DBDispatcher {
public:
    DB_AvatarColorChange();
    virtual ~DB_AvatarColorChange();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_AVATAR_COLOR_CHANGE_H_
