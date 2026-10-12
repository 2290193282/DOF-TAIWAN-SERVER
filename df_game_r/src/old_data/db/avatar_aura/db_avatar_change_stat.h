#ifndef OLDSRV_DB_DB_AVATAR_CHANGE_STAT_H_
#define OLDSRV_DB_DB_AVATAR_CHANGE_STAT_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x840065E/0x844E0AA/0x845FBD4/0x845FC04.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarChangeStat.
class DB_AvatarChangeStat : public DBDispatcher {
public:
    DB_AvatarChangeStat();
    virtual ~DB_AvatarChangeStat();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_AVATAR_CHANGE_STAT_H_
