#ifndef OLDSRV_DB_DB_AVATAR_CHANGE_OWNER_H_
#define OLDSRV_DB_DB_AVATAR_CHANGE_OWNER_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x84004BC/0x840055C/0x844D7F2/0x844D80E/0x844D83E.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarChangeOwner.
class DB_AvatarChangeOwner : public DBDispatcher {
public:
    DB_AvatarChangeOwner();
    virtual ~DB_AvatarChangeOwner();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int ui_id,
                            unsigned int cur_charac_no,
                            unsigned int new_charac_no,
                            unsigned char slot);
};

#endif  // OLDSRV_DB_DB_AVATAR_CHANGE_OWNER_H_
