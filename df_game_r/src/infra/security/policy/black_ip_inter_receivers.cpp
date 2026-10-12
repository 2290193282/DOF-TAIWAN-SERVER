#include "infra/security/policy/black_ip_inter_receivers.h"

#include <arpa/inet.h>

#include "infra/security/policy/black_ip_monitor.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"


// IDA_EA: 0x84D8608
// IDA_NAME: _ZN23Inter_BlackIPMonitorReq12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_BlackIPMonitorReq::dispatch_sig(CUser *,char *,int)
int Inter_BlackIPMonitorReq::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;
    DB_BlackIPMonitor::makeRequest();
    cMyTrace trace(
        "virtual int Inter_BlackIPMonitorReq::dispatch_sig(CUser*, char*, int)",
        16168,
        0);
    trace("Get BlackIPMonitor Request");
    return 0;
}

// IDA_EA: 0x84EDF6C
// IDA_NAME: _ZN23Inter_BlackIPMonitorReqC2Ev
// DEMANGLED: Inter_BlackIPMonitorReq::Inter_BlackIPMonitorReq(void)
Inter_BlackIPMonitorReq::Inter_BlackIPMonitorReq() {
}

// IDA_EA: 0x84F845A
// IDA_NAME: _ZN23Inter_BlackIPMonitorReqD2Ev
// DEMANGLED: Inter_BlackIPMonitorReq::~Inter_BlackIPMonitorReq()
// IDA_EA: 0x84F848A
// IDA_NAME: _ZN23Inter_BlackIPMonitorReqD0Ev
// DEMANGLED: Inter_BlackIPMonitorReq::~Inter_BlackIPMonitorReq()
Inter_BlackIPMonitorReq::~Inter_BlackIPMonitorReq() {
}

// IDA_EA: 0x84D87A0
// IDA_NAME: _ZN31Inter_BlackIPMonitorPartLoadReq12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_BlackIPMonitorPartLoadReq::dispatch_sig(CUser *,char *,int)
int Inter_BlackIPMonitorPartLoadReq::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;
    DB_BlackIPMonitorPartLoad::makeRequest();
    cMyTrace trace(
        "virtual int Inter_BlackIPMonitorPartLoadReq::dispatch_sig(CUser*, char*, int)",
        16227,
        0);
    trace("Get BlackIPMonitor Request Part Load");
    return 0;
}

// IDA_EA: 0x84EDFA4
// IDA_NAME: _ZN31Inter_BlackIPMonitorPartLoadReqC2Ev
// DEMANGLED: Inter_BlackIPMonitorPartLoadReq::Inter_BlackIPMonitorPartLoadReq(void)
Inter_BlackIPMonitorPartLoadReq::Inter_BlackIPMonitorPartLoadReq() {
}

// IDA_EA: 0x84F83BE
// IDA_NAME: _ZN31Inter_BlackIPMonitorPartLoadReqD2Ev
// DEMANGLED: Inter_BlackIPMonitorPartLoadReq::~Inter_BlackIPMonitorPartLoadReq()
// IDA_EA: 0x84F83EE
// IDA_NAME: _ZN31Inter_BlackIPMonitorPartLoadReqD0Ev
// DEMANGLED: Inter_BlackIPMonitorPartLoadReq::~Inter_BlackIPMonitorPartLoadReq()
Inter_BlackIPMonitorPartLoadReq::~Inter_BlackIPMonitorPartLoadReq() {
}

// IDA_EA: 0x84D8650
// IDA_NAME: _ZN20Inter_BlackIPMonitor12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_BlackIPMonitor::dispatch_sig(CUser *,char *,int)
int Inter_BlackIPMonitor::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)user;
    (void)payload_size;

    SIG_IP_MONITOR_PUNISH* reply =
        reinterpret_cast<SIG_IP_MONITOR_PUNISH*>(payload);
    if (reply->initial_load != 0) {
        GlobalData::g_blackIPMonitor->reset();
    }
    for (uint32_t index = 0; index < reply->count; ++index) {
        const SIG_IP_MONITOR_PUNISH_ENTRY& entry = reply->entries[index];
        GlobalData::g_blackIPMonitor->insertNewIP(
            static_cast<uint32_t>(::inet_addr(entry.ip)),
            static_cast<WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE>(
                entry.punish_type),
            entry.member_id_count,
            static_cast<long>(entry.start_time),
            static_cast<long>(entry.end_time));
    }

    // The old protocol guarantees a non-empty reply.  Do not invent an
    // alternate empty-payload success/error path here.
    GlobalData::g_IPMonitorLastDBAccessTime =
        static_cast<time_t>(reply->entries[reply->count - 1].start_time);
    cMyTrace trace(
        "virtual int Inter_BlackIPMonitor::dispatch_sig(CUser*, char*, int)",
        16219,
        0);
    trace("IPMonitorPunish LAST LOAD TIME : %d",
          static_cast<int>(GlobalData::g_IPMonitorLastDBAccessTime));
    return 0;
}

// IDA_EA: 0x84EDF88
// IDA_NAME: _ZN20Inter_BlackIPMonitorC2Ev
// DEMANGLED: Inter_BlackIPMonitor::Inter_BlackIPMonitor(void)
Inter_BlackIPMonitor::Inter_BlackIPMonitor() {
}

// IDA_EA: 0x84F840C
// IDA_NAME: _ZN20Inter_BlackIPMonitorD2Ev
// DEMANGLED: Inter_BlackIPMonitor::~Inter_BlackIPMonitor()
// IDA_EA: 0x84F843C
// IDA_NAME: _ZN20Inter_BlackIPMonitorD0Ev
// DEMANGLED: Inter_BlackIPMonitor::~Inter_BlackIPMonitor()
Inter_BlackIPMonitor::~Inter_BlackIPMonitor() {
}
