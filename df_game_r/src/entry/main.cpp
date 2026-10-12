#include "entry/app_entry.h"

#include "core/lifecycle/boot/app_lifecycle.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/log/format/clog.h"
#include "core/lifecycle/serve/process_files.h"
#include "core/lifecycle/shutdown/shutdown_manager.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

// IDA_EA: 0x829F839
// IDA_NAME: main
// DEMANGLED: main
int main(int argc, char** argv) {
    setCodePageByLocal();
    ShowLogo();
    Neof_startupGlobalInstances();

    const char init_ok = Init(argc, argv);
    if (init_ok) {
        MainLoop();
    }
    if (init_ok != 1) {
        return 1;
    }

    GetInstanceShutdowManager()->WaitUntilExitThread();
    if (CLog::instance() != 0) {
        CLog::instance()->logNotice("GAME SERVER SHUTDOWN COMPLETE");
    }

    RemoveOwnedPidFile();

    Neof_shutdownGlobalInstances();
    GlobalData::Fini();
    CLog::destroyInstance();
    std::puts("main() exited OK");
    return 0;
}
