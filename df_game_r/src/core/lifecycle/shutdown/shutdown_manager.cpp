#include "core/lifecycle/shutdown/shutdown_manager.h"

#include "core/config/env/environment.h"
#include "core/lifecycle/serve/process_files.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/taiwan/taiwan_runtime_dispatch.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <signal.h>


static_assert(SIGUSR2 == 12,
              "recovered df_game_r shutdown signal must remain Linux SIGUSR2");


// IDA_EA: 0x82A6FA8
// IDA_NAME: _ZN15CShutdowManagerC2Ev
// DEMANGLED: CShutdowManager::CShutdowManager(void)
CShutdowManager::CShutdowManager() {
    Reset();
}

// IDA_EA: 0x82A35A6
// IDA_NAME: _ZN15CShutdowManager18SendShutdownSignalEv
// DEMANGLED: CShutdowManager::SendShutdownSignal(void)
int CShutdowManager::SendShutdownSignal() {
    pid_t pid = 0;
    char error_text[256] = {0};
    if (!ReadValidatedInstancePid(G_CEnvironment()->get_file_name(),
                                  &pid,
                                  error_text,
                                  sizeof(error_text))) {
        return std::printf("REFUSE SHUTDOWN SIGNAL: %s\n", error_text);
    }

    // CentOS 6 has no pidfd_send_signal.  Re-read and revalidate immediately
    // before kill to narrow the unavoidable PID-reuse window on this kernel.
    pid_t verified_pid = 0;
    if (!ReadValidatedInstancePid(G_CEnvironment()->get_file_name(),
                                  &verified_pid,
                                  error_text,
                                  sizeof(error_text))) {
        return std::printf("REFUSE SHUTDOWN SIGNAL: %s\n", error_text);
    }
    if (verified_pid != pid) {
        return std::printf(
            "REFUSE SHUTDOWN SIGNAL: PID changed from %d to %d\n",
            pid, verified_pid);
    }

    if (kill(pid, 12) >= 0) {
        return std::printf("SEND SHUTDOWN SIGNAL TO %d\n", pid);
    }
    return std::printf("%d kill error: %s\n", pid, std::strerror(errno));
}

// IDA_EA: 0x82A36BC
// IDA_NAME: _ZN15CShutdowManager15OnAllUserLogoutEv
// DEMANGLED: CShutdowManager::OnAllUserLogout(void)
void CShutdowManager::OnAllUserLogout() {
    if (shutdown_enabled_ != true) {
        return;
    }

    cMyTrace trace("void CShutdowManager::OnAllUserLogout()", 12859, 0);
    trace("ALL USER LOGOUT COMPLETE\n");
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("App.cpp", 12861), true);
    *guard << 63;
    *guard << -1;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    GlobalData::set_server_stop(true);
}

// IDA_EA: 0x82A37C8
// IDA_NAME: _ZN15CShutdowManager18SendLastMsgDBQueueEP5CUser
// DEMANGLED: CShutdowManager::SendLastMsgDBQueue(CUser *)
int CShutdowManager::SendLastMsgDBQueue(CUser* user) {
    (void)user;
    if (shutdown_enabled_ == true) {
        CStreamGuard db_guard(
            GlobalData::s_stream_pool->Acquire("App.cpp", 12876), true);
        *db_guard << 149;
        *db_guard << -1;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, db_guard);
        ++last_msg_count_;

        CStreamGuard log_db_guard(
            GlobalData::s_stream_pool->Acquire("App.cpp", 12887), true);
        *log_db_guard << 150;
        *log_db_guard << -1;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, log_db_guard);
        ++last_msg_count_;
    }
    return 1;
}

// IDA_EA: 0x82A396E
// IDA_NAME: _ZN15CShutdowManager19WaitUntilExitThreadEv
// DEMANGLED: CShutdowManager::WaitUntilExitThread(void)
int CShutdowManager::WaitUntilExitThread() {
    std::printf("WaitUntilExitThread (%d)\n", shutdown_enabled_ ? 1 : 0);
    if (shutdown_enabled_ == true) {
        // The original function is the pre-Neof thread-join boundary.  The
        // restored runtime owns those joins as one dependency-aware operation:
        // ingress -> Exchange -> q6/DB fixed point -> DB/LOG -> q6 -> Dispatch.
        // GlobalData::Fini retains the same call as an idempotent fallback.
        StopTaiwanDatabaseRuntime();
    }
    return 1;
}

// IDA_EA: 0x82A6F70
// IDA_NAME: _ZN15CShutdowManager14EnableShutdownEb
// DEMANGLED: CShutdowManager::EnableShutdown(bool)
CShutdowManager* CShutdowManager::EnableShutdown(bool enabled) {
    shutdown_enabled_ = enabled;
    return this;
}

// IDA_EA: 0x82A6F88
// IDA_NAME: _ZN15CShutdowManager5ResetEv
// DEMANGLED: CShutdowManager::Reset(void)
CShutdowManager* CShutdowManager::Reset() {
    shutdown_enabled_ = false;
    last_msg_count_ = 0;
    last_msg_response_count_ = 0;
    return this;
}

// IDA_EA: 0x84ED1EA
// IDA_NAME: _ZN15CShutdowManager29IncrementLastMsgResponseCountEv
// DEMANGLED: CShutdowManager::IncrementLastMsgResponseCount(void)
CShutdowManager* CShutdowManager::IncrementLastMsgResponseCount() {
    ++last_msg_response_count_;
    return this;
}

// IDA_EA: 0x84ED1FE
// IDA_NAME: _ZN15CShutdowManager15GetLastMsgCountEv
// DEMANGLED: CShutdowManager::GetLastMsgCount(void)
int CShutdowManager::GetLastMsgCount() const {
    return last_msg_count_;
}

// IDA_EA: 0x84ED20A
// IDA_NAME: _ZN15CShutdowManager23GetLastMsgResponseCountEv
// DEMANGLED: CShutdowManager::GetLastMsgResponseCount(void)
int CShutdowManager::GetLastMsgResponseCount() const {
    return last_msg_response_count_;
}

bool CShutdowManager::shutdown_enabled() const {
    return shutdown_enabled_;
}

// IDA_EA: 0x82A6FBB
// IDA_NAME: _Z25GetInstanceShutdowManagerv
// DEMANGLED: GetInstanceShutdowManager(void)
CShutdowManager* GetInstanceShutdowManager() {
    static CShutdowManager instance;
    return &instance;
}
