#ifndef OLDSRV_DB_DB_RELOAD_AUTO_PUNISH_RULE_HACK_TYPE_H_
#define OLDSRV_DB_DB_RELOAD_AUTO_PUNISH_RULE_HACK_TYPE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 02_login_account/DB_ReloadAutoPunishRuleHackType.
// IDA_EA: dispatch @0x842D16C; makeRequest @0x842D610; ctor/dtor
// @0x844EA2E/@0x845E0B6/@0x845E0E6.
class DB_ReloadAutoPunishRuleHackType : public DBDispatcher {
public:
    DB_ReloadAutoPunishRuleHackType();
    virtual ~DB_ReloadAutoPunishRuleHackType();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest();
};

#endif  // OLDSRV_DB_DB_RELOAD_AUTO_PUNISH_RULE_HACK_TYPE_H_
