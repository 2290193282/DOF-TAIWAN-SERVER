#include "infra/security/policy/hack_log_inter_receiver.h"

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/player/core/cuser.h"


// IDA_EA: 0x84D41AA
// IDA_NAME: _ZN20Inter_LoadDnfHackLog12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadDnfHackLog::dispatch_sig(CUser *,char *,int)
int Inter_LoadDnfHackLog::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)payload_size;

    const SIG_LOAD_DNF_HACK_LOG* const reply =
        reinterpret_cast<const SIG_LOAD_DNF_HACK_LOG*>(payload);
    if (user->get_acc_id() != static_cast<int>(reply->m_id)) {
        return 0;
    }

    // The original receiver overwrites all 3,900 daily counters, including
    // zero values, and deliberately obtains the analyzer for each iteration.
    for (int hack_type = 101; hack_type <= 4000; ++hack_type) {
        user->getHackAnalyzer()->setServerHackAccumulatedCnt(
            static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
            reply->counts_by_hack_type[hack_type - 101]);
    }
    return 0;
}

// IDA_EA: 0x84EDB7C
// IDA_NAME: _ZN20Inter_LoadDnfHackLogC2Ev
// DEMANGLED: Inter_LoadDnfHackLog::Inter_LoadDnfHackLog(void)
Inter_LoadDnfHackLog::Inter_LoadDnfHackLog() {
}

// IDA_EA: 0x84F8F52
// IDA_NAME: _ZN20Inter_LoadDnfHackLogD2Ev
// DEMANGLED: Inter_LoadDnfHackLog::~Inter_LoadDnfHackLog()
// IDA_EA: 0x84F8F82
// IDA_NAME: _ZN20Inter_LoadDnfHackLogD0Ev
// DEMANGLED: Inter_LoadDnfHackLog::~Inter_LoadDnfHackLog()
Inter_LoadDnfHackLog::~Inter_LoadDnfHackLog() {
}
