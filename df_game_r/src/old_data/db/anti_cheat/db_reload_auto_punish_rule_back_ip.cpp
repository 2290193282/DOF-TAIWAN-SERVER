#include "old_data/db/anti_cheat/db_reload_auto_punish_rule_back_ip.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>



namespace {

enum {
    kDbTaskSignalReloadAutoPunishRuleBackIP = 264,
    kDispatchSignalAutoPunishRuleBlackIP = 264,
    kAutoPunishDbHandle = 6,
    kAutoPunishBlackIpLimit = 2000,
};

#pragma pack(push, 1)
struct SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY {
    uint32_t c_class_ip;    // +0
    uint8_t start_ip;       // +4
    uint8_t end_ip;         // +5
    uint8_t raw_06[2];      // +6
};

struct SIG_AUTO_PUNISH_RULE_BLACK_IP {
    uint16_t count;         // +0
    uint8_t raw_02[2];      // +2
    SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY entries[kAutoPunishBlackIpLimit];
};
#pragma pack(pop)

typedef char SIG_AUTO_PUNISH_RULE_BLACK_IP_Size_Must_Be_16004[
    (sizeof(SIG_AUTO_PUNISH_RULE_BLACK_IP) == 16004) ? 1 : -1];
typedef char SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY_Size_Must_Be_8[
    (sizeof(SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY) == 8) ? 1 : -1];

}  // namespace

// IDA_EA: 0x844EA12
// IDA_NAME: _ZN29DB_ReloadAutoPunishRuleBackIPC2Ev
// DEMANGLED: DB_ReloadAutoPunishRuleBackIP::DB_ReloadAutoPunishRuleBackIP(void)
DB_ReloadAutoPunishRuleBackIP::DB_ReloadAutoPunishRuleBackIP() {}

// IDA_EA: 0x845E104
// IDA_NAME: _ZN29DB_ReloadAutoPunishRuleBackIPD2Ev
// DEMANGLED: DB_ReloadAutoPunishRuleBackIP::~DB_ReloadAutoPunishRuleBackIP()
// IDA_EA: 0x845E134
// IDA_NAME: _ZN29DB_ReloadAutoPunishRuleBackIPD0Ev
// DEMANGLED: DB_ReloadAutoPunishRuleBackIP::~DB_ReloadAutoPunishRuleBackIP() deleting destructor
DB_ReloadAutoPunishRuleBackIP::~DB_ReloadAutoPunishRuleBackIP() {}

// IDA_EA: 0x842CDF6
// IDA_NAME: _ZN29DB_ReloadAutoPunishRuleBackIP8dispatchEiiP6Stream
// DEMANGLED: DB_ReloadAutoPunishRuleBackIP::dispatch(int,int,Stream *)
bool DB_ReloadAutoPunishRuleBackIP::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAutoPunishDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect ip,start_ip,end_ip from auto_punish_blackip_info where apply_flag=1 limit %d",
        kAutoPunishBlackIpLimit);
    if (db_handle->exec(true) != true) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 28761);
    CStreamGuard guard(out_stream, true);
    *guard << kDispatchSignalAutoPunishRuleBlackIP;
    *guard << -1;

    SIG_AUTO_PUNISH_RULE_BLACK_IP* payload =
        static_cast<SIG_AUTO_PUNISH_RULE_BLACK_IP*>(
            guard->GetInBufferUntyped(sizeof(SIG_AUTO_PUNISH_RULE_BLACK_IP)));
    if (!payload) {
        return false;
    }
    memset(payload, 0, sizeof(SIG_AUTO_PUNISH_RULE_BLACK_IP));

    const int row_count = db_handle->get_n_rows();
    payload->count = static_cast<uint16_t>(row_count);
    for (unsigned int row = 0; row < static_cast<unsigned int>(row_count);
         ++row) {
        if (db_handle->fetch() != true) {
            return false;
        }

        char ip_text[16];
        memset(ip_text, 0, sizeof(ip_text));
        unsigned int start_ip = 0;
        unsigned int end_ip = 0;
        if (db_handle->get_str(0, ip_text, 12) != true ||
            db_handle->get_uint(1, start_ip) != true ||
            db_handle->get_uint(2, end_ip) != true) {
            return false;
        }

        char c_class_text[16];
        memset(c_class_text, 0, sizeof(c_class_text));
        snprintf(c_class_text, sizeof(c_class_text), "%s.0", ip_text);

        payload->entries[row].c_class_ip = inet_addr(c_class_text);
        payload->entries[row].start_ip = static_cast<uint8_t>(start_ip);
        payload->entries[row].end_ip = static_cast<uint8_t>(end_ip);
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x842D0AE
// IDA_NAME: _ZN29DB_ReloadAutoPunishRuleBackIP11makeRequestEv
// DEMANGLED: DB_ReloadAutoPunishRuleBackIP::makeRequest(void)
void DB_ReloadAutoPunishRuleBackIP::makeRequest() {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28791);
    CStreamGuard guard(stream, true);

    // Old DBTask stores this dispatcher at this+265; active DBTask has a
    // leading queue_idx_ field, so the matching active DB_Q slot is 264.
    // The DB result sent to DISPATCH_Q still uses the old signal 8.
    *guard << kDbTaskSignalReloadAutoPunishRuleBackIP;
    *guard << -1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
