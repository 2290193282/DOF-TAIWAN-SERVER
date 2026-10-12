#ifndef OLDSRV_DB_DB_AVATAR_ITEM_DEL_H_
#define OLDSRV_DB_DB_AVATAR_ITEM_DEL_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

#pragma pack(push, 1)
struct SIG_AVATAR_DELETE {
    uint32_t event_type;
    uint32_t charac_no;
    uint8_t slot;
    uint8_t opaque_tail[0x70Cu - 9u];
};
#pragma pack(pop)

static_assert(sizeof(SIG_AVATAR_DELETE) == 0x70Cu,
              "SIG_AVATAR_DELETE old ABI size must stay 0x70C");


// IDA_EA: 0x83FFC42/0x83FFC5E/0x83FFC8E/0x83FFCAC/0x83FFD46.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarItemDel.
class DB_AvatarItemDel : public DBDispatcher {
public:
    DB_AvatarItemDel();
    virtual ~DB_AvatarItemDel();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool DelAvatarItem(SIG_AVATAR_DELETE& payload);
};

#endif  // OLDSRV_DB_DB_AVATAR_ITEM_DEL_H_
