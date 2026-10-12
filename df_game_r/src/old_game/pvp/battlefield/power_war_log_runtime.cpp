#include "old_game/pvp/battlefield/power_war_runtime.h"

#include <cstring>

#include "infra/log/format/trace.h"

// IDA_EA: 0x82F082A
// IDA_NAME: _ZN12CPowerWarLog22IncAssaultAttemptCountEv
// DEMANGLED: CPowerWarLog::IncAssaultAttemptCount(void)
CPowerWarLog* CPowerWarLog::IncAssaultAttemptCount() {
    ++counters_[0];
    return this;
}

// IDA_EA: 0x82F083C
// IDA_NAME: _ZN12CPowerWarLog29IncAssaultAttemptSuccessCountEv
// DEMANGLED: CPowerWarLog::IncAssaultAttemptSuccessCount(void)
CPowerWarLog* CPowerWarLog::IncAssaultAttemptSuccessCount() {
    ++counters_[1];
    return this;
}

// IDA_EA: 0x82F0850
// IDA_NAME: _ZN12CPowerWarLog29IncAssaultAttemptFailureCountEv
// DEMANGLED: CPowerWarLog::IncAssaultAttemptFailureCount(void)
CPowerWarLog* CPowerWarLog::IncAssaultAttemptFailureCount() {
    ++counters_[2];
    return this;
}

// IDA_EA: 0x82F0864
// IDA_NAME: _ZN12CPowerWarLog28IncLoadingTimeOutCountSecedeEv
// DEMANGLED: CPowerWarLog::IncLoadingTimeOutCountSecede(void)
CPowerWarLog* CPowerWarLog::IncLoadingTimeOutCountSecede() {
    ++counters_[4];
    return this;
}

// IDA_EA: 0x82F0878
// IDA_NAME: _ZN12CPowerWarLog24IncP2PTimeOutCountSecedeEv
// DEMANGLED: CPowerWarLog::IncP2PTimeOutCountSecede(void)
CPowerWarLog* CPowerWarLog::IncP2PTimeOutCountSecede() {
    ++counters_[6];
    return this;
}

// IDA_EA: 0x82F088C
// IDA_NAME: _ZN12CPowerWarLog21IncP2PBadCountFailureEv
// DEMANGLED: CPowerWarLog::IncP2PBadCountFailure(void)
CPowerWarLog* CPowerWarLog::IncP2PBadCountFailure() {
    ++counters_[7];
    return this;
}

// IDA_EA: 0x82F08A0
// IDA_NAME: _ZN12CPowerWarLog20IncP2PBadCountSecedeEv
// DEMANGLED: CPowerWarLog::IncP2PBadCountSecede(void)
CPowerWarLog* CPowerWarLog::IncP2PBadCountSecede() {
    ++counters_[8];
    return this;
}

// IDA_EA: 0x82F08B4
// IDA_NAME: _ZN12CPowerWarLog20IncStartFailureCountEv
// DEMANGLED: CPowerWarLog::IncStartFailureCount(void)
CPowerWarLog* CPowerWarLog::IncStartFailureCount() {
    ++counters_[10];
    return this;
}

// IDA_EA: 0x82F08C8
// IDA_NAME: _ZN12CPowerWarLog20IncStartSuccessCountEv
// DEMANGLED: CPowerWarLog::IncStartSuccessCount(void)
CPowerWarLog* CPowerWarLog::IncStartSuccessCount() {
    ++counters_[9];
    return this;
}

// IDA_EA: 0x82F08DC
// IDA_NAME: _ZN12CPowerWarLog23IncTotalJoinAssaultUserEi
// DEMANGLED: CPowerWarLog::IncTotalJoinAssaultUser(int)
CPowerWarLog* CPowerWarLog::IncTotalJoinAssaultUser(int count) {
    counters_[13] += count;
    return this;
}

// IDA_EA: 0x82F08F2
// IDA_NAME: _ZN12CPowerWarLog24IncTotalLeaveAssaultUserEv
// DEMANGLED: CPowerWarLog::IncTotalLeaveAssaultUser(void)
CPowerWarLog* CPowerWarLog::IncTotalLeaveAssaultUser() {
    ++counters_[14];
    return this;
}

// IDA_EA: 0x848021E
// IDA_NAME: _ZN12CPowerWarLogC2Ev
// DEMANGLED: CPowerWarLog::CPowerWarLog(void)
CPowerWarLog::CPowerWarLog() {
    Reset();
}

// IDA_EA: 0x8480232
// IDA_NAME: _ZN12CPowerWarLog5ResetEv
// DEMANGLED: CPowerWarLog::Reset(void)
CPowerWarLog* CPowerWarLog::Reset() {
    std::memset(counters_, 0, sizeof(counters_));
    return this;
}

// IDA_EA: 0x84802CC
// IDA_NAME: _ZN12CPowerWarLog25PrintPowerWarAssultReportEv
// DEMANGLED: CPowerWarLog::PrintPowerWarAssultReport(void)
int CPowerWarLog::PrintPowerWarAssultReport() {
    const double attempt_failure_ratio = counters_[0]
        ? static_cast<double>(counters_[2]) / counters_[0] * 100.0
        : 0.0;
    const double failure_ratio = counters_[1]
        ? static_cast<double>(counters_[10]) / counters_[1] * 100.0
        : 0.0;
    cMyTrace trace("void CPowerWarLog::PrintPowerWarAssultReport()", 815, 0);
    trace("-------------------------------------------------------------------------------------------------------------");
    trace("|                                  POWAR WAR ASSAULT RESULT REPORT                                          |");
    trace("-------------------------------------------------------------------------------------------------------------");
    trace("1. POWER WAR ATTEMPT REPORT");
    trace("- POWER WAR ATTEMPT                : %d", counters_[0]);
    trace("- POWER WAR ATTEMPT SUCCESS        : %d", counters_[1]);
    trace("- POWER WAR ATTEMPT FAILURE        : %d", counters_[2]);
    trace("- POWER WAR ATTEMPT FAILURE RATIO  : %.2f%%", attempt_failure_ratio);
    trace("2. POWER WAR LOADING & P2P FAILURE REPORT");
    trace("- POWER WAR LOADING TIMEOUT        : %d", counters_[3]);
    trace("- POWER WAR P2P TIMEOUT            : %d", counters_[5]);
    trace("- POWER WAR P2P BAD                : %d", counters_[7]);
    trace("- POWER WAR FAILURE RATIO         : %.2f%%", failure_ratio);
    trace("3. POWER WAR START SECEDE REPORT");
    trace("- POWER WAR LOADING SECEDE        : %d", counters_[4]);
    trace("- POWER WAR P2P SECEDE            : %d", counters_[6]);
    trace("- POWER WAR P2P Bad               : %d", counters_[8]);
    trace("4. POWER WAR SUCCESS/FAILURE INFOMATION");
    trace("- SUCCESS COUNT                   : %d", counters_[11]);
    return trace("-------------------------------------------------------------------------------------------------------------");
}

// IDA_EA: 0x82F0906
// IDA_NAME: _ZN13CPowerManager14GetPowerWarLogEv
// DEMANGLED: CPowerManager::GetPowerWarLog(void)
CPowerWarLog* CPowerManager::GetPowerWarLog() {
    return &power_war_log_;
}
