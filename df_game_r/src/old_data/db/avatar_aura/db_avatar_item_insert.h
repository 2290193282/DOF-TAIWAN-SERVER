#ifndef OLDSRV_DB_DB_AVATAR_ITEM_INSERT_H_
#define OLDSRV_DB_DB_AVATAR_ITEM_INSERT_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x83FFD50/0x83FFD6C/0x83FFD9C/0x83FFDBA/0x83FFE0E/0x84001FE.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarItemInsert.
class DB_AvatarItemInsert : public DBDispatcher {
public:
    DB_AvatarItemInsert();
    virtual ~DB_AvatarItemInsert();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static bool InsertAvatarItem(SIG_AVATAR_INSERT* payload);
    void _SendResult(SIG_AVATAR_INSERT* payload, int user_idx);
};

#endif  // OLDSRV_DB_DB_AVATAR_ITEM_INSERT_H_
