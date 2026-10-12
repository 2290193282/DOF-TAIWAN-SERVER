#ifndef OLDSRV_DB_DB_AVATAR_ITEM_MOVE_H_
#define OLDSRV_DB_DB_AVATAR_ITEM_MOVE_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x840031E/0x840033A/0x840036A/0x8400388/0x84003CA/0x8400432.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarItemMove.
class DB_AvatarItemMove : public DBDispatcher {
public:
    DB_AvatarItemMove();
    virtual ~DB_AvatarItemMove();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool MoveAvatarItem(SIG_AVATAR_MOVE* payload);
    bool ChangeSlot(int ui_id, int slot, int clear_avatar_id);
};

#endif  // OLDSRV_DB_DB_AVATAR_ITEM_MOVE_H_
