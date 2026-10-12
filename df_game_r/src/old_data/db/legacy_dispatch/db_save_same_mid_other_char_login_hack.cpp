#include "old_data/db/anti_cheat/db_save_same_mid_other_char_login_hack.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E462
// IDA_NAME: _ZN32DB_SaveSameMIDOtherCharLoginHackC2Ev
// DEMANGLED: DB_SaveSameMIDOtherCharLoginHack::DB_SaveSameMIDOtherCharLoginHack(void)
DB_SaveSameMIDOtherCharLoginHack::DB_SaveSameMIDOtherCharLoginHack() {}

// IDA_EA: 0x845F0DC
// IDA_NAME: _ZN32DB_SaveSameMIDOtherCharLoginHackD2Ev
// DEMANGLED: DB_SaveSameMIDOtherCharLoginHack::~DB_SaveSameMIDOtherCharLoginHack()
// IDA_EA: 0x845F10C
// IDA_NAME: _ZN32DB_SaveSameMIDOtherCharLoginHackD0Ev
// DEMANGLED: DB_SaveSameMIDOtherCharLoginHack::~DB_SaveSameMIDOtherCharLoginHack() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveSameMIDOtherCharLoginHack::~DB_SaveSameMIDOtherCharLoginHack() {}

// IDA_EA: 0x8422680
// IDA_NAME: _ZN32DB_SaveSameMIDOtherCharLoginHack8dispatchEiiP6Stream
// DEMANGLED: DB_SaveSameMIDOtherCharLoginHack::dispatch(int,int,Stream *)
bool DB_SaveSameMIDOtherCharLoginHack::dispatch(int signal, int user_idx,
                                                Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK log;
    if (stream->get_binary(&log, sizeof(log)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    const char* first_mid = NumberToString(log.first_mid, 0);
    const char* second_mid = NumberToString(log.second_mid, 1);
    db_handle->set_query(
        "inSert into login_view_log values(now(),%s,%s,%d,'%s')",
        first_mid,
        second_mid,
        log.charac_no,
        log.peer_ip);
    return db_handle->exec(true) == true;
}
