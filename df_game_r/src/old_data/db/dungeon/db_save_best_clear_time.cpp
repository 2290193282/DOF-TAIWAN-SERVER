#include "old_data/db/dungeon/db_save_best_clear_time.h"

#include "old_data/db/dungeon/best_clear_time_payload.h"
#include "common/types/compressed_blob.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

namespace {

enum {
    kBestClearTimeDataSize = 24000,
};

}  // namespace

// IDA_EA: 0x844FCFE
// IDA_NAME: _ZN20DB_SaveBestClearTimeC2Ev
// DEMANGLED: DB_SaveBestClearTime::DB_SaveBestClearTime(void)
DB_SaveBestClearTime::DB_SaveBestClearTime() {}

// IDA_EA: 0x845B044
// IDA_NAME: _ZN20DB_SaveBestClearTimeD2Ev
// DEMANGLED: DB_SaveBestClearTime::~DB_SaveBestClearTime()
// IDA_EA: 0x845B074
// IDA_NAME: _ZN20DB_SaveBestClearTimeD0Ev
// DEMANGLED: DB_SaveBestClearTime::~DB_SaveBestClearTime()
DB_SaveBestClearTime::~DB_SaveBestClearTime() {}

// IDA_EA: 0x8447CC4
// IDA_NAME: _ZN20DB_SaveBestClearTime8dispatchEiiP6Stream
// DEMANGLED: DB_SaveBestClearTime::dispatch(int,int,Stream *)
bool DB_SaveBestClearTime::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_BEST_CLEAR_TIME* payload =
        stream->GetOutBuffer<SIG_BEST_CLEAR_TIME>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* compressed_blob = 0;
    if (put_compressed_blob_data(
            db_handle,
            0,
            reinterpret_cast<char*>(payload->records),
            kBestClearTimeDataSize,
            &compressed_blob) != 1) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_SaveBestClearTime::dispatch(int, int, Stream*)",
            45709,
            "DB_SaveBestClearTime::dispatch compress_zip(%u) ERROR charac_no=%u",
            0,
            payload->charac_no);
        return false;
    }

    db_handle->set_query(
        "upDate charac_option set best_clear_time = '%s' where charac_no=%u",
        compressed_blob,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_SaveBestClearTime::dispatch(int, int, Stream*)",
            45721,
            "DB_SaveBestClearTime->upDate Query ERROR charac_no=%u",
            payload->charac_no);
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_option(charac_no, best_clear_time) values(%u, '%s')",
        payload->charac_no,
        compressed_blob);
    if (db_handle->exec(true) == true) {
        return true;
    }

    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "virtual bool DB_SaveBestClearTime::dispatch(int, int, Stream*)",
        45734,
        "DB_SaveBestClearTime->inSert Query ERROR charac_no=%u",
        payload->charac_no);
    return false;
}
