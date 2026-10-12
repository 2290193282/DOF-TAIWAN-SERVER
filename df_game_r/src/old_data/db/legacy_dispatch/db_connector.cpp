#include "old_data/db/core/db_connector.h"
#include "old_data/db/core/db_mgr.h"
#include "common/manager/global_data.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "infra/log/format/trace.h"

#include <cstring>

CDBConnector* GlobalData::s_db_connector = 0;

typedef char CDBConnector_Size_Must_Be_0x16C[
    (sizeof(CDBConnector) == 0x16C) ? 1 : -1];

// IDA_EA: 0x80EC5FC
// IDA_NAME: _ZN12CDBConnectorC2EP5DBMgr
// DEMANGLED: CDBConnector::CDBConnector(DBMgr *)
CDBConnector::CDBConnector(DBMgr* db_mgr)
    : db_mgr_(db_mgr), master_info_() {}

// IDA_EA: 0x80EC61A
// IDA_NAME: _ZN12CDBConnectorD2Ev
// DEMANGLED: CDBConnector::~CDBConnector()
CDBConnector::~CDBConnector() {}

// IDA_EA: 0x80EC620
// IDA_NAME: _ZN12CDBConnector4InitER12STDBConnInfo
// DEMANGLED: CDBConnector::Init(STDBConnInfo &)
bool CDBConnector::Init(STDBConnInfo& conn_info) {
    std::memcpy(&master_info_, &conn_info, sizeof(master_info_));
    return OpenMaster() == true;
}

// IDA_EA: 0x80EC668
// IDA_NAME: _ZN12CDBConnector5CloseEv
// DEMANGLED: CDBConnector::Close(void)
int CDBConnector::Close() {
    return CloseMaster();
}

// IDA_EA: 0x80EC67C
// IDA_NAME: _ZN12CDBConnector10OpenMasterEv
// DEMANGLED: CDBConnector::OpenMaster(void)
bool CDBConnector::OpenMaster() {
    return Open(DB_HANDLE_0, SERVER_GROUP_DEFAULT, master_info_) == true;
}

// IDA_EA: 0x80EC6BC
// IDA_NAME: _ZN12CDBConnector11CloseMasterEv
// DEMANGLED: CDBConnector::CloseMaster(void)
int CDBConnector::CloseMaster() {
    DBMgr* mgr = db_mgr_ ? db_mgr_ : GlobalData::s_db_mgr;
    if (!mgr) {
        return 0;
    }
    mgr->Close(DB_HANDLE_0);
    return 1;
}

// IDA_EA: 0x80EC6DA
// IDA_NAME: _ZN12CDBConnector4OpenE18ENUM_DB_HANDLE_IDX17ENUM_SERVER_GROUPR12STDBConnInfo
// DEMANGLED: CDBConnector::Open(ENUM_DB_HANDLE_IDX,ENUM_SERVER_GROUP,STDBConnInfo &)
bool CDBConnector::Open(ENUM_DB_HANDLE_IDX handle_idx,
                        ENUM_SERVER_GROUP server_group,
                        STDBConnInfo& conn_info) {
    if (!conn_info.db_ip[0]) {
        return true;
    }

    DBMgr* mgr = db_mgr_ ? db_mgr_ : GlobalData::s_db_mgr;
    if (!mgr) {
        return false;
    }

    return mgr->Open(handle_idx,
                     conn_info.db_ip,
                     conn_info.db_port,
                     conn_info.db_name,
                     conn_info.db_userid,
                     conn_info.db_passwd,
                     server_group) == true;
}

// IDA_EA: 0x80EC760
// IDA_NAME: _ZN12CDBConnector7ConnectESt3mapI18ENUM_DB_HANDLE_IDX12STDBConnInfoSt4lessIS1_ESaISt4pairIKS1_S2_EEE
// DEMANGLED: CDBConnector::Connect(std::map<ENUM_DB_HANDLE_IDX,STDBConnInfo,std::less<ENUM_DB_HANDLE_IDX>,std::allocator<std::pair<ENUM_DB_HANDLE_IDX const,STDBConnInfo>>>)
int CDBConnector::Connect(
    std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo> conn_info) {
    for (std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo>::iterator it =
             conn_info.begin();
         it != conn_info.end(); ++it) {
        if (it->first != DB_HANDLE_0 && it->first != DB_HANDLE_WEB) {
            cMyTrace trace("bool CDBConnector::Connect(DBConnInfo)", 78, 9,
                           true, true);
            trace("\t\t- Open(type:%2d, %s/%d/%s/%s",
                  static_cast<int>(it->first),
                  it->second.db_ip,
                  it->second.db_port,
                  it->second.db_name,
                  it->second.db_userid);
            if (!Open(it->first, SERVER_GROUP_DEFAULT, it->second)) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x80EC8F8
// IDA_NAME: _ZN12CDBConnector22ConnectWithServerGroupESt3mapI18ENUM_DB_HANDLE_IDX12STDBConnInfoSt4lessIS1_ESaISt4pairIKS1_S2_EEE
// DEMANGLED: CDBConnector::ConnectWithServerGroup(std::map<ENUM_DB_HANDLE_IDX,STDBConnInfo,std::less<ENUM_DB_HANDLE_IDX>,std::allocator<std::pair<ENUM_DB_HANDLE_IDX const,STDBConnInfo>>>)
int CDBConnector::ConnectWithServerGroup(
    std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo> conn_info) {
    for (std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo>::iterator it =
             conn_info.begin();
         it != conn_info.end(); ++it) {
        if (it->first != DB_HANDLE_0) {
            if (!Open(it->first,
                      static_cast<ENUM_SERVER_GROUP>(
                          it->second.server_group),
                      it->second)) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x80EC9CA
// IDA_NAME: _ZN12CDBConnector11GetConnInfoERSt3mapI18ENUM_DB_HANDLE_IDX17ENUM_SERVER_GROUPSt4lessIS1_ESaISt4pairIKS1_S2_EEERS0_IS1_12STDBConnInfoS4_SaIS5_IS6_SB_EEE
// DEMANGLED: CDBConnector::GetConnInfo(std::map<ENUM_DB_HANDLE_IDX,ENUM_SERVER_GROUP,std::less<ENUM_DB_HANDLE_IDX>,std::allocator<std::pair<ENUM_DB_HANDLE_IDX const,ENUM_SERVER_GROUP>>> &,std::map&<ENUM_DB_HANDLE_IDX,STDBConnInfo,std::less<ENUM_DB_HANDLE_IDX>,std::allocator<std::pair<ENUM_DB_HANDLE_IDX const,std::map&>>>)
int CDBConnector::GetConnInfo(
    std::map<ENUM_DB_HANDLE_IDX, ENUM_SERVER_GROUP>& groups,
    std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo>& out_info) {
    for (std::map<ENUM_DB_HANDLE_IDX, ENUM_SERVER_GROUP>::iterator it =
             groups.begin();
         it != groups.end(); ++it) {
        if (QueryConnInfo(it->first, it->second, out_info) != 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x80ECA6C
// IDA_NAME: _ZN12CDBConnector13QueryConnInfoE18ENUM_DB_HANDLE_IDX17ENUM_SERVER_GROUPRSt3mapIS0_12STDBConnInfoSt4lessIS0_ESaISt4pairIKS0_S3_EEE
// DEMANGLED: CDBConnector::QueryConnInfo(ENUM_DB_HANDLE_IDX,ENUM_SERVER_GROUP,std::map<ENUM_DB_HANDLE_IDX,STDBConnInfo,std::less<ENUM_DB_HANDLE_IDX>,std::allocator<std::pair<ENUM_DB_HANDLE_IDX const,STDBConnInfo>>> &)
int CDBConnector::QueryConnInfo(
    ENUM_DB_HANDLE_IDX handle_idx,
    ENUM_SERVER_GROUP server_group,
    std::map<ENUM_DB_HANDLE_IDX, STDBConnInfo>& out_info) {
    DBMgr* mgr = db_mgr_ ? db_mgr_ : GlobalData::s_db_mgr;
    if (!mgr) {
        return 0;
    }

    MySQL* db_handle = mgr->GetDBHandle(DB_HANDLE_0, SERVER_GROUP_DEFAULT);
    if (db_handle->set_query(
            "seLect db_name,db_ip,db_port,db_userid,db_passwd from db_connect where db_server_group = %d and db_type = %d",
            static_cast<int>(server_group),
            static_cast<int>(handle_idx)) != true) {
        return 0;
    }
    if (db_handle->exec(true) != true) {
        return 0;
    }
    if (db_handle->fetch() != true) {
        return 0;
    }
    if (db_handle->get_n_rows() != 1) {
        return 0;
    }

    STDBConnInfo info;
    int column = 0;
    if (db_handle->get_str(column++, info.db_name, sizeof(info.db_name)) !=
        true) {
        return 0;
    }
    if (db_handle->get_str(column++, info.db_ip, sizeof(info.db_ip)) != true) {
        return 0;
    }
    if (db_handle->get_int(column++, info.db_port) != true) {
        return 0;
    }
    if (db_handle->get_str(column++, info.db_userid,
                           sizeof(info.db_userid)) != true) {
        return 0;
    }

    char encrypted[60];
    std::memset(encrypted, 0, sizeof(encrypted));
    if (db_handle->get_str(column++, encrypted, sizeof(encrypted)) != true) {
        return 0;
    }

    CEnvironment* env = G_CEnvironment();
    if (!env || env->DecryptValue(encrypted, info.db_passwd) != true) {
        return 0;
    }

    info.server_group = static_cast<unsigned int>(server_group);
    out_info[handle_idx] = info;
    return 1;
}
