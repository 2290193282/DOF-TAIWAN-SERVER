#include "old_data/db/legacy_dispatch/db_update_login.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"

#include <cstdio>


// IDA_EA: 0x844DA04
// IDA_NAME: _ZN14DB_UpdateLoginC2Ev
// DEMANGLED: DB_UpdateLogin::DB_UpdateLogin(void)
DB_UpdateLogin::DB_UpdateLogin() {}

// IDA_EA: 0x844DA20
// IDA_NAME: _ZN14DB_UpdateLoginD2Ev
// DEMANGLED: DB_UpdateLogin::~DB_UpdateLogin()
// IDA_EA: 0x844DA50
// IDA_NAME: _ZN14DB_UpdateLoginD0Ev
// DEMANGLED: DB_UpdateLogin::~DB_UpdateLogin() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateLogin::~DB_UpdateLogin() {}

// IDA_EA: 0x841826A
// IDA_NAME: _ZN14DB_UpdateLogin8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateLogin::dispatch(int,int,Stream *)
bool DB_UpdateLogin::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_LOGIN login;
    if (stream->get_binary(&login, sizeof(login)) != 1) {
        return false;
    }

    int login_account_table = 0;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        login_account_table = login.server_group;
    } else {
        login_account_table = G_CEnvironment()->get_server_group();
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6), static_cast<ENUM_SERVER_GROUP>(0));
    char account_id[32];
    std::snprintf(account_id, sizeof(account_id), "%u", login.m_id);
    db_handle->set_query(
        "upDate login_account_%d set m_channel_no=%d,login_status=1,last_login_date=now(),login_ip='%s' where m_id=%s",
        login_account_table,
        login.channel_no,
        login.login_ip,
        account_id);

    bool should_insert = true;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount() != 0) {
            should_insert = false;
        }
    }

    if (should_insert) {
        std::snprintf(account_id, sizeof(account_id), "%u", login.m_id);
        db_handle->set_query(
            "inSert into login_account_%d (m_id,m_channel_no,login_status,last_login_date,login_ip) values(%s,%d,1,now(),'%s')",
            login_account_table,
            account_id,
            login.channel_no,
            login.login_ip);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
