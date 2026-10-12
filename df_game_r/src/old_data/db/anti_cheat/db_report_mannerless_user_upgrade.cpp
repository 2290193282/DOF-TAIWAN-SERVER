#include "old_data/db/anti_cheat/db_report_mannerless_user_upgrade.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/mannerless_user_accusation_packet.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

namespace {

const int kMannerlessLogDbHandle = 4;

}  // namespace

// IDA_EA: 0x844E692
// IDA_NAME: _ZN31DB_ReportMannerlessUser_UpgradeC2Ev
// DEMANGLED: DB_ReportMannerlessUser_Upgrade::DB_ReportMannerlessUser_Upgrade(void)
DB_ReportMannerlessUser_Upgrade::DB_ReportMannerlessUser_Upgrade()
    : DBDispatcher() {}

// IDA_EA: 0x845EA28
// IDA_NAME: _ZN31DB_ReportMannerlessUser_UpgradeD2Ev
// DEMANGLED: DB_ReportMannerlessUser_Upgrade::~DB_ReportMannerlessUser_Upgrade()
// IDA_EA: 0x845EA58
// IDA_NAME: _ZN31DB_ReportMannerlessUser_UpgradeD0Ev
// DEMANGLED: DB_ReportMannerlessUser_Upgrade::~DB_ReportMannerlessUser_Upgrade()
// ROLE: deleting destructor (D0)
DB_ReportMannerlessUser_Upgrade::~DB_ReportMannerlessUser_Upgrade() {}

// IDA_EA: 0x84276B0
// IDA_NAME: _ZN31DB_ReportMannerlessUser_Upgrade8dispatchEiiP6Stream
// DEMANGLED: DB_ReportMannerlessUser_Upgrade::dispatch(int,int,Stream *)
bool DB_ReportMannerlessUser_Upgrade::dispatch(int signal,
                                               int user_idx,
                                               Stream* stream) {
    (void)signal;
    (void)user_idx;

    Packet_Mannerless_User_Accusation* accusation =
        stream->GetOutBuffer<Packet_Mannerless_User_Accusation>();
    if (accusation == 0) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kMannerlessLogDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));

    // IDA @0x8427701 calls time(&timer) before composing the SQL.  The query
    // itself uses MySQL now(), but the old observable libc call is retained.
    time_t timer;
    time(&timer);

    char values[2048];
    char escaped_accused_name[59];
    memset(values, 0, sizeof(values));
    memset(escaped_accused_name, 0, sizeof(escaped_accused_name));
    db_handle->escape_string(escaped_accused_name, accusation->accused_name);

    if (Packet_Mannerless_User_Accusation::isNeedTypeForChattingMsg(
            accusation->accusation_type)) {
        char escaped_chat_message[511];
        memset(escaped_chat_message, 0, sizeof(escaped_chat_message));
        db_handle->escape_string(escaped_chat_message,
                                 accusation->chat_message);
        sprintf(values,
                "values(%d, %s, %d, '%s', %d, '%s', now(), '%s')",
                accusation->accusation_type,
                NumberToString(
                    static_cast<uint32_t>(accusation->reporter_acc_id), 0),
                accusation->reporter_server_group,
                accusation->reporter_name,
                accusation->accused_server_group,
                escaped_accused_name,
                escaped_chat_message);
    } else {
        sprintf(values,
                "values(%d, %s, %d, '%s', %d, '%s', now(), 'NULL')",
                accusation->accusation_type,
                NumberToString(
                    static_cast<uint32_t>(accusation->reporter_acc_id), 0),
                accusation->reporter_server_group,
                accusation->reporter_name,
                accusation->accused_server_group,
                escaped_accused_name);
    }

    db_handle->set_query(
        "inSert into charac_accusation(accusation_type, m_id, server_id, "
        "charac_name, server_id_accu, charac_name_accu, occ_time, "
        "chatting_msg) %s",
        values);
    return db_handle->exec(true) == true;
}
