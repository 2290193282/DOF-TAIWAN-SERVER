#include "old_data/db/character/db_save_charac_inform_notice.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/character/charac_inform_notice_payload.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream_charac_inform_notice.h"

#include <string.h>

// IDA_EA: 0x844FEA2
// IDA_NAME: _ZN25DB_SaveCharacInformNoticeC2Ev
// DEMANGLED: DB_SaveCharacInformNotice::DB_SaveCharacInformNotice(void)
DB_SaveCharacInformNotice::DB_SaveCharacInformNotice() {}

// IDA_EA: 0x845A9AC
// IDA_NAME: _ZN25DB_SaveCharacInformNoticeD2Ev
// DEMANGLED: DB_SaveCharacInformNotice::~DB_SaveCharacInformNotice()
// IDA_EA: 0x845A9DC
// IDA_NAME: _ZN25DB_SaveCharacInformNoticeD0Ev
// DEMANGLED: DB_SaveCharacInformNotice::~DB_SaveCharacInformNotice()
DB_SaveCharacInformNotice::~DB_SaveCharacInformNotice() {}

// IDA_EA: 0x844C170
// IDA_NAME: _ZN25DB_SaveCharacInformNotice8dispatchEiiP6Stream
// DEMANGLED: DB_SaveCharacInformNotice::dispatch(int,int,Stream *)
bool DB_SaveCharacInformNotice::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    SIG_SAVE_CHARAC_INFORM_NOTICE* payload =
        stream->GetOutBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>();
    if (!payload) {
        return false;
    }

    char escaped[257];
    memset(escaped, 0, sizeof(escaped));
    db_handle->escape_string(escaped, payload->flags);
    db_handle->set_query(
        "upDate charac_option set charac_inform_notice='%s' where charac_no=%u",
        escaped,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount() != 0) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_option(charac_no, options, best_clear_time, blue_marble_enter_count, charac_inform_notice) values (%u, '%s', '%s', 0, '%s')",
        payload->charac_no,
        "",
        "",
        escaped);
    if (db_handle->exec(true) != true) {
        return false;
    }
    return true;
}
