#include "old_data/db/taiwan/inter_slots_331_353.h"

#include "infra/security/policy/black_ip_monitor.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"


// IDA_EA: 0x84D87E8
// IDA_NAME: _ZN28Inter_BlackIPMonitorDeleteIP12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_BlackIPMonitorDeleteIP::dispatch_sig(CUser *,char *,int)
int Inter_BlackIPMonitorDeleteIP::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_BLACK_IP_MONITOR_DELETE* const reply =
        reinterpret_cast<const SIG_BLACK_IP_MONITOR_DELETE*>(payload);
    cMyTrace trace(
        "virtual int Inter_BlackIPMonitorDeleteIP::dispatch_sig(CUser*, char*, int)",
        16237, 0);
    trace("Delete IP Cnt : %d", static_cast<int>(reply->count));

    for (uint32_t index = 0; reply->count > index; ++index) {
        const SIG_BLACK_IP_MONITOR_DELETE_ENTRY& entry = reply->entries[index];
        if (GlobalData::g_blackIPMonitor->deleteIP(entry.ip,
                                                   entry.punish_type)) {
            cMyTrace deleted_trace(
                "virtual int Inter_BlackIPMonitorDeleteIP::dispatch_sig(CUser*, char*, int)",
                16246, 0);
            deleted_trace("Delete IP: %s, Type: %d", entry.ip,
                          static_cast<int>(entry.punish_type));
        }
    }
    return 0;
}


// IDA_EA: 0x84EDFC0
// IDA_NAME: _ZN28Inter_BlackIPMonitorDeleteIPC2Ev
// DEMANGLED: Inter_BlackIPMonitorDeleteIP::Inter_BlackIPMonitorDeleteIP(void)
Inter_BlackIPMonitorDeleteIP::Inter_BlackIPMonitorDeleteIP() {}

// IDA_EA: 0x84F8370 (D2), 0x84F83A0 (D0)
Inter_BlackIPMonitorDeleteIP::~Inter_BlackIPMonitorDeleteIP() {}
