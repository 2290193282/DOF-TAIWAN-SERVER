#ifndef OLDSRV_DB_DB_CONNECTOR_H_
#define OLDSRV_DB_DB_CONNECTOR_H_

#include "old_data/db/core/db_conn_info.h"

#include <map>


class DBMgr;

// IDA unit: 99_misc_global/CDBConnector.
class CDBConnector {
public:
    explicit CDBConnector(DBMgr* db_mgr);
    ~CDBConnector();

    bool Init(STDBConnInfo& conn_info);
    int Close();
    bool OpenMaster();
    int CloseMaster();
    bool Open(ENUM_DB_HANDLE_IDX handle_idx, ENUM_SERVER_GROUP server_group,
              STDBConnInfo& conn_info);
    int Connect(std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo> conn_info);
    int ConnectWithServerGroup(
        std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo> conn_info);
    int GetConnInfo(std::map<ENUM_DB_HANDLE_IDX, ENUM_SERVER_GROUP>& groups,
                    std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo>& out_info);
    int QueryConnInfo(ENUM_DB_HANDLE_IDX handle_idx,
                      ENUM_SERVER_GROUP server_group,
                      std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo>& out_info);

private:
    DBMgr* db_mgr_;
    STDBConnInfo master_info_;
};


#endif  // OLDSRV_DB_DB_CONNECTOR_H_
