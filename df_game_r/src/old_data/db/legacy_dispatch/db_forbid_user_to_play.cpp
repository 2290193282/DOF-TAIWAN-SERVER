#include "old_data/db/anti_cheat/db_forbid_user_to_play.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E3D6
// IDA_NAME: _ZN19DB_ForbidUserToPlayC2Ev
// DEMANGLED: DB_ForbidUserToPlay::DB_ForbidUserToPlay(void)
DB_ForbidUserToPlay::DB_ForbidUserToPlay() {}

// IDA_EA: 0x845F2B0
// IDA_NAME: _ZN19DB_ForbidUserToPlayD2Ev
// DEMANGLED: DB_ForbidUserToPlay::~DB_ForbidUserToPlay()
// IDA_EA: 0x845F2E0
// IDA_NAME: _ZN19DB_ForbidUserToPlayD0Ev
// DEMANGLED: DB_ForbidUserToPlay::~DB_ForbidUserToPlay() deleting destructor
DB_ForbidUserToPlay::~DB_ForbidUserToPlay() {}

// IDA_EA: 0x8421AF4
// IDA_NAME: _ZN19DB_ForbidUserToPlay8dispatchEiiP6Stream
// DEMANGLED: DB_ForbidUserToPlay::dispatch(int,int,Stream *)
bool DB_ForbidUserToPlay::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_FORBID_USER_TO_PLAY* const payload =
        stream->GetOutBuffer<SIG_FORBID_USER_TO_PLAY>();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (payload->forbid == 1 && payload->duration_days) {
        db_handle->set_query(
            "upDate charac_stat set forbidden_to_play=1, forbidden_due_to = date_add(now(), interval %d day) where charac_no = %d",
            payload->duration_days,
            payload->charac_no);
    } else {
        db_handle->set_query(
            "upDate charac_stat set forbidden_to_play=0, forbidden_due_to= 0 where charac_no = %d",
            payload->charac_no);
    }
    return db_handle->exec(true) == true;
}
