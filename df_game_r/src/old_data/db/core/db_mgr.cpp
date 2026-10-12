#include "old_data/db/core/db_mgr.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/timer/query_counter.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"


typedef char DBMgr_UserDBInfo_Size_Must_Be_12[
    (sizeof(DBMgr::stUserDBInfo_t) == 12) ? 1 : -1];

namespace {

// POD TLS is supported by the target GCC 4.9/i386 runtime and does not alter
// DBMgr's fixed 0x38-byte object layout.
__thread int g_feature_db_context = 0;

bool IsValidHandleIndex(ENUM_DB_HANDLE_IDX handle_idx) {
    const int index = static_cast<int>(handle_idx);
    return index >= 0 && index < DB_HANDLE_COUNT;
}

bool RemapFeatureDBHandle(ENUM_DB_HANDLE_IDX requested,
                           ENUM_DB_HANDLE_IDX& mapped) {
    switch (requested) {
        case DB_HANDLE_2:
            mapped = DB_HANDLE_FEATURE_DB2;
            return true;
        case DB_HANDLE_3:
        case DB_HANDLE_FEATURE_DB3:
            mapped = DB_HANDLE_FEATURE_DB3;
            return true;
        case DB_HANDLE_LOG:
        case DB_HANDLE_FEATURE_DB4:
            mapped = DB_HANDLE_FEATURE_DB4;
            return true;
        case DB_HANDLE_LOGIN:
        case DB_HANDLE_FEATURE_DB9:
            mapped = DB_HANDLE_FEATURE_DB9;
            return true;
        case DB_HANDLE_FEATURE_DB2:
            mapped = DB_HANDLE_FEATURE_DB2;
            return true;
        default:
            return false;
    }
}

}  // namespace

// IDA_EA: 0x818FB1A
// IDA_NAME: _ZN5DBMgr10setUserIdxEi
// DEMANGLED: DBMgr::setUserIdx(int)
DBMgr* DBMgr::setUserIdx(int user_idx) {
    user_idx_ = user_idx;
    return this;
}

// IDA_EA: 0x82A420C
// IDA_NAME: _ZN5DBMgr16activateGlobalDBEv
// DEMANGLED: DBMgr::activateGlobalDB(void)
DBMgr* DBMgr::activateGlobalDB() {
    active_global_db_ = true;
    return this;
}

// IDA_EA: 0x83F48DE
// IDA_NAME: _ZN5DBMgrC2Ev
// DEMANGLED: DBMgr::DBMgr(void)
DBMgr::DBMgr() {
    // user_db_info_ and group_handles_ have completed their construction
    // before this body.  The old constructor then assigns the primary array,
    // initializes every entry, and only then writes +0x08/+0x04.
    primary_handles_ = new MySQL[DB_HANDLE_COUNT];
    if (!primary_handles_) {
        cMyTrace trace("DBMgr::DBMgr()", 769, 5);
        trace("[%s][%d]", "DBMgr::DBMgr()", 769);
    }

    for (int index = 0; index < DB_HANDLE_COUNT; ++index) {
        primary_handles_[index].init();
    }

    active_global_db_ = false;
    user_idx_ = -1;
}

// IDA_EA: 0x83F4A1C
// IDA_NAME: _ZN5DBMgrD2Ev
// DEMANGLED: DBMgr::~DBMgr()
DBMgr::~DBMgr() {
    Close();
    delete[] primary_handles_;

    for (ServerGroupHandleMap::iterator group_it = group_handles_.begin();
         group_it != group_handles_.end(); ++group_it) {
        DBHandleMap& handles = group_it->second;
        for (DBHandleMap::iterator handle_it = handles.begin();
             handle_it != handles.end(); ++handle_it) {
            if (handle_it->second) {
                delete handle_it->second;
            }
        }
    }
}

// IDA_EA: 0x83F4BC4
// IDA_NAME: _ZN5DBMgr4OpenE18ENUM_DB_HANDLE_IDXPcS1_S1_S1_17ENUM_SERVER_GROUP
// DEMANGLED: DBMgr::Open(ENUM_DB_HANDLE_IDX,char *,char *,char *,char *,ENUM_SERVER_GROUP)
bool DBMgr::Open(ENUM_DB_HANDLE_IDX handle_idx, char* host, char* database,
                 char* user, char* password,
                 ENUM_SERVER_GROUP server_group) {
    if (!IsValidHandleIndex(handle_idx)) {
        return false;
    }
    if (server_group == SERVER_GROUP_DEFAULT) {
        return primary_handles_[handle_idx].open(host, database, user,
                                                 password);
    }

    MySQL*& db_handle = group_handles_[server_group][handle_idx];
    if (db_handle) {
        return false;
    }

    db_handle = new MySQL();
    if (db_handle->init() != true) {
        return false;
    }
    return db_handle->open(host, database, user, password) == true;
}

// IDA_EA: 0x83F4D2E
// IDA_NAME: _ZN5DBMgr4OpenE18ENUM_DB_HANDLE_IDXPcjS1_S1_S1_17ENUM_SERVER_GROUP
// DEMANGLED: DBMgr::Open(ENUM_DB_HANDLE_IDX,char *,unsigned int,char *,char *,char *,ENUM_SERVER_GROUP)
bool DBMgr::Open(ENUM_DB_HANDLE_IDX handle_idx, char* host, unsigned int port,
                 char* database, char* user, char* password,
                 ENUM_SERVER_GROUP server_group) {
    if (!IsValidHandleIndex(handle_idx)) {
        return false;
    }
    if (server_group == SERVER_GROUP_DEFAULT) {
        return primary_handles_[handle_idx].open(host, port, database, user,
                                                 password);
    }

    MySQL*& db_handle = group_handles_[server_group][handle_idx];
    if (db_handle) {
        return false;
    }

    db_handle = new MySQL();
    if (db_handle->init() != true) {
        return false;
    }
    return db_handle->open(host, port, database, user, password) == true;
}

// IDA_EA: 0x83F4EA6
// IDA_NAME: _ZN5DBMgr5CloseEv
// DEMANGLED: DBMgr::Close(void)
bool DBMgr::Close() {
    for (int index = 0; index < DB_HANDLE_COUNT; ++index) {
        primary_handles_[index].close();
    }
    return false;
}

// IDA_EA: 0x83F4EE0
// IDA_NAME: _ZN5DBMgr5CloseE18ENUM_DB_HANDLE_IDX
// DEMANGLED: DBMgr::Close(ENUM_DB_HANDLE_IDX)
MySQL* DBMgr::Close(ENUM_DB_HANDLE_IDX handle_idx) {
    const int index = static_cast<int>(handle_idx);
    if (index >= 0 && index < DB_HANDLE_COUNT) {
        return primary_handles_[index].close();
    }
    return reinterpret_cast<MySQL*>(index);
}

// IDA_EA: 0x83F4F10
// IDA_NAME: _ZN5DBMgr13GetServerTypeEi
// DEMANGLED: DBMgr::GetServerType(int)
int DBMgr::GetServerType(int user_idx) {
    UserDBInfoMap::const_iterator it = user_db_info_.find(user_idx);
    if (it != user_db_info_.end()) {
        return static_cast<int>(it->second.server_id);
    }
    return 0;
}

// IDA_EA: 0x83F4F9E
// IDA_NAME: _ZN5DBMgr13addUserDBInfoEiRKNS_14stUserDBInfo_tE
// DEMANGLED: DBMgr::addUserDBInfo(int,DBMgr::stUserDBInfo_t const&)
DBMgr::stUserDBInfo_t* DBMgr::addUserDBInfo(
    int user_idx, const stUserDBInfo_t& info) {
    const std::pair<UserDBInfoMap::iterator, bool> inserted =
        user_db_info_.insert(std::make_pair(user_idx, info));
    if (inserted.second) {
        return 0;
    }

    stUserDBInfo_t& stored = user_db_info_[user_idx];
    stored.m_id = info.m_id;
    stored.charac_no = info.charac_no;
    stored.server_id = info.server_id;
    return &stored;
}

// IDA_EA: 0x83F5038
// IDA_NAME: _ZN5DBMgr16removeUserDBInfoEi
// DEMANGLED: DBMgr::removeUserDBInfo(int)
int DBMgr::removeUserDBInfo(int user_idx) {
    if (!GlobalData::s_db_mgr->isActiveGlobalDB()) {
        return 1;
    }
    return static_cast<int>(user_db_info_.erase(user_idx));
}

// IDA_EA: 0x83F506C
// IDA_NAME: _ZN5DBMgr22GetDBHandleServerGroupE18ENUM_DB_HANDLE_IDX17ENUM_SERVER_GROUP
// DEMANGLED: DBMgr::GetDBHandleServerGroup(ENUM_DB_HANDLE_IDX,ENUM_SERVER_GROUP)
MySQL* DBMgr::GetDBHandleServerGroup(ENUM_DB_HANDLE_IDX handle_idx,
                                     ENUM_SERVER_GROUP server_group) {
    if (IsFeatureDBThreadContext()) {
        ENUM_DB_HANDLE_IDX mapped = DB_HANDLE_0;
        if (!RemapFeatureDBHandle(handle_idx, mapped)) {
            return 0;
        }
        return GetDBHandleExact(mapped, server_group);
    }
    if (!IsValidHandleIndex(handle_idx)) {
        return 0;
    }
    if (IsFeatureDBHandle(handle_idx)) {
        // Dedicated sockets are never exposed to legacy/game threads.  Open()
        // may initialize them during bootstrap, but only the queue-6 worker's
        // TLS context may retrieve and execute on them.
        return 0;
    }
    if (server_group == SERVER_GROUP_DEFAULT) {
        UserDBInfoMap::const_iterator it = user_db_info_.find(user_idx_);
        if (it != user_db_info_.end()) {
            server_group =
                static_cast<ENUM_SERVER_GROUP>(it->second.server_id);
        }
    }

    if (server_group != SERVER_GROUP_DEFAULT) {
        ServerGroupHandleMap::const_iterator group_it =
            group_handles_.find(server_group);
        if (group_it == group_handles_.end()) {
            return 0;
        }
        DBHandleMap::const_iterator handle_it =
            group_it->second.find(handle_idx);
        return handle_it != group_it->second.end() ? handle_it->second : 0;
    }
    if (user_idx_ == -1) {
        return primary_handles_ + handle_idx;
    }
    return 0;
}

MySQL* DBMgr::GetDBHandleExact(ENUM_DB_HANDLE_IDX handle_idx,
                               ENUM_SERVER_GROUP server_group) {
    if (!IsValidHandleIndex(handle_idx)) {
        return 0;
    }
    if (IsFeatureDBHandle(handle_idx) &&
        !IsFeatureDBThreadContext()) {
        return 0;
    }
    if (!active_global_db_ || server_group == SERVER_GROUP_DEFAULT) {
        return primary_handles_ + static_cast<int>(handle_idx);
    }

    ServerGroupHandleMap::const_iterator group_it =
        group_handles_.find(server_group);
    if (group_it == group_handles_.end()) {
        return 0;
    }
    DBHandleMap::const_iterator handle_it =
        group_it->second.find(handle_idx);
    if (handle_it == group_it->second.end()) {
        return 0;
    }
    return handle_it->second;
}

// IDA_EA: 0x83F5150
// IDA_NAME: _ZN5DBMgr22SetQueryCounterPointerEP13CQueryCounter
// DEMANGLED: DBMgr::SetQueryCounterPointer(CQueryCounter *)
int DBMgr::SetQueryCounterPointer(CQueryCounter* counter) {
    for (ServerGroupHandleMap::iterator group_it = group_handles_.begin();
         group_it != group_handles_.end(); ++group_it) {
        DBHandleMap& handles = group_it->second;
        for (DBHandleMap::iterator handle_it = handles.begin();
             handle_it != handles.end(); ++handle_it) {
            MySQL* const db_handle = handle_it->second;
            if (db_handle && !IsFeatureDBHandle(handle_it->first)) {
                db_handle->SetQueryCounter(counter);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x83F523E
// IDA_NAME: _ZN5DBMgr11GetDBHandleE18ENUM_DB_HANDLE_IDX17ENUM_SERVER_GROUP
// DEMANGLED: DBMgr::GetDBHandle(ENUM_DB_HANDLE_IDX,ENUM_SERVER_GROUP)
MySQL* DBMgr::GetDBHandle(ENUM_DB_HANDLE_IDX handle_idx,
                          ENUM_SERVER_GROUP server_group) {
    if (IsFeatureDBThreadContext()) {
        ENUM_DB_HANDLE_IDX mapped = DB_HANDLE_0;
        if (!RemapFeatureDBHandle(handle_idx, mapped)) {
            return 0;
        }
        return GetDBHandleExact(mapped, server_group);
    }
    if (!IsValidHandleIndex(handle_idx)) {
        return 0;
    }
    if (IsFeatureDBHandle(handle_idx)) {
        return 0;
    }
    MySQL* const primary_base = primary_handles_;
    if (active_global_db_ != true) {
        return primary_base + handle_idx;
    }
    if (handle_idx != DB_HANDLE_2 && handle_idx != DB_HANDLE_3) {
        return primary_base + handle_idx;
    }

    MySQL* const db_handle =
        GetDBHandleServerGroup(handle_idx, server_group);
    if (db_handle) {
        return db_handle;
    }
    return primary_base + DB_HANDLE_LOG;
}

// IDA_EA: 0x83F533C
// IDA_NAME: _ZN5DBMgr16isActiveGlobalDBEv
// DEMANGLED: DBMgr::isActiveGlobalDB(void)
int DBMgr::isActiveGlobalDB() {
    return active_global_db_;
}

void DBMgr::SetFeatureDBThreadContext(bool enabled) {
    g_feature_db_context = enabled ? 1 : 0;
}

bool DBMgr::IsFeatureDBThreadContext() {
    return g_feature_db_context != 0;
}
