#include "entry/app_entry.h"

#include "core/config/env/environment.h"
#include "core/lifecycle/serve/process_files.h"
#include "core/lifecycle/shutdown/shutdown_manager.h"
#include "common/types/code_page.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "infra/concurrent/timer/timer_daily_schedule.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/history_log_abi.h"
#include "common/manager/ipg_runtime.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/server_xml_runtime.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/clog.h"
#include "infra/log/format/trace.h"
#include "old_data/channel/legacy_runtime/channel_proxy.h"
#include "old_data/Login/garena/auth_garena_token.h"
#include "old_data/PVF/arad/arad_script_manager.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/taiwan/taiwan_runtime_dispatch.h"
#include "old_game/activity/schedule/event_table.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/dungeon/instance/timer_dungeon_statistic_entrance_hour_runtime.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "runtime/exchange_server/exchange_server_runtime.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/os_api.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <ctime>

namespace {

// A signal handler may run on any worker thread.  These are the only objects
// it touches: no logger, allocator, singleton, queue or pthread primitive is
// signal-safe.  MainLoop consumes the flags and performs the real transition.
volatile sig_atomic_t g_pending_suspend_signal = 0;
volatile sig_atomic_t g_pending_shutdown_signal = 0;
volatile sig_atomic_t g_pending_fatal_signal = 0;

// BeginProcessPidLifecycle() runs before the rest of Init.  This guard closes
// the daemon-parent handshake and removes only our exclusive PID inode on
// every later Init failure, without changing main's successful shutdown order.
class ProcessStartupGuard {
public:
    ProcessStartupGuard() : committed_(false) {}

    ~ProcessStartupGuard() {
        if (!committed_) {
            AbortProcessStartup();
        }
    }

    bool Commit() {
        if (!CommitProcessStartup()) {
            return false;
        }
        committed_ = true;
        return true;
    }

private:
    ProcessStartupGuard(const ProcessStartupGuard&);
    ProcessStartupGuard& operator=(const ProcessStartupGuard&);

    bool committed_;
};

bool IsFatalProcessSignal(int signal_number) {
    switch (signal_number) {
    case SIGSEGV:
    case SIGFPE:
    case SIGILL:
    case SIGBUS:
    case SIGSTKFLT:
    case SIGURG:
    case SIGXCPU:
    case SIGXFSZ:
    case SIGSYS:
        return true;
    default:
        return false;
    }
}

void ArmShutdownManager() {
    CShutdowManager* const shutdown_manager =
        GetInstanceShutdowManager();
    if (!shutdown_manager->shutdown_enabled()) {
        // Repeated SIGUSR2/SIGTERM must not reset the final DB-message
        // counters after logout has started.
        shutdown_manager->Reset();
        shutdown_manager->EnableShutdown(true);
    }
}

void BeginGracefulShutdownFromMainThread(int signal_number) {
    std::fprintf(stderr, "RECV graceful shutdown signal %d\n", signal_number);
    G_CEnvironment()->set_suspend_state(true);

    CShutdowManager* const shutdown_manager =
        GetInstanceShutdowManager();
    if (shutdown_manager->shutdown_enabled()) {
        return;
    }
    ArmShutdownManager();

    GameWorld* const game_world = G_GameWorld();
    if (!game_world || game_world->get_UserCount_InWorld() <= 0) {
        shutdown_manager->OnAllUserLogout();
        return;
    }
    game_world->arrange_users();
}

// The old launcher passes the instance name and operation as positional
// arguments (for example: `df_game_r siroco11 start`).  check_argv in the
// original binary writes these values before logging or loading any global
// subsystem, so the instance-specific cfg file is selected at startup.
bool LoadStartArguments(int argc, char** argv) {
    if (argc < 3 || argv == 0 || argv[1] == 0 || argv[1][0] == '\0' ||
        argv[2] == 0 || argv[2][0] == '\0') {
        std::fprintf(stderr,
                     "usage: %s <instance> "
                     "[start|start_all|nofork|stop|test]\n",
                     (argc > 0 && argv != 0 && argv[0] != 0) ? argv[0]
                                                              : "df_game_r");
        return false;
    }

    // CEnvironment::set_file_name is the recovered fixed-width strcpy
    // boundary.  Validate argv[1] before obtaining/mutating the environment,
    // including control commands such as stop that never call Begin below.
    if (!IsSafeProcessInstanceName(argv[1])) {
        std::fprintf(stderr, "invalid df_game_r instance name\n");
        return false;
    }

    CEnvironment* const environment = G_CEnvironment();
    environment->set_file_name(argv[1]);
    const bool stop_mode = std::strcmp(argv[2], "stop") == 0;
    if (argc >= 3 && argv[2] != 0) {
        if (std::strcmp(argv[2], "start") == 0) {
            environment->set_operation_type(START_ARGUMENT_START);
        } else if (std::strcmp(argv[2], "start_all") == 0) {
            environment->set_operation_type(START_ARGUMENT_START_ALL);
        } else if (std::strcmp(argv[2], "nofork") == 0) {
            environment->set_operation_type(START_ARGUMENT_NOFORK);
        } else if (stop_mode) {
            // The original check_argv loads the named instance first, then
            // uses its PID file to signal the already-running server.  stop
            // is a control action, not a server operation type.
        } else if (std::strcmp(argv[2], "test") == 0) {
            environment->set_operation_type(START_ARGUMENT_TEST);
        } else {
            std::fprintf(stderr, "unknown start operation: %s\n", argv[2]);
            return false;
        }
    }

    if (!environment->load()) {
        std::fprintf(stderr, "failed to load ./cfg/%s.cfg\n",
                     environment->get_file_name());
        return false;
    }
    if (stop_mode) {
        GetInstanceShutdowManager()->SendShutdownSignal();
        return false;
    }
    if (!BeginProcessPidLifecycle(argv[2], environment->get_file_name())) {
        return false;
    }
    return true;
}

template <typename Value, int Capacity>
int GetStaticPoolUsedCount(const StaticPool<Value, Capacity>& pool) {
    return Capacity - static_cast<int>(pool.free_nodes.size());
}

template <typename Value, int Capacity>
int GetStaticPoolCapacity(const StaticPool<Value, Capacity>&) {
    return Capacity;
}

void SendMonitorHeartbeats(CServerProxyMgr<CMonitorServerProxy>* manager,
                           int channel_no) {
    int cursor = 0;
    for (int server_group = manager->GetStartIndex();;
         server_group = manager->GetNextIndex(cursor)) {
        if (manager->GetEndIndex() <= cursor) {
            break;
        }
        manager->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(server_group))
            ->SendHeartBeat(channel_no);
    }
}

void SendGuildHeartbeats(CServerProxyMgr<CGuildServerProxy>* manager,
                         int channel_no) {
    int cursor = 0;
    for (int server_group = manager->GetStartIndex();;
         server_group = manager->GetNextIndex(cursor)) {
        if (manager->GetEndIndex() <= cursor) {
            break;
        }
        manager->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(server_group))
            ->SendHeartBeat(channel_no);
    }
}

// IDA_EA: Init @0x82A08F4, standard non-PvP branch at 0x82A0B5C..
// 0x82A0D1E.  The original installs exactly one default endpoint for each
// proxy manager when this is a conventional channel such as siroco11.
bool ConfigureServerProxies() {
    GameWorld* const world = G_GameWorld();
    CEnvironment* const environment = G_CEnvironment();
    if (!world || !environment) {
        return false;
    }
    if (world->GetChannelType() == 7) {
        return true;
    }
    if (world->IsIntegratedPvPBaseChannel()) {
        TournamentConfigMap& tournament_configs =
            environment->tournament_config_map();
        const int server_count =
            static_cast<int>(tournament_configs.size());
        int server_index = 0;
        for (TournamentConfigMap::iterator config = tournament_configs.begin();
             config != tournament_configs.end(); ++config, ++server_index) {
            ConfigTagMap& values = config->second;
            if (GlobalData::s_monitor_proxy_mgr->createServerProxy(
                    server_count,
                    server_index,
                    config->first,
                    const_cast<char*>(GetConfigFileString(
                        values, "udp_ip_of_monitor")),
                    std::atoi(GetConfigFileString(
                        values, "udp_port_of_monitor")),
                    std::atoi(GetConfigFileString(
                        values, "tcp_port_of_monitor"))) != 1) {
                std::cerr
                    << "[s_monitor_proxy_mgr::Init] Fail Init (monitor server proxy)"
                    << std::endl;
                return false;
            }
            if (GlobalData::s_guild_proxy_mgr->createServerProxy(
                    server_count,
                    server_index,
                    config->first,
                    const_cast<char*>(GetConfigFileString(
                        values, "udp_ip_of_guild")),
                    std::atoi(GetConfigFileString(
                        values, "udp_port_of_guild")),
                    std::atoi(GetConfigFileString(
                        values, "tcp_port_of_guild"))) != 1) {
                std::cerr
                    << "[s_guild_proxy_mgr::Init] Fail Init (guild server proxy)"
                    << std::endl;
                return false;
            }
            if (GlobalData::s_statistic_proxy_mgr->createServerProxy(
                    server_count,
                    server_index,
                    config->first,
                    const_cast<char*>(GetConfigFileString(
                        values, "udp_ip_of_statistic")),
                    std::atoi(GetConfigFileString(
                        values, "udp_port_of_statistic")),
                    0) != 1) {
                std::cerr
                    << "[s_statistic_proxy_mgr::Init] Fail Init (statistic server proxy)"
                    << std::endl;
                return false;
            }
        }
        return true;
    }

    if (GlobalData::s_monitor_proxy_mgr->createServerProxy(
            1,
            0,
            SERVER_GROUP_DEFAULT,
            const_cast<char*>(environment->get_udp_ip_monitor()),
            environment->get_udp_port_monitor(),
            environment->get_tcp_port_monitor()) != 1) {
        std::cerr << "[s_monitor_proxy_mgr::Init] Fail Init (monitor server proxy)"
                  << std::endl;
        return false;
    }
    if (GlobalData::s_guild_proxy_mgr->createServerProxy(
            1,
            0,
            SERVER_GROUP_DEFAULT,
            const_cast<char*>(environment->get_udp_ip_guild()),
            environment->get_udp_port_guild(),
            environment->get_tcp_port_guild()) != 1) {
        std::cerr << "[s_guild_proxy_mgr::Init] Fail Init (guild server proxy)"
                  << std::endl;
        return false;
    }
    if (GlobalData::s_statistic_proxy_mgr->createServerProxy(
            1,
            0,
            SERVER_GROUP_DEFAULT,
            const_cast<char*>(environment->get_udp_ip_statistic()),
            environment->get_udp_port_statistic(),
            0) != 1) {
        std::cerr << "[s_statistic_proxy_mgr::Init] Fail Init (statistic server proxy)"
                  << std::endl;
        return false;
    }
    return true;
}

}  // namespace


// IDA_EA: 0x829F1FC
// IDA_NAME: _Z13regist_signaliPFviE
// DEMANGLED: regist_signal(int,void (*)(int))
bool regist_signal(int signal_number, void (*handler)(int)) {
    struct sigaction action;
    std::memset(&action, 0, sizeof(action));
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    // The legacy binary dumped/exited from inside synchronous fatal handlers.
    // That was not async-signal-safe.  SA_RESETHAND preserves a deterministic
    // fatal fallback: a real fault records the flag, returns once, then the
    // repeated fault takes the kernel default/core path instead of spinning.
    if (handler == signal_handler && IsFatalProcessSignal(signal_number)) {
        action.sa_flags |= SA_RESETHAND;
    }
    return sigaction(signal_number, &action, 0) == 0;
}

// IDA_EA: 0x829F28B
// IDA_NAME: _Z14signal_handleri
// DEMANGLED: signal_handler(int)
void signal_handler(int signal_number) {
    if (signal_number == SIGUSR1) {
        g_pending_suspend_signal = signal_number;
    } else if (signal_number == SIGUSR2 || signal_number == SIGTERM) {
        g_pending_shutdown_signal = signal_number;
    } else {
        g_pending_fatal_signal = signal_number;
    }
}

// IDA_EA: 0x829F4C5
// IDA_NAME: _Z14regist_signalsv
// DEMANGLED: regist_signals(void)
bool regist_signals() {
    g_pending_suspend_signal = 0;
    g_pending_shutdown_signal = 0;
    g_pending_fatal_signal = 0;

    // Exact old registration order recovered from five matching df_game_r
    // candidates.  SIGPIPE remains ignored; signal 12 is SIGUSR2 and is the
    // target used by CShutdowManager::SendShutdownSignal.
    return regist_signal(SIGTERM, signal_handler) &&
           regist_signal(SIGUSR1, signal_handler) &&
           regist_signal(SIGUSR2, signal_handler) &&
           regist_signal(SIGPIPE, SIG_IGN) &&
           regist_signal(SIGSEGV, signal_handler) &&
           regist_signal(SIGFPE, signal_handler) &&
           regist_signal(SIGILL, signal_handler) &&
           regist_signal(SIGBUS, signal_handler) &&
           regist_signal(SIGSTKFLT, signal_handler) &&
           regist_signal(SIGURG, signal_handler) &&
           regist_signal(SIGXCPU, signal_handler) &&
           regist_signal(SIGXFSZ, signal_handler) &&
           regist_signal(SIGSYS, signal_handler);
}

void ProcessPendingOSSignals() {
    const sig_atomic_t fatal_signal = g_pending_fatal_signal;
    if (fatal_signal != 0) {
        g_pending_fatal_signal = 0;
        std::fprintf(stderr, "RECV fatal process signal %d\n",
                     static_cast<int>(fatal_signal));
        // Fatal signals do not attempt user-state work, matching the old
        // abnormal exit class, but they still arm WaitUntilExitThread so the
        // restored worker DAG cannot be skipped on an asynchronous signal.
        ArmShutdownManager();
        GlobalData::set_server_stop(true);
        return;
    }

    const sig_atomic_t suspend_signal = g_pending_suspend_signal;
    if (suspend_signal != 0) {
        g_pending_suspend_signal = 0;
        G_CEnvironment()->set_suspend_state(true);
    }

    const sig_atomic_t shutdown_signal = g_pending_shutdown_signal;
    if (shutdown_signal != 0) {
        g_pending_shutdown_signal = 0;
        BeginGracefulShutdownFromMainThread(
            static_cast<int>(shutdown_signal));
    }
}


// IDA_EA: 0x82A08F4
// IDA_NAME: _Z4InitiPPc
// DEMANGLED: Init(int,char **)
char Init(int argc, char** argv) {
    // IDA Init @0x82A08F4 calls this before getrlimit/check_argv and refuses
    // startup if any original process-signal mapping cannot be installed.
    if (!regist_signals()) {
        std::cerr
            << "Fail to regist os signals, Server process is terminated."
            << std::endl;
        return 0;
    }

    struct rlimit rlimits;
    getrlimit(4, &rlimits);
    rlimits.rlim_cur = static_cast<rlim_t>(-1);
    setrlimit(4, &rlimits);

    std::cerr << "[!] Check Args ..." << std::endl;
    if (!LoadStartArguments(argc, argv)) {
        return 0;
    }
    ProcessStartupGuard startup_guard;
    std::cerr << "[!] Check Args Success" << std::endl;

    std::srand(static_cast<unsigned int>(std::time(0)));

    CLog::createInstance();
    std::cerr << "[!] Init Log ..." << std::endl;
    char log_path[200];
    std::memset(log_path, 0, sizeof(log_path));
    std::sprintf(log_path, "./log/%s/", G_CEnvironment()->get_file_name());
    CLog::instance()->initiate(log_path, 0, 0);
    std::cerr << "\t- log path: " << log_path << std::endl;
    std::cerr << "[!] Init Log Success" << std::endl;

    CLog::instance()->logNotice("[!] Init Taiwan Garena Auth Key ...");
    const char* garena_publickey = G_CEnvironment()->get_garena_publickey();
    if (Taiwan::AuthGarenaToken::init(garena_publickey) != 1) {
        CLog::instance()->logError(" %s Fail",
                                   garena_publickey != 0 ? garena_publickey : "");
        return 0;
    }
    CLog::instance()->logNotice(" %s Success",
                                garena_publickey != 0 ? garena_publickey : "");

    CodePage::initCodePage();

    CLog::instance()->logNotice("[!] Init GlobalData ...");
    if (!GlobalData::Init()) {
        CLog::instance()->logError("[!] Init GlobalData Fail");
        return 0;
    }
    CLog::instance()->logNotice("[!] Init GlobalData Success");

    CLog::instance()->logNotice("[!] Init HistoryLog");
    if (HistoryLog::Init() != 1) {
        CLog::instance()->logError("Fail");
        return 0;
    }
    CLog::instance()->logNotice(" Success");

    // IDA: Init @0x82A08F4 initializes the process-global DataManager before
    // the server-string and packet-cipher stages.  Restored importers now use
    // a fail-closed return contract, so do not continue with partial script
    // registries (which would later be misreported as InitChannelType Fail).
    CLog::instance()->logNotice("[!] Init DataManager ...");
    if (G_CDataManager()->init() != 1) {
        CLog::instance()->logError("[!] Init DataManager Fail");
        return 0;
    }
    CLog::instance()->logNotice("[!] Init DataManager Success");

    CLog::instance()->logNotice("[!] Init GameManager ...");
    if (!G_CGameManager()->init()) {
        CLog::instance()->logError("[!] Init GameManager Fail");
        return 0;
    }
    CLog::instance()->logNotice("[!] Init GameManager Success");

    CLog::instance()->logNotice("[!] Init Arad GameScriptManager ...");
    if (!ARAD::ScriptManager::Get()->LoadScript()) {
        CLog::instance()->logError("[!] Init Arad GameScriptManager Fail");
        return 0;
    }
    CLog::instance()->logNotice("[!] Init Arad GameScriptManager Success");
    G_CGameManager()->GetQuickPartySystemManager();

    CLog::instance()->logNotice("[!] Init ChannelType ");
    if (!G_GameWorld()->InitChannelType()) {
        CLog::instance()->logError("[!] Init ChannelType Fail");
        return 0;
    }
    CLog::instance()->logNotice("Success");
    CLog::instance()->logNotice("\t- Channel Type : %d",
                                G_GameWorld()->GetChannelType());

    CLog::instance()->logNotice("[!] Init tournament db ");
    if (!GlobalData::initTournamentDB(G_CEnvironment()->get_db_tbl_file())) {
        CLog::instance()->logError("[!] Init tournament db Fail");
        return 0;
    }
    CLog::instance()->logNotice("Success");

    MySQL* const game_db = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2), SERVER_GROUP_DEFAULT);
    if (!game_db) {
        return 0;
    }
    GlobalData::s_db_mgr->SetQueryCounterPointer(game_db->GetQueryCounter());

    if (!ConfigureServerProxies()) {
        return 0;
    }

    CLog::instance()->logNotice("[!] Init GameWorld ");
    if (G_GameWorld()->init() != 1) {
        CLog::instance()->logError("[!] Init GameWorld Fail");
        return 0;
    }
    CLog::instance()->logNotice("Success");

    // IDA Init @0x82A08F4 (generated app.cpp line 866): the original
    // calculates every imported schedule's
    // initial state immediately after GameWorld::init.  The per-user login
    // sender deliberately transmits only state-1 entries, so omitting this
    // call suppresses the whole CMD493 login sequence.
    GlobalData::s_DailyScheduleManager->CheckAndSendToAllUserSchedule();

    CLog::instance()->logNotice("[!] Init Login State ");
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        int proxy_index = 0;
        for (int server_group =
                 GlobalData::s_statistic_proxy_mgr->GetStartIndex();;
             server_group =
                 GlobalData::s_statistic_proxy_mgr->GetNextIndex(proxy_index)) {
            if (GlobalData::s_statistic_proxy_mgr->GetEndIndex() <=
                proxy_index) {
                break;
            }
            if (G_CDataManager()->InitLoginState(
                    static_cast<ENUM_SERVER_GROUP>(server_group),
                    G_CEnvironment()->get_gc_no_hardcode(server_group)) != 1) {
                CLog::instance()->logError("[!] Init Login State Fail");
                return 0;
            }
        }
    } else if (G_CDataManager()->InitLoginState(
                   static_cast<ENUM_SERVER_GROUP>(
                       G_CEnvironment()->get_server_group()),
                   G_CEnvironment()->get_gc_no()) != 1) {
        CLog::instance()->logError("[!] Init Login State Fail");
        return 0;
    }
    CLog::instance()->logNotice("Success");

    // IDA_EA: Init @0x82A1B84..0x82A1BA5.  The completed login-state
    // initialization is the prerequisite for the non-light-server WarArea
    // allocation branch.  Do not move this behind the event-table loop: the
    // original creates all configured WarRooms before it enters that loop.
    if (!IsLightServer()) {
        G_CGameManager()->WarRoomCountAdjustByChannelInfo();
        G_CGameManager()->WarRoomAlloc();
    }

    // IDA_EA: Init @0x82A1BAA..0x82A1BB2.  Blue Marble allocation is
    // unconditional and occurs after the light-server-gated WarRoom branch,
    // before EventTable verification.
    G_CGameManager()->allocBlueMarble();

    // IDA_EA: Init @0x82A1BC0..0x82A1CF7.  The original does not call
    // CEventManager::InitEventManager here: it verifies the static
    // CEventTable metadata, inserts any missing database rows, transfers the
    // table's heap-owned objects to CEventManager, then loads active events.
    CLog::instance()->logNotice("[!] Init Event Table ");
    std::vector<int> missing_event_indexes;
    if (!CEventTable::VerifyEventTable(missing_event_indexes)) {
        CLog::instance()->logError("Fail verify Event Table");
        return 0;
    }
    if (!CEventTable::InsertEventTable(missing_event_indexes)) {
        CLog::instance()->logError("Fail insert Event Table");
        return 0;
    }
    for (int event_index = 0;
         event_index < CEventTable::GetEventTableCount();
         ++event_index) {
        GlobalData::s_event_manager->AddEvent(
            CEventTable::GetEventId(event_index),
            CEventTable::GetEventBase(event_index));
    }
    if (!GlobalData::s_event_manager->LoadEventFromDB()) {
        CLog::instance()->logError("Fail Load Event From DB");
        return 0;
    }
    CLog::instance()->logNotice("Success");

    // The server XML object owns its parsed maps and is deliberately loaded
    // after world/login initialization, matching the old app entrypoint.
    LoadServerStringXml();

    // IDA: Init @0x82A08F4 calls InitCipherKeyString() as an initialization
    // step near the end of startup and does not use its return value as a
    // boolean gate. The key-string maps are restored in their own data module;
    // this boundary must not stop the bootstrap flow by itself.
    InitCipherKeyString();
    RegistServerSchedule();
    // IDA Init @0x82A08F4 (generated app.cpp line 993) invokes the dedicated
    // daily-schedule timer after RegistServerSchedule.  Its callback owns all
    // subsequent 60-second runs.
    Timer_DailySchedule::RegistNextTimer();
    // IDA Init @0x82A08F4 (generated app.cpp line 994) starts the altar's
    // one-second message-166 chain between the daily timers and thread start.
    advancealtar::Timer_MainTick::RegistNextTimerGage();
    // Old Init @0x82A08F4 schedules this after the server schedules and
    // before worker-thread creation. The callback owns its hourly repeat.
    Timer_DungeonStatisticEntrance_Hour::RegistNextTimer();

    if (!StartTaiwanDatabaseRuntime()) {
        CLog::instance()->logError("Fail to Create Threads.");
        return 0;
    }
    // IDA create_thread @0x829FD30: ordinary game channels start the
    // Exchange Server after the network worker, then bind its exact packet
    // pool owner slot (THREAD_EXCHANGE == 3).  This thread is also the sole
    // consumer of MsgQueueMgr queue 5 used by repeat character selection.
    if (!G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        G_GameWorld()->GetChannelType() != 7) {
        if (exchange_server::BeginExchangeServerThread() != 1 ||
            !GlobalData::setCurrentPacketPool(
                static_cast<THREAD_TYPE>(GlobalData::THREAD_EXCHANGE),
                exchange_server::getExchangeServerThreadID())) {
            CLog::instance()->logError("Fail to Create Exchange Server Thread.");
            // Init failure bypasses GlobalData::Fini in main().  Run the full
            // ingress -> Exchange -> joint DB closure -> DB/LOG -> queue6 ->
            // Dispatch join DAG here so no worker can outlive its queue or
            // the global dispatcher graph used by final acknowledgements.
            StopTaiwanDatabaseRuntime();
            return 0;
        }
    }
    CLog::instance()->logNotice(
        "[!] The %s channel %d('%s') server has been started successfully.",
        G_CEnvironment()->get_file_name(),
        G_CEnvironment()->get_channel_no(),
        G_CEnvironment()->get_channel_name());

    if (!startup_guard.Commit()) {
        CLog::instance()->logError(
            "Fail to commit process PID/startup handshake.");
        StopTaiwanDatabaseRuntime();
        return 0;
    }

    return 1;
}

// IDA_EA: 0x829FA2B
// IDA_NAME: _Z8MainLoopv
// DEMANGLED: MainLoop(void)
int MainLoop() {
    int heartbeat_ticks = 0;
    int channel_user_ticks = 0;
    int previous_world_user_count = 0;
    int previous_user_pool_count = 0;
    int channel_info_toggle = 0;
    int dispatch_timer_ticks = 0;

    while (!GlobalData::server_stop()) {
        ProcessPendingOSSignals();
        if (GlobalData::server_stop()) {
            break;
        }

        // Process-lifetime matching maintenance must run even on an idle
        // server with no packet dispatches; the hook is O(active cohorts).
        channel_zombie_matching_authority::Tick();

        if (channel_user_ticks == 30) {
            GameWorld* const game_world = G_GameWorld();
            game_world->UpdateChannelUser();

            const int world_user_count = game_world->get_UserCount_InWorld();
            CGameManager* const game_manager = G_CGameManager();
            const int user_pool_count =
                GetStaticPoolUsedCount(game_manager->user_pool_);
            if (previous_world_user_count != world_user_count ||
                previous_user_pool_count != user_pool_count) {
                ProcessServerStatusLog();
                previous_world_user_count = world_user_count;
                previous_user_pool_count = user_pool_count;
            }
            channel_user_ticks = 0;
        }

        if (heartbeat_ticks++ > 9) {
            const int channel_no = G_CEnvironment()->get_channel_no();
            SendMonitorHeartbeats(GlobalData::s_monitor_proxy_mgr, channel_no);
            SendGuildHeartbeats(GlobalData::s_guild_proxy_mgr, channel_no);
            heartbeat_ticks = 0;
        }

        if (channel_info_toggle == 1) {
            GlobalData::s_channel_proxy->SendChannelInfo();
            channel_info_toggle = 0;
        } else {
            channel_info_toggle = 1;
        }

        if (dispatch_timer_ticks == 300) {
            CStreamGuard stream_guard(
                GlobalData::small_stream_pool()->Acquire("App.cpp", 10024),
                true);
            // IDA 0x829FC7B--0x829FCA1: both fields are written through
            // CStreamGuard::operator<<(int).  The first immediate is 0x30a
            // (Inter_UpdateServerSnapShot), not the decompiler's narrowed
            // low-byte value 10 (Inter_LoadQuest).
            stream_guard << 0x30A;
            stream_guard << -1;
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q,
                                        stream_guard);
            dispatch_timer_ticks = 0;
        } else {
            ++dispatch_timer_ticks;
        }

        sleep_ext(1, 0);
        ++channel_user_ticks;
        ProcessAntiBotInputWrapper();
    }
    return 1;
}

// IDA_EA: 0x80EDDB0
// IDA_NAME: _Z26ProcessAntiBotInputWrapperv
// DEMANGLED: ProcessAntiBotInputWrapper(void)
int ProcessAntiBotInputWrapper() {
    timeval started;
    timeval completed;
    gettimeofday(&started, 0);
    AntibotCheckerProcessInput(GlobalData::antibot_checker());
    gettimeofday(&completed, 0);

    const int elapsed_milliseconds =
        static_cast<int>(1000 * (completed.tv_sec - started.tv_sec) +
                         (completed.tv_usec - started.tv_usec) / 1000);
    if (elapsed_milliseconds > 500) {
        LogManager::logFormat(
            1,
            "localchina/localchina_App.cpp",
            "void ProcessAntiBotInputWrapper()",
            0x2A,
            "Delay notice, ProcessAntiBotInput : %d, %ld, %ld, %ld, %ld",
            elapsed_milliseconds,
            static_cast<long>(started.tv_sec),
            static_cast<long>(started.tv_usec),
            static_cast<long>(completed.tv_sec),
            static_cast<long>(completed.tv_usec));
    }
    return elapsed_milliseconds;
}

// IDA_EA: 0x80EDE91
// IDA_NAME: _Z22ProcessServerStatusLogv
// DEMANGLED: ProcessServerStatusLog(void)
int ProcessServerStatusLog() {
    GameWorld* const game_world = G_GameWorld();
    CGameManager* const game_manager = G_CGameManager();

    const int world_user_count = game_world->get_UserCount_InWorld();
    const int user_pool_capacity =
        GetStaticPoolCapacity(game_manager->user_pool_);
    const int user_pool_used = GetStaticPoolUsedCount(game_manager->user_pool_);
    const int party_pool_capacity =
        GetStaticPoolCapacity(game_manager->party_pool_);
    const int party_pool_used =
        GetStaticPoolUsedCount(game_manager->party_pool_);
    const int war_room_pool_capacity =
        GetStaticPoolCapacity(game_manager->war_room_pool_);
    const int war_room_pool_used =
        GetStaticPoolUsedCount(game_manager->war_room_pool_);
    const int death_tower_pool_capacity =
        GetStaticPoolCapacity(game_manager->death_tower_pool_);
    const int death_tower_pool_used =
        GetStaticPoolUsedCount(game_manager->death_tower_pool_);
    const int pvp_pool_capacity = GetStaticPoolCapacity(game_manager->pvp_pool_);
    const int pvp_pool_used = GetStaticPoolUsedCount(game_manager->pvp_pool_);
    const int trade_pool_capacity =
        GetStaticPoolCapacity(game_manager->trade_space_pool_);
    const int trade_pool_used =
        GetStaticPoolUsedCount(game_manager->trade_space_pool_);

    const int dispatcher_queue_count =
        GlobalData::msgq_mgr()->remain(MsgQueueMgr::DISPATCH_Q);
    const int db_queue_count =
        GlobalData::msgq_mgr()->remain(MsgQueueMgr::DB_Q);
    const int stream_pool_remain = GlobalData::stream_pool()->Remain();
    const int stream_pool_size = GlobalData::stream_pool()->Size();
    const int small_stream_pool_remain = GlobalData::small_stream_pool()->Remain();
    const int small_stream_pool_size = GlobalData::small_stream_pool()->Size();
    const int big_stream_pool_remain = GlobalData::big_stream_pool()->Remain();
    const int big_stream_pool_size = GlobalData::big_stream_pool()->Size();

    WongWork::IPG::stIPGStatus_t ipg_status;
    GlobalData::s_pIPGHelper->getIPGStatus(ipg_status);

    const int avatar_pool_remain =
        WongWork::CAvatarItemMgr::m_AvatarItemPool.Remain();
    const int avatar_pool_size = WongWork::CAvatarItemMgr::m_AvatarItemPool.Size();
    const int creature_pool_size = user_creature::GetCreaturePoolSize();
    const int creature_pool_remain = user_creature::GetCreaturePoolRemainSize();
    const int egg_pool_size = user_creature::GetCreatureEggPoolSize();
    const int egg_pool_remain = user_creature::GetCreatureEggPoolRemainSize();

    cMyTrace trace("void ProcessServerStatusLog()", 92, 0);
    trace(
        "\n"
        "[DISPATCHER_Q (%3d) DB_Q (%3d) IPGQueue_Size (%3d)]\n"
        "[Small_Pool (%3d/%3d) Default_Pool (%3d/%3d) Big_Pool (%3d/%3d) IPGPool_Size(%3d/%3d)]\n"
        "[AvatarPool_Size(%3d/%3d), CreaturePoolSize(%3d/%3d), CreatureEggPoolSize(%3d/%3d)]\n"
        "[Sessions (%3d/%3d) Users (%3d/%3d) Parties (%3d/%3d) WarRooms (%3d/%3d) DeathTowers (%3d/%3d) PVPs (%3d/%3d) Trades (%3d/%3d)]",
        dispatcher_queue_count,
        db_queue_count,
        ipg_status.request_count,
        small_stream_pool_remain,
        small_stream_pool_size,
        stream_pool_remain,
        stream_pool_size,
        big_stream_pool_remain,
        big_stream_pool_size,
        ipg_status.remain,
        ipg_status.size,
        avatar_pool_remain,
        avatar_pool_size,
        creature_pool_remain,
        creature_pool_size,
        egg_pool_remain,
        egg_pool_size,
        world_user_count,
        user_pool_capacity,
        user_pool_used,
        user_pool_capacity,
        party_pool_used,
        party_pool_capacity,
        war_room_pool_used,
        war_room_pool_capacity,
        death_tower_pool_used,
        death_tower_pool_capacity,
        pvp_pool_used,
        pvp_pool_capacity,
        trade_pool_used,
        trade_pool_capacity);

    if (world_user_count < party_pool_used) {
        LogManager::logFormat(
            1,
            "localchina/localchina_App.cpp",
            "void ProcessServerStatusLog()",
            0x77,
            "[SESSION ERROR] CurrentUserCount(%d) < CurrentPartyCount(%d)",
            world_user_count,
            party_pool_used);
    }
    if (world_user_count < pvp_pool_used) {
        LogManager::logFormat(
            1,
            "localchina/localchina_App.cpp",
            "void ProcessServerStatusLog()",
            0x79,
            "[SESSION ERROR] CurrentUserCount(%d) < CurrentPVPCount(%d)",
            world_user_count,
            pvp_pool_used);
    }
    if (world_user_count < trade_pool_used) {
        LogManager::logFormat(
            1,
            "localchina/localchina_App.cpp",
            "void ProcessServerStatusLog()",
            0x7B,
            "[SESSION ERROR] CurrentUserCount(%d) < CurrentTraceSpaceCount(%d)",
            world_user_count,
            trade_pool_used);
    }
    return world_user_count;
}
