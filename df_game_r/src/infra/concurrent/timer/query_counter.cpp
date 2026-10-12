#include "infra/concurrent/timer/query_counter.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <ctime>
#include <pthread.h>


extern int GetIdentityFromDB(MySQL* mysql) __asm__("_Z17GetIdentityFromDBP5MySQL");

namespace {

pthread_mutex_t g_query_counter_mutex;
pthread_once_t g_query_counter_mutex_once = PTHREAD_ONCE_INIT;

void InitQueryCounterMutex() {
    pthread_mutexattr_t attributes;
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_query_counter_mutex, &attributes);
    pthread_mutexattr_destroy(&attributes);
}

class QueryCounterLock {
public:
    QueryCounterLock() {
        pthread_once(&g_query_counter_mutex_once, InitQueryCounterMutex);
        pthread_mutex_lock(&g_query_counter_mutex);
    }
    ~QueryCounterLock() { pthread_mutex_unlock(&g_query_counter_mutex); }

private:
    QueryCounterLock(const QueryCounterLock&);
    QueryCounterLock& operator=(const QueryCounterLock&);
};

}  // namespace

typedef char QueryInfo_Size_Must_Be_12[
    (sizeof(CQueryCounter::QUERY_INFO) == 12) ? 1 : -1];
typedef char CQueryCounter_Size_Must_Be_60[
    (sizeof(CQueryCounter) == 60) ? 1 : -1];

// IDA_EA: 0x85ED760
// IDA_NAME: _ZN13CQueryCounter10QUERY_INFOC2Ev
// DEMANGLED: CQueryCounter::QUERY_INFO::QUERY_INFO(void)
CQueryCounter::QUERY_INFO::QUERY_INFO() : count(0), response_time(0.0) {}

// IDA_EA: 0x85ECAEC
// IDA_NAME: _ZN13CQueryCounterC2Ev
// DEMANGLED: CQueryCounter::CQueryCounter(void)
CQueryCounter::CQueryCounter()
    : log_db_(false),
      query_count_table_(),
      query_id_map_(),
      last_query_(query_count_table_.end()),
      dirty_query_count_(0) {}

// IDA_EA: 0x85ECB8A
// IDA_NAME: _ZN13CQueryCounterD2Ev
// DEMANGLED: CQueryCounter::~CQueryCounter()
CQueryCounter::~CQueryCounter() {}

// IDA_EA: 0x844D63A
// IDA_NAME: _ZN13CQueryCounter8setLogDBEb
// DEMANGLED: CQueryCounter::setLogDB(bool)
CQueryCounter* CQueryCounter::setLogDB(bool is_log_db) {
    log_db_ = is_log_db;
    return this;
}

// IDA_EA: 0x844D652
// IDA_NAME: _ZN13CQueryCounter18GetQueryCountTableEv
// DEMANGLED: CQueryCounter::GetQueryCountTable(void)
CQueryCounter::QueryCountTable* CQueryCounter::GetQueryCountTable() {
    return &query_count_table_;
}

// IDA_EA: 0x85ECBD6
// IDA_NAME: _ZN13CQueryCounter5ResetEv
// DEMANGLED: CQueryCounter::Reset(void)
CQueryCounter* CQueryCounter::Reset() {
    for (QueryCountIterator it = query_count_table_.begin();
         it != query_count_table_.end(); ++it) {
        it->second.count = 0;
        it->second.response_time = 0.0;
    }
    dirty_query_count_ = 0;
    return this;
}

// IDA_EA: 0x85ECC5E
// IDA_NAME: _ZN13CQueryCounter18IncreaseQueryCountERKSs
// DEMANGLED: CQueryCounter::IncreaseQueryCount(std::string const&)
void CQueryCounter::IncreaseQueryCount(const std::string& query) {
    QueryCounterLock lock;
    if (query.find("log_query_ref", 0) != std::string::npos ||
        query.find("from db_connect", 0) != std::string::npos) {
        return;
    }

    std::map<std::string, QueryCountIterator>::iterator found =
        query_id_map_.find(query);
    if (found == query_id_map_.end()) {
        if (log_db_) {
            Stream* stream =
                GlobalData::stream_pool()->Acquire("QueryCounter.cpp", 466);
            CStreamGuard guard(stream, true);
            guard << 190;
            guard << -1;
            guard << static_cast<int>(query.size());
            guard->put_binary(const_cast<char*>(query.c_str()),
                              static_cast<int>(query.size()));
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        } else {
            int query_id = 0;
            if (SaveQuery(query.c_str(), query_id)) {
                RegisterQuery(query.c_str(), query_id, 0);
            }
        }
        return;
    }

    last_query_ = found->second;
    if (last_query_->second.count == 0) {
        ++dirty_query_count_;
    }
    ++last_query_->second.count;
}

// IDA_EA: 0x85ECEE6
// IDA_NAME: _ZN13CQueryCounter18UpdateQueryCounterEv
// DEMANGLED: CQueryCounter::UpdateQueryCounter(void)
void CQueryCounter::UpdateQueryCounter() {
    QueryCounterLock lock;
    Stream* stream = GlobalData::stream_pool()->Acquire("QueryCounter.cpp", 497);
    CStreamGuard guard(stream, true);
    guard << 125;
    guard << -1;
    guard << static_cast<int>(std::time(0));
    guard << dirty_query_count_;

    for (QueryCountIterator it = query_count_table_.begin();
         it != query_count_table_.end(); ++it) {
        const int query_id = it->first;
        const int count = it->second.count;
        const int response_time =
            static_cast<int>(it->second.response_time * 1000.0);
        if (count > 0) {
            guard << query_id;
            guard << count;
            guard << response_time;
        }
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
    Reset();
}

// IDA_EA: 0x85ED108
// IDA_NAME: _ZN13CQueryCounter15SetResponseTimeEd
// DEMANGLED: CQueryCounter::SetResponseTime(double)
bool CQueryCounter::SetResponseTime(double elapsed_seconds) {
    QueryCounterLock lock;
    if (last_query_ != query_count_table_.end()) {
        last_query_->second.response_time += elapsed_seconds;
        last_query_ = query_count_table_.end();
        return true;
    }
    return false;
}

// IDA_EA: 0x85ED18A
// IDA_NAME: _ZN13CQueryCounter14LoadQueryTableEv
// DEMANGLED: CQueryCounter::LoadQueryTable(void)
bool CQueryCounter::LoadQueryTable() {
    QueryCounterLock lock;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_LOG_REF,
                                                         SERVER_GROUP_DEFAULT);
    db_handle->set_query("seLect q_id,query from log_query_ref");
    if (db_handle->exec(true) != true) {
        return false;
    }

    const int rows = db_handle->get_n_rows();
    for (int row = 0; row < rows; ++row) {
        int query_id = 0;
        char query[24577];
        std::memset(query, 0, sizeof(query));
        if (db_handle->fetch() != true) {
            cMyTrace trace("bool CQueryCounter::LoadQueryTable()", 559, 5);
            trace("LoadQueryTable, fatch ERROR");
            return false;
        }
        if (db_handle->get_int(0, query_id) != true) {
            cMyTrace trace("bool CQueryCounter::LoadQueryTable()", 565, 5);
            trace("LoadQueryTable, get_int(0) ERROR");
            return false;
        }
        if (db_handle->get_str(1, query, 24576) != true) {
            cMyTrace trace("bool CQueryCounter::LoadQueryTable()", 571, 5);
            trace("LoadQueryTable, get_str(1) ERROR");
            return false;
        }
        if (RegisterQuery(query, query_id, 0) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x85ED3A2
// IDA_NAME: _ZN13CQueryCounter9SaveQueryEPKcRi
// DEMANGLED: CQueryCounter::SaveQuery(char const*,int&)
bool CQueryCounter::SaveQuery(const char* query, int& query_id) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_LOG_REF,
                                                         SERVER_GROUP_DEFAULT);
    char escaped[49153];
    std::memset(escaped, 0, sizeof(escaped));
    db_handle->escape_string(escaped, query);
    db_handle->set_query(
        "inSert into log_query_ref(query, query_hash) values('%s',password('%s'))",
        escaped,
        escaped);
    if (db_handle->exec(true) == true) {
        query_id = GetIdentityFromDB(db_handle);
        return true;
    }

    db_handle->set_query(
        "seLect q_id from log_query_ref where query_hash=password('%s')",
        escaped);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool CQueryCounter::SaveQuery(const char*, int&)",
                       613,
                       5);
        trace("SaveQuery, exec() ERROR");
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool CQueryCounter::SaveQuery(const char*, int&)",
                       625,
                       5);
        trace("SaveQuery, fetch() ERROR");
        return false;
    }
    if (db_handle->get_int(0, query_id) != true) {
        cMyTrace trace("bool CQueryCounter::SaveQuery(const char*, int&)",
                       631,
                       5);
        trace("SaveQuery, get_int(0) ERROR");
        return false;
    }
    return true;
}

// IDA_EA: 0x85ED5AE
// IDA_NAME: _ZN13CQueryCounter13RegisterQueryEPKcii
// DEMANGLED: CQueryCounter::RegisterQuery(char const*,int,int)
bool CQueryCounter::RegisterQuery(const char* query,
                                  int query_id,
                                  int initial_count) {
    QUERY_INFO info;
    info.count = initial_count;
    std::pair<QueryCountIterator, bool> query_insert =
        query_count_table_.insert(std::make_pair(query_id, info));
    if (query_insert.second != true) {
        return false;
    }

    std::pair<std::map<std::string, QueryCountIterator>::iterator, bool>
        map_insert =
            query_id_map_.insert(std::make_pair(query, query_insert.first));
    if (map_insert.second == true) {
        return true;
    }

    cMyTrace trace("bool CQueryCounter::RegisterQuery(const char*, int, int)",
                   657,
                   0);
    trace("LoadQueryTable,  query_id_map_.insert ERROR");
    return false;
}
