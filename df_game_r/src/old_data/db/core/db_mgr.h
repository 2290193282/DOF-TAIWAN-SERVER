#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_MGR_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_MGR_H_

#include "old_data/db/core/db_types.h"

#include <ext/hash_map>
#include <map>
#include <stddef.h>
#include <stdint.h>


class CQueryCounter;
class MySQL;

#if defined(__i386__)
struct DBMgrI386LayoutVerifier;
#endif

// IDA unit: 99_misc_global/DBMgr.
// Layout is allocated with size 0x38 by GlobalData::Init @0x829A35F.
class DBMgr {
public:
    struct stUserDBInfo_t {
        uint32_t m_id;
        uint32_t charac_no;
        uint32_t server_id;
    };

    DBMgr();
    ~DBMgr();

    DBMgr* setUserIdx(int user_idx);
    DBMgr* activateGlobalDB();
    bool Open(ENUM_DB_HANDLE_IDX handle_idx, char* host, char* database,
              char* user, char* password, ENUM_SERVER_GROUP server_group);
    bool Open(ENUM_DB_HANDLE_IDX handle_idx, char* host, unsigned int port,
              char* database, char* user, char* password,
              ENUM_SERVER_GROUP server_group);
    bool Close();
    MySQL* Close(ENUM_DB_HANDLE_IDX handle_idx);
    int GetServerType(int user_idx);
    stUserDBInfo_t* addUserDBInfo(int user_idx,
                                  const stUserDBInfo_t& info);
    int removeUserDBInfo(int user_idx);
    MySQL* GetDBHandleServerGroup(ENUM_DB_HANDLE_IDX handle_idx,
                                  ENUM_SERVER_GROUP server_group);
    // Explicit lookup for the native 0828 worker.  This path never consults
    // user_idx_, never inserts into the group maps, and never falls back to a
    // different schema connection.
    MySQL* GetDBHandleExact(ENUM_DB_HANDLE_IDX handle_idx,
                            ENUM_SERVER_GROUP server_group);
    int SetQueryCounterPointer(CQueryCounter* counter);
    MySQL* GetDBHandle(ENUM_DB_HANDLE_IDX handle_idx,
                       ENUM_SERVER_GROUP server_group);
    int isActiveGlobalDB();

    // The dedicated worker sets this thread-local context for its complete
    // dispatch loop.  Legacy handle requests 2/3/4/9 are then fail-closed
    // remapped to the worker-owned connections 18/17/19/20 respectively.
    static void SetFeatureDBThreadContext(bool enabled);
    static bool IsFeatureDBThreadContext();

private:
#if defined(__i386__)
    friend struct DBMgrI386LayoutVerifier;
#endif
    typedef __gnu_cxx::hash_map<int, stUserDBInfo_t> UserDBInfoMap;
    typedef std::map<ENUM_DB_HANDLE_IDX, MySQL*> DBHandleMap;
    typedef std::map<ENUM_SERVER_GROUP, DBHandleMap> ServerGroupHandleMap;

    MySQL* primary_handles_;              // +0x00
    int user_idx_;                        // +0x04
    bool active_global_db_;               // +0x08
    UserDBInfoMap user_db_info_;          // +0x0c
    ServerGroupHandleMap group_handles_;  // +0x20
};

// The target is i386.  These assertions deliberately check every live IDA
// member boundary while avoiding a false failure in host-only x86_64 tools.
#if defined(__i386__)
struct DBMgrI386LayoutVerifier {
    typedef char UserDBInfoSize[
        (sizeof(DBMgr::stUserDBInfo_t) == 0x0c) ? 1 : -1];
    typedef char DBMgrSize[(sizeof(DBMgr) == 0x38) ? 1 : -1];
    typedef char PrimaryHandlesOffset[
        (offsetof(DBMgr, primary_handles_) == 0x00) ? 1 : -1];
    typedef char UserIdxOffset[
        (offsetof(DBMgr, user_idx_) == 0x04) ? 1 : -1];
    typedef char ActiveGlobalDBOffset[
        (offsetof(DBMgr, active_global_db_) == 0x08) ? 1 : -1];
    typedef char UserDBInfoOffset[
        (offsetof(DBMgr, user_db_info_) == 0x0c) ? 1 : -1];
    typedef char GroupHandlesOffset[
        (offsetof(DBMgr, group_handles_) == 0x20) ? 1 : -1];
};
#endif

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_MGR_H_
