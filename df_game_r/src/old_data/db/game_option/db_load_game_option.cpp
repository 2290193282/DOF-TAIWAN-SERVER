#include "old_data/db/game_option/db_load_game_option.h"

#include "common/config.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>

extern int uncompress_zip(char* dest, unsigned int& dest_len,
                          const char* source, unsigned int source_len);


extern const char* get_member_game_option()
    __asm__("_Z22get_member_game_optionv");
extern unsigned char get_compressed_blob_data(MySQL* db_handle,
                                             int column,
                                             char* dest,
                                             int dest_len,
                                             int unit_size)
    __asm__("_Z24get_compressed_blob_dataP5MySQLiPcii");

typedef char SIG_REQ_LOAD_GAME_OPTION_Size_Must_Be_4[
    (sizeof(SIG_REQ_LOAD_GAME_OPTION) == 4) ? 1 : -1];
typedef char SIG_LOAD_GAME_OPTION_Size_Must_Follow_Config[
    (sizeof(SIG_LOAD_GAME_OPTION) ==
     oldsrv::config::kGameEtcOptionBytes + 600 + 528) ? 1 : -1];

namespace {

enum {
    // IDA_EA: 0x842F8E0 writes 0x12f (303) to the dispatch stream.
    kInterLoadGameOptionSignal = 303,
    kLoadGameOptionLineEmpty = 29890,
    kLoadGameOptionLineFetch = 29917,
    kLoadGameOptionLineRow = 29921,
    kLoadGameOptionLineOption1 = 29945,
    kLoadGameOptionLineOption3 = 29973,
};

}  // namespace

// IDA_EA: 0x844EBEE
// IDA_NAME: _ZN17DB_LoadGameOptionC2Ev
// DEMANGLED: DB_LoadGameOption::DB_LoadGameOption(void)
DB_LoadGameOption::DB_LoadGameOption() {}

// IDA_EA: 0x845DD5C
// IDA_NAME: _ZN17DB_LoadGameOptionD2Ev
// DEMANGLED: DB_LoadGameOption::~DB_LoadGameOption()
// IDA_EA: 0x845DD8C
// IDA_NAME: _ZN17DB_LoadGameOptionD0Ev
// DEMANGLED: DB_LoadGameOption::~DB_LoadGameOption() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadGameOption::~DB_LoadGameOption() {}

// IDA_EA: 0x842F7F4
// IDA_NAME: _ZN17DB_LoadGameOption8dispatchEiiP6Stream
// DEMANGLED: DB_LoadGameOption::dispatch(int,int,Stream *)
bool DB_LoadGameOption::dispatch(int signal, int user_idx, Stream* stream) {
    LogManager::logFormat(
        1, "login_flow_diag.cpp",
        "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
        "[LOGIN_FLOW][GAME_OPTION_DB_ENTER] signal=%d uid=%d", signal,
        user_idx);

    const int bytes = sizeof(SIG_REQ_LOAD_GAME_OPTION);
    if (!stream->enable_out(bytes)) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=enable_out uid=%d",
            user_idx);
        return false;
    }
    SIG_REQ_LOAD_GAME_OPTION* request =
        reinterpret_cast<SIG_REQ_LOAD_GAME_OPTION*>(stream->get_buffer());
    stream->out_ptr(bytes);

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(6),
        static_cast<ENUM_SERVER_GROUP>(0));
    const char* account_id = NumberToString(request->m_id, 0);
    const char* table_name = get_member_game_option();
    db_handle->set_query(
        "seLect option_1,option_3,shortcut_emoticon from %s where m_id = %s",
        table_name,
        account_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=exec uid=%d account=%u",
            user_idx, request->m_id);
        return false;
    }

    const int row_count = static_cast<int>(db_handle->get_n_rows());
    LogManager::logFormat(
        1, "login_flow_diag.cpp",
        "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
        "[LOGIN_FLOW][GAME_OPTION_DB_ROWS] uid=%d account=%u rows=%d",
        user_idx, request->m_id, row_count);
    if (!row_count) {
        Stream* out_stream = GlobalData::stream_pool()->Acquire(
            "DBThread.cpp",
            kLoadGameOptionLineEmpty);
        CStreamGuard guard(out_stream, true);
        *guard << kInterLoadGameOptionSignal;
        *guard << user_idx;
        SIG_LOAD_GAME_OPTION* const payload =
            guard->GetInBuffer<SIG_LOAD_GAME_OPTION>();
        if (!payload) {
            return false;
        }
        for (unsigned int index = 0;
             index < oldsrv::config::kGameEtcOptionCount;
             ++index) {
            payload->etc_option.values[index] =
                oldsrv::config::kGameEtcOptionDefaultValue;
        }
        payload->quick_chat[0].clear();
        payload->quick_chat[1].clear();
        payload->emoticon_config.clear();
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_ENQUEUE] uid=%d signal=%d rows=0",
            user_idx, kInterLoadGameOptionSignal);
        return true;
    }

    if (db_handle->fetch() != true) {
        account_id = NumberToString(request->m_id, 0);
        cMyTrace trace("virtual bool DB_LoadGameOption::dispatch(int, int, Stream*)",
                       kLoadGameOptionLineFetch,
                       5);
        trace("DB_LoadGameOption::dispatch, fetch ERROR m_id=%s",
              account_id);
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=fetch uid=%d account=%u",
            user_idx, request->m_id);
        return false;
    }

    Stream* out_stream = GlobalData::stream_pool()->Acquire(
        "DBThread.cpp",
        kLoadGameOptionLineRow);
    CStreamGuard guard(out_stream, true);
    *guard << kInterLoadGameOptionSignal;
    *guard << user_idx;
    SIG_LOAD_GAME_OPTION* const payload =
        guard->GetInBuffer<SIG_LOAD_GAME_OPTION>();
    if (!payload) {
        return false;
    }
    memset(&payload->etc_option, 0, sizeof(payload->etc_option));
    payload->quick_chat[0].clear();
    payload->quick_chat[1].clear();
    payload->emoticon_config.clear();

    char compressed[732];
    memset(compressed,
           0,
           oldsrv::config::kMaximumCompressedGameEtcOptionBytes);
    if (db_handle->get_binary(
            0,
            compressed,
            oldsrv::config::kMaximumCompressedGameEtcOptionBytes) != true) {
        account_id = NumberToString(request->m_id, 0);
        cMyTrace trace("virtual bool DB_LoadGameOption::dispatch(int, int, Stream*)",
                       kLoadGameOptionLineOption1,
                       5);
        trace("DB_LoadGameOption, get_binary(0) ERROR m_id=%s", account_id);
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=option_1 uid=%d account=%u",
            user_idx, request->m_id);
        return false;
    }

    unsigned int raw_len = sizeof(payload->etc_option);
    unsigned int compressed_len =
        static_cast<unsigned int>(db_handle->get_binary_length(0));
    if (uncompress_zip(reinterpret_cast<char*>(&payload->etc_option),
                       raw_len,
                       compressed,
                       compressed_len) != 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=option_1_uncompress uid=%d source_len=%u",
            user_idx, compressed_len);
        return false;
    }
    if (raw_len == 0 || raw_len > sizeof(payload->etc_option) ||
        raw_len % sizeof(payload->etc_option.values[0]) != 0) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=option_1_size uid=%d raw_len=%u",
            user_idx, raw_len);
        return false;
    }
    if (raw_len < sizeof(payload->etc_option)) {
        const unsigned int first_new_index =
            raw_len / sizeof(payload->etc_option.values[0]);
        for (unsigned int index = first_new_index;
             index < oldsrv::config::kGameEtcOptionCount;
             ++index) {
            payload->etc_option.values[index] =
                oldsrv::config::kGameEtcOptionDefaultValue;
        }
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_MIGRATE] uid=%d old_bytes=%u new_bytes=%u",
            user_idx, raw_len,
            static_cast<unsigned int>(sizeof(payload->etc_option)));
    }

    memset(compressed, 0, sizeof(compressed));
    if (db_handle->get_binary(1, compressed, sizeof(compressed)) != true) {
        cMyTrace trace("virtual bool DB_LoadGameOption::dispatch(int, int, Stream*)",
                       kLoadGameOptionLineOption3,
                       5);
        trace("DB_LoadGameOption, get_binary(1) ERROR m_id=%u",
              request->m_id);
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=option_3 uid=%d account=%u",
            user_idx, request->m_id);
        return false;
    }

    raw_len = 600;
    compressed_len =
        static_cast<unsigned int>(db_handle->get_binary_length(1));
    if (uncompress_zip(reinterpret_cast<char*>(&payload->quick_chat[0]),
                       raw_len,
                       compressed,
                       compressed_len) != 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=option_3_uncompress uid=%d source_len=%u",
            user_idx, compressed_len);
        return false;
    }
    if (get_compressed_blob_data(db_handle,
                                 2,
                                 reinterpret_cast<char*>(&payload->emoticon_config),
                                 528,
                                 22) != 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
            "[LOGIN_FLOW][GAME_OPTION_DB_FAIL] stage=emoticon uid=%d",
            user_idx);
        return false;
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    LogManager::logFormat(
        1, "login_flow_diag.cpp",
        "bool DB_LoadGameOption::dispatch(int, int, Stream*)", 0,
        "[LOGIN_FLOW][GAME_OPTION_DB_ENQUEUE] uid=%d signal=%d rows=%d",
        user_idx, kInterLoadGameOptionSignal, row_count);
    return true;
}
