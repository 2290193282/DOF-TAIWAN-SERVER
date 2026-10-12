#include "old_data/db/anti_cheat/db_hack_ip_save.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



namespace {

enum {
    kDbSignalHackIpSave = 400,
    kDbHandleSecurity = 6,
};

#pragma pack(push, 1)
struct SIG_HACK_IP_SAVE_LOCAL {
    char c_class_ip[13];       // +0x00
    char full_ip[16];          // +0x0d
    unsigned char reserved_29[3];
    uint32_t hack_type;        // +0x20
    uint32_t hack_sub_type;    // +0x24
};
#pragma pack(pop)

typedef char SIG_HACK_IP_SAVE_LOCAL_Size_Must_Be_40[
    (sizeof(SIG_HACK_IP_SAVE_LOCAL) == 40) ? 1 : -1];

static void trace_hack_ip_error(int line, const char* message) {
    cMyTrace trace("virtual bool DB_HackIPSave::dispatch(int, int, Stream*)",
                   line,
                   1);
    trace("%s", message);
}

}  // namespace

// IDA_EA: 0x844F032
// IDA_NAME: _ZN13DB_HackIPSaveC2Ev
// DEMANGLED: DB_HackIPSave::DB_HackIPSave(void)
DB_HackIPSave::DB_HackIPSave() {}

// IDA_EA: 0x845D216
// IDA_NAME: _ZN13DB_HackIPSaveD2Ev
// DEMANGLED: DB_HackIPSave::~DB_HackIPSave()
// IDA_EA: 0x845D246
// IDA_NAME: _ZN13DB_HackIPSaveD0Ev
// DEMANGLED: DB_HackIPSave::~DB_HackIPSave() deleting destructor
DB_HackIPSave::~DB_HackIPSave() {}

// IDA_EA: 0x8436DEC
// IDA_NAME: _ZN13DB_HackIPSave8dispatchEiiP6Stream
// DEMANGLED: DB_HackIPSave::dispatch(int,int,Stream *)
bool DB_HackIPSave::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_HACK_IP_SAVE_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kDbHandleSecurity),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate auto_punish_hack_ip set cnt = cnt+1 where occ_date = CAST(DATE_FORMAT(now(), '%%Y-%%m-%%d') AS CHAR) and hack_type = %d and hack_sub_type = %d and c_class_ip = '%s'",
        payload.hack_type,
        payload.hack_sub_type,
        payload.c_class_ip);

    bool need_insert_class_ip = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert_class_ip = false;
    }
    if (need_insert_class_ip) {
        db_handle->set_query(
            "inSert into auto_punish_hack_ip (occ_date, hack_type, hack_sub_type, c_class_ip, cnt) values (now(), %d, %d, '%s', 1)",
            payload.hack_type,
            payload.hack_sub_type,
            payload.c_class_ip);
        if (db_handle->exec(true) != true) {
            trace_hack_ip_error(33437, "DB_HackIPSave DB ERROR");
            return false;
        }
    }

    db_handle->set_query(
        "upDate auto_punish_hack_full_ip set cnt = cnt+1 where occ_date = CAST(DATE_FORMAT(now(), '%%Y-%%m-%%d') AS CHAR) and hack_type = %d and hack_sub_type = %d and full_ip = '%s'",
        payload.hack_type,
        payload.hack_sub_type,
        payload.full_ip);

    bool need_insert_full_ip = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert_full_ip = false;
    }
    if (!need_insert_full_ip) {
        return true;
    }

    db_handle->set_query(
        "inSert into auto_punish_hack_full_ip (occ_date, hack_type, hack_sub_type, full_ip, cnt) values (now(), %d, %d, '%s', 1)",
        payload.hack_type,
        payload.hack_sub_type,
        payload.full_ip);
    if (db_handle->exec(true) == true) {
        return true;
    }

    trace_hack_ip_error(33455, "DB_HackIPSave FullIP DB ERROR");
    return false;
}

// IDA_EA: 0x8436FF8
// IDA_NAME: _ZN13DB_HackIPSave11makeRequestEPKcS1_jj
// DEMANGLED: DB_HackIPSave::makeRequest(char const*,char const*,unsigned int,unsigned int)
void DB_HackIPSave::makeRequest(const char* c_class_ip,
                                const char* full_ip,
                                unsigned int hack_type,
                                unsigned int hack_sub_type) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 33470);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalHackIpSave;
    *guard << -1;

    if (!c_class_ip || strlen(c_class_ip) > 7u) {
        SIG_HACK_IP_SAVE_LOCAL* payload =
            static_cast<SIG_HACK_IP_SAVE_LOCAL*>(
                guard->GetInBufferUntyped(sizeof(SIG_HACK_IP_SAVE_LOCAL)));
        memset(payload, 0, sizeof(*payload));
        if (c_class_ip) {
            strncpy(payload->c_class_ip, c_class_ip, 0x0Du);
        }
        payload->hack_type = hack_type;
        payload->hack_sub_type = hack_sub_type;
        if (full_ip) {
            strncpy(payload->full_ip, full_ip, 0x10u);
        }
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
}
