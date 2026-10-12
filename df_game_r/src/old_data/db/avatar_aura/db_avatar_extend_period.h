#ifndef OLDSRV_DB_DB_AVATAR_EXTEND_PERIOD_H_
#define OLDSRV_DB_DB_AVATAR_EXTEND_PERIOD_H_

#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x83FFAB8/0x83FFAD4/0x83FFB04/0x83FFB22/0x83FFBB4.
// IDA unit: 06_userinfo_stat_inventory/DB_AvatarExtendPeriod.
class DB_AvatarExtendPeriod : public DBDispatcher {
public:
    DB_AvatarExtendPeriod();
    virtual ~DB_AvatarExtendPeriod();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static bool ExtendPeriod(SIG_AVATAR_RENEW& payload);
};

#endif  // OLDSRV_DB_DB_AVATAR_EXTEND_PERIOD_H_
