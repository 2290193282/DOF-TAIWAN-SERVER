#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_TYPES_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_TYPES_H_

// IDA type: ENUM_DB_HANDLE_IDX.  Values 0..16 retain the old executable's
// numeric ABI.  The reconstructed native-0828 worker owns the appended
// handles; each is a distinct MySQL object/socket even when it targets the
// same schema as its legacy counterpart.
enum ENUM_DB_HANDLE_IDX {
    DB_HANDLE_0 = 0,
    DB_HANDLE_1 = 1,
    DB_HANDLE_2 = 2,
    DB_HANDLE_3 = 3,
    DB_HANDLE_LOG = 4,
    DB_HANDLE_WEB = 5,
    DB_HANDLE_LOG_REF = 6,
    DB_HANDLE_GUILD = 8,
    DB_HANDLE_LOGIN = 9,
    // Values 15 and 16 remain legacy-owned.  In particular, deployed
    // db_connect type 15 is d_technical_report and must never be repurposed.
    // Native 0828 connections therefore begin strictly after the old 0..16
    // handle range.
    DB_HANDLE_FEATURE_DB3 = 17,
    // Compatibility spelling retained for the already-restored SQL004 owner.
    DB_HANDLE_CLONE_TITLE = DB_HANDLE_FEATURE_DB3,
    DB_HANDLE_FEATURE_DB2 = 18,
    DB_HANDLE_FEATURE_DB4 = 19,
    DB_HANDLE_FEATURE_DB9 = 20,
    DB_HANDLE_COUNT = 21,
};

inline bool IsFeatureDBHandle(ENUM_DB_HANDLE_IDX handle_idx) {
    return handle_idx == DB_HANDLE_FEATURE_DB3 ||
           handle_idx == DB_HANDLE_FEATURE_DB2 ||
           handle_idx == DB_HANDLE_FEATURE_DB4 ||
           handle_idx == DB_HANDLE_FEATURE_DB9;
}

// IDA type: ENUM_SERVER_GROUP.  Group zero selects the primary DB handles;
// nonzero values select the DBMgr server-group map.
enum ENUM_SERVER_GROUP : int {
    SERVER_GROUP_DEFAULT = 0,
};

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_TYPES_H_
