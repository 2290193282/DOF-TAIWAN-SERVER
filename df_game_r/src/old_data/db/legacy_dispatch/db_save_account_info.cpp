#include "old_data/db/account/db_save_account_info.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

#pragma pack(push, 1)
struct SIG_SAVE_ACCOUNT_INFO {
    uint32_t m_id;             // +0
    unsigned char pcbang_flag; // +4
    unsigned char reserved_5[3];
    uint32_t last_play_time;   // +8
    unsigned char reserved_12[16];
    char ip[12];               // +28
    char end_ip[4];            // +40
    char mac_addr[68];         // +44
};
#pragma pack(pop)

typedef char SIG_SAVE_ACCOUNT_INFO_Size_Must_Be_112[
    (sizeof(SIG_SAVE_ACCOUNT_INFO) == 112) ? 1 : -1];

// IDA_EA: 0x844EB9A
// IDA_NAME: _ZN18DB_SaveAccountInfoC2Ev
// DEMANGLED: DB_SaveAccountInfo::DB_SaveAccountInfo(void)
DB_SaveAccountInfo::DB_SaveAccountInfo() {}

// IDA_EA: 0x845D8CA
// IDA_NAME: _ZN18DB_SaveAccountInfoD2Ev
// DEMANGLED: DB_SaveAccountInfo::~DB_SaveAccountInfo()
// IDA_EA: 0x845D8FA
// IDA_NAME: _ZN18DB_SaveAccountInfoD0Ev
// DEMANGLED: DB_SaveAccountInfo::~DB_SaveAccountInfo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveAccountInfo::~DB_SaveAccountInfo() {}

// IDA_EA: 0x8432646
// IDA_NAME: _ZN18DB_SaveAccountInfo8dispatchEiiP6Stream
// DEMANGLED: DB_SaveAccountInfo::dispatch(int,int,Stream *)
bool DB_SaveAccountInfo::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_ACCOUNT_INFO account;
    if (stream->get_binary(&account, sizeof(account)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6), static_cast<ENUM_SERVER_GROUP>(0));
    const char* account_id = NumberToString(account.m_id, 0);
    db_handle->set_query(
        "upDate member_play_info set ip=\"%s\", last_play_time=%u, pcbang_flag=%d, end_ip=\"%s\", mac_addr=\"%s\" where occ_date=now() and m_id=%s",
        account.ip,
        account.last_play_time,
        static_cast<int>(account.pcbang_flag),
        account.end_ip,
        account.mac_addr,
        account_id);

    bool should_insert = true;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount() != 0) {
            should_insert = false;
        }
    }

    if (should_insert) {
        account_id = NumberToString(account.m_id, 0);
        db_handle->set_query(
            "inSert into member_play_info(occ_date, m_id, play_count, ip, last_play_time, pcbang_flag, end_ip, mac_addr) values(now(), %s, 1, \"%s\", %u, %d, \"%s\", \"%s\")",
            account_id,
            account.ip,
            account.last_play_time,
            static_cast<int>(account.pcbang_flag),
            account.end_ip,
            account.mac_addr);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
