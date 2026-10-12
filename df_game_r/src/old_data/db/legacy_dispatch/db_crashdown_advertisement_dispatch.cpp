#include "old_data/db/legacy_dispatch/db_crashdown_advertisement_dispatch.h"

#include <ctime>
#include <map>

#include "common/manager/crashdown_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/in_game_advertisement_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"
#include "common/manager/system_time.h"


// IDA_EA: 0x844E0C6
// IDA_NAME: _ZN18DB_CrashDownReportC2Ev
// DEMANGLED: DB_CrashDownReport::DB_CrashDownReport(void)
DB_CrashDownReport::DB_CrashDownReport() : DBDispatcher() {
}

// IDA_EA: 0x8450022
// IDA_NAME: _ZN18DB_CrashDownReportD2Ev
// DEMANGLED: DB_CrashDownReport::~DB_CrashDownReport()
// IDA_EA: 0x8450052
// IDA_NAME: _ZN18DB_CrashDownReportD0Ev
// DEMANGLED: DB_CrashDownReport::~DB_CrashDownReport()
DB_CrashDownReport::~DB_CrashDownReport() {
}

// IDA_EA: 0x841ABC6
// IDA_NAME: _ZN18DB_CrashDownReport8dispatchEiiP6Stream
// DEMANGLED: DB_CrashDownReport::dispatch(int,int,Stream *)
bool DB_CrashDownReport::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    time_t timer = static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm local_time;
    std::tm* minute_time = localtime_r(&timer, &local_time);
    minute_time->tm_sec = 0;
    const time_t occ_time = mktime(minute_time);

    StatisticsToObserveCrash* statistics =
        reinterpret_cast<StatisticsToObserveCrash*>(
            G_GameWorld()->getStatisticsToObserveCrash());
    const int abnormal_count = statistics->getAbnormalDownUserCount();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    db_handle->set_query(
        "upDate client_down set occ_count = occ_count+%d where occ_date = cast(now() as date)",
        abnormal_count);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("virtual bool DB_CrashDownReport::dispatch(int, int, Stream*)",
                       19804, 5);
        trace("upDate client_down failed");
    }
    cMyTrace abnormal_trace(
        "virtual bool DB_CrashDownReport::dispatch(int, int, Stream*)", 19807,
        0);
    abnormal_trace("abnormal down user : %d", abnormal_count);

    statistics->lock();

    db_handle = GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_LOG,
                                                   SERVER_GROUP_DEFAULT);
    for (std::map<int, int>::const_iterator entry =
             statistics->dungeon_counts_.begin();
         entry != statistics->dungeon_counts_.end(); ++entry) {
        db_handle->set_query(
            "inSert into crash_down_by_dungeon(occ_time,dungeon_index,down_count) values(from_unixtime(%d),%d,%d)",
            static_cast<int>(occ_time), entry->first, entry->second);
        if (db_handle->exec(true) == true ||
            (db_handle->set_query(
                 "upDate crash_down_by_dungeon set down_count = down_count + %d where occ_time = from_unixtime(%d) and dungeon_index = %d",
                 entry->second, static_cast<int>(occ_time), entry->first),
             db_handle->exec(true) == true)) {
            cMyTrace trace(
                "virtual bool DB_CrashDownReport::dispatch(int, int, Stream*)",
                19837, 0);
            trace("abnormal down user by dungeon : %d DUNGEON %d",
                  entry->second, entry->first);
        }
    }

    db_handle = GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_LOG,
                                                   SERVER_GROUP_DEFAULT);
    for (std::map<int, int>::const_iterator entry =
             statistics->packet_counts_.begin();
         entry != statistics->packet_counts_.end(); ++entry) {
        db_handle->set_query(
            "inSert into crash_down_by_packet(occ_date,packet_index,down_count) values(cast(now() as date),%d,%d)",
            entry->first, entry->second);
        if (db_handle->exec(true) == true ||
            (db_handle->set_query(
                 "upDate crash_down_by_packet set down_count = down_count + %d where occ_date = cast(now() as date) and packet_index = %d",
                 entry->second, entry->first),
             db_handle->exec(true) == true)) {
            cMyTrace trace(
                "virtual bool DB_CrashDownReport::dispatch(int, int, Stream*)",
                19864, 0);
            trace("abnormal down user by packet : %d PACKET %d",
                  entry->second, entry->first);
        }
    }

    db_handle = GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_LOG,
                                                   SERVER_GROUP_DEFAULT);
    for (std::map<int, int>::const_iterator entry =
             statistics->play_status_counts_.begin();
         entry != statistics->play_status_counts_.end(); ++entry) {
        db_handle->set_query(
            "inSert into crash_down_by_playstatus(occ_date,play_status,down_count) values(cast(now() as date),%d,%d)",
            entry->first, entry->second);
        if (db_handle->exec(true) == true ||
            (db_handle->set_query(
                 "upDate crash_down_by_playstatus set down_count = down_count + %d where occ_date = cast(now() as date) and play_status = %d",
                 entry->second, entry->first),
             db_handle->exec(true) == true)) {
            cMyTrace trace(
                "virtual bool DB_CrashDownReport::dispatch(int, int, Stream*)",
                19890, 0);
            trace("abnormal down user by playerstatus : %d PLAYERSTATUS %d",
                  entry->second, entry->first);
        }
    }

    statistics->unlock();
    statistics->reset();
    return true;
}


// IDA_EA: 0x844E6CA
// IDA_NAME: _ZN27DB_UpdateClientCrashDownLogC2Ev
// DEMANGLED: DB_UpdateClientCrashDownLog::DB_UpdateClientCrashDownLog(void)
DB_UpdateClientCrashDownLog::DB_UpdateClientCrashDownLog() : DBDispatcher() {
}

// IDA_EA: 0x845E98C
// IDA_NAME: _ZN27DB_UpdateClientCrashDownLogD2Ev
// DEMANGLED: DB_UpdateClientCrashDownLog::~DB_UpdateClientCrashDownLog()
// IDA_EA: 0x845E9BC
// IDA_NAME: _ZN27DB_UpdateClientCrashDownLogD0Ev
// DEMANGLED: DB_UpdateClientCrashDownLog::~DB_UpdateClientCrashDownLog()
DB_UpdateClientCrashDownLog::~DB_UpdateClientCrashDownLog() {
}

// IDA_EA: 0x8427BCE
// IDA_NAME: _ZN27DB_UpdateClientCrashDownLog8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateClientCrashDownLog::dispatch(int,int,Stream *)
bool DB_UpdateClientCrashDownLog::dispatch(int signal, int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;
    int reason = 0;
    // The raw producer and receiver both use CStreamGuard/Stream << and >>
    // int.  Do not narrow this to the old decompiler's char temporary.
    *stream >> reason;
    GetInstanceClientCrashDownLogStatistic()->RegisterClientCrashDown(reason);
    return true;
}


// IDA_EA: 0x844FC8E
// IDA_NAME: _ZN28DB_NoticeIngameAdvertisementC2Ev
// DEMANGLED: DB_NoticeIngameAdvertisement::DB_NoticeIngameAdvertisement(void)
DB_NoticeIngameAdvertisement::DB_NoticeIngameAdvertisement() : DBDispatcher() {
}

// IDA_EA: 0x845B17C
// IDA_NAME: _ZN28DB_NoticeIngameAdvertisementD2Ev
// DEMANGLED: DB_NoticeIngameAdvertisement::~DB_NoticeIngameAdvertisement()
// IDA_EA: 0x845B1AC
// IDA_NAME: _ZN28DB_NoticeIngameAdvertisementD0Ev
// DEMANGLED: DB_NoticeIngameAdvertisement::~DB_NoticeIngameAdvertisement()
DB_NoticeIngameAdvertisement::~DB_NoticeIngameAdvertisement() {
}

// IDA_EA: 0x844710E
// IDA_NAME: _ZN28DB_NoticeIngameAdvertisement8dispatchEiiP6Stream
// DEMANGLED: DB_NoticeIngameAdvertisement::dispatch(int,int,Stream *)
bool DB_NoticeIngameAdvertisement::dispatch(int signal, int user_idx,
                                            Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;
    if (InGameADManagerInstance()->ReloadADExposeInfo() == 1) {
        return true;
    }
    LogManager::logFormat(
        1, "DBThread.cpp",
        "virtual bool DB_NoticeIngameAdvertisement::dispatch(int, int, Stream*)",
        0xb17f, "Fail Load In Game Advertisement Expose Info");
    return false;
}
