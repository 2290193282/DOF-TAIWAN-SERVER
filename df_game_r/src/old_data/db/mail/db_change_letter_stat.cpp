#include "old_data/db/mail/db_change_letter_stat.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>



namespace {

enum {
    kDbSignalChangeLetterStat = 194,
    kChangeLetterStatDbHandle = 3,
};

}  // namespace

// IDA_EA: 0x844E622
// IDA_NAME: _ZN19DB_ChangeLetterStatC2Ev
// DEMANGLED: DB_ChangeLetterStat::DB_ChangeLetterStat(void)
DB_ChangeLetterStat::DB_ChangeLetterStat() {}

// IDA_EA: 0x845EBAE
// IDA_NAME: _ZN19DB_ChangeLetterStatD2Ev
// DEMANGLED: DB_ChangeLetterStat::~DB_ChangeLetterStat()
// IDA_EA: 0x845EBDE
// IDA_NAME: _ZN19DB_ChangeLetterStatD0Ev
// DEMANGLED: DB_ChangeLetterStat::~DB_ChangeLetterStat() deleting destructor
DB_ChangeLetterStat::~DB_ChangeLetterStat() {}

// IDA_EA: 0x842602E
// IDA_NAME: _ZN19DB_ChangeLetterStat8dispatchEiiP6Stream
// DEMANGLED: DB_ChangeLetterStat::dispatch(int,int,Stream *)
bool DB_ChangeLetterStat::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;

    SIG_LETTER_STAT payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kChangeLetterStatDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    const int charac_no = payload.charac_no;
    const int letter_id = payload.letter_id;
    const short letter_stat = payload.letter_stat;

    if (letter_stat) {
        if (letter_stat >= 0 &&
            static_cast<unsigned int>(letter_stat - 2) <= 1) {
            db_handle->set_query(
                "upDate letter set stat=%d where letter_id=%d",
                static_cast<int>(letter_stat),
                letter_id);
            if (db_handle->exec(true) == true &&
                db_handle->getAffectedRowCount()) {
                makeRequest(signal, user_idx, charac_no, letter_id, letter_stat);
                return true;
            }
        }
        makeRequest(signal, user_idx, charac_no, 0, 0);
        return false;
    }

    db_handle->set_query("deLete from letter where letter_id = %d", letter_id);
    if (db_handle->exec(true) != true) {
        makeRequest(signal, user_idx, charac_no, 0, 0);
        return false;
    }

    makeRequest(signal, user_idx, charac_no, letter_id, letter_stat);
    return true;
}

// IDA_EA: 0x84261FC
// IDA_NAME: _ZN19DB_ChangeLetterStat11makeRequestEiiiii
// DEMANGLED: DB_ChangeLetterStat::makeRequest(int,int,int,int,int)
void DB_ChangeLetterStat::makeRequest(int signal,
                                      int user_idx,
                                      int charac_no,
                                      int letter_id,
                                      int letter_stat) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 25684);
    CStreamGuard guard(stream, true);
    *guard << signal;
    // The official 0627 image at 0x842625D..0x8426267 calls
    // CStreamGuard::operator<<(int).  Hex-Rays' raw export inferred the
    // dispatch parameter as char, but the response stream carries the full
    // four-byte composite user index consumed by DispatchThread.
    *guard << user_idx;
    SIG_LETTER_STAT* payload = guard.GetInBuffer<SIG_LETTER_STAT>();
    if (!payload) {
        return;
    }
    memset(payload, 0, sizeof(*payload));
    payload->charac_no = charac_no;
    payload->letter_id = letter_id;
    payload->letter_stat = static_cast<short>(letter_stat);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}
