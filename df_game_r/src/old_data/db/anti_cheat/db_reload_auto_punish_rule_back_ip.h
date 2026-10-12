#ifndef OLDSRV_DB_DB_RELOAD_AUTO_PUNISH_RULE_BACK_IP_H_
#define OLDSRV_DB_DB_RELOAD_AUTO_PUNISH_RULE_BACK_IP_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 02_login_account/DB_ReloadAutoPunishRuleBackIP.
// IDA_EA: dispatch @0x842CDF6; makeRequest @0x842D0AE; ctor/dtor
// @0x844EA12/@0x845E104/@0x845E134.
class DB_ReloadAutoPunishRuleBackIP : public DBDispatcher {
public:
    DB_ReloadAutoPunishRuleBackIP();
    virtual ~DB_ReloadAutoPunishRuleBackIP();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest();
};

#endif  // OLDSRV_DB_DB_RELOAD_AUTO_PUNISH_RULE_BACK_IP_H_
