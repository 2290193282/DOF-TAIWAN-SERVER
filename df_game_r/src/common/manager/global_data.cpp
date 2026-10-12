#include "common/manager/global_data.h"
#include "common/manager/gm_list_abi.h"
#include "old_game/progression/career/expert_extraction_runtime.h"
#include "old_game/gm/core/cgm_manager_abi.h"
#include "old_game/gm/core/game_master_mgr_runtime.h"

#include "infra/security/crypto/ctea_abi.h"

#include "old_data/Login/simple_sso.h"

#include "infra/security/policy/black_ip_monitor.h"
#include "common/manager/community_server_proxy_abi.h"
#include "common/manager/double_conn_check_server_proxy_abi.h"
#include "common/manager/school_server_proxy_abi.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_data/channel/log_game_channel.h"
#include "old_data/channel/arad_server_state.h"
#include "core/config/env/environment.h"
#include "old_game/economy/mall/auction_server_packet_bridge.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "infra/network/packet/packet_pool.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/fatigue_battery_handle_abi.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/global_effect_manager_abi.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/security/policy/hades_server_packet_bridge.h"
#include "infra/security/policy/hades_server_proxy_abi.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/pvp/battlefield/pvp_reference_public_abi.h"
#include "infra/security/policy/restrict_geolocation_abi.h"
#include "infra/security/policy/secu_data_control_runtime.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/concurrent/timer/time_gate.h"
#include "old_data/channel/legacy_runtime/channel_proxy.h"
#include "old_data/db/core/db_connector.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/dungeon/room/pc_room_server_proxy_runtime.h"
#include "old_game/pvp/battlefield/timer_update_pvp_grade_runtime.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "old_game/world/portal/solo_teleport_dispatcher.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/stackable_action_runtime.h"
#include "old_data/Login/security/security_card_center.h"
#include "old_data/Login/security/auto_punish_black_ip_manager.h"
#include "old_data/Login/security/auto_punish_hack_type_manager.h"
#include "old_data/Login/inter_returns/break_away_prevent_runtime.h"
#include "old_data/Login/inter_returns/statistic_manager_runtime.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"
#include "infra/security/policy/antibot_business.h"
#include "infra/security/crypto/cipher.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/log/format/trace.h"
#include "common/manager/system_time.h"
#include "old_data/db/core/db_thread.h"
#include "old_data/db/core/feature_db_thread.h"
#include "infra/concurrent/thread/accept_thread.h"
#include "infra/concurrent/thread/monitor_tcp_thread.h"
#include "infra/concurrent/thread/network_thread.h"
#include "infra/concurrent/thread/udp_thread.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "old_data/db/taiwan/taiwan_runtime_dispatch.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <memory>
#include <new>
#include <pthread.h>
#if defined(__i386__)
static_assert(sizeof(ARAD::Arad_ServerStateManager) == 0x34, "Arad_ServerStateManager ABI size");
static_assert(sizeof(village_object::CVillageObjectMgr) == 0x18, "CVillageObjectMgr ABI size");
static_assert(sizeof(village_attacked::CVillageMonsterMgr) == 0x24, "CVillageMonsterMgr ABI size");
#endif

CServerProxyMgr<CMonitorServerProxy>* GlobalData::s_monitor_proxy_mgr = 0;
CServerProxyMgr<CGuildServerProxy>* GlobalData::s_guild_proxy_mgr = 0;
CServerProxyMgr<CStatisticServerProxy>* GlobalData::s_statistic_proxy_mgr = 0;
CPCRoomServerProxy* GlobalData::s_pcroom_proxy = 0;
WongWork::IPG::CIPGHelper* GlobalData::s_pIPGHelper = 0;
// IDA_DATA: _ZN10GlobalData9g_ActionsE
StackableAction::Action* GlobalData::g_Actions = 0;
expert_extraction::CExtractionMgr* GlobalData::g_ExtractionMgr = 0;
game_master::CGameMasterMgr* GlobalData::s_GameMasterMgr = 0;
// IDA_DATA: _ZN10GlobalData11g_pTeleportE
NPC_Teleport* GlobalData::g_pTeleport = 0;
// IDA_EA: 0x8299FA0
// IDA_NAME: _ZN10GlobalData20g_randomOptionHandleE
// DEMANGLED: GlobalData::g_randomOptionHandle
random_option::CRandomOptionItemHandle* GlobalData::g_randomOptionHandle = 0;
CStatisticMgr* GlobalData::s_statisticMgr = 0;
break_away_prevent::CBreakAwayPreventSystem* GlobalData::s_BreakAwaySys = 0;
CDoubleConnCheckServerProxy* GlobalData::s_double_check_proxy = 0;
CSchoolServerProxy* GlobalData::s_school_proxy = 0;
village_attacked::CRevengeDungeon* GlobalData::s_revengeDungeonMgr = 0;
TowerOfDespairMgr* GlobalData::s_pTowerOfDespairMgr = 0;
WongWork::CCeraShop* GlobalData::s_pCeraShop = 0;
ItemVendingMachine* GlobalData::s_pItemVendingMachine = 0;
// IDA_EA: 0x941F7C8
// IDA_NAME: _ZN10GlobalData18s_pSecuDataControlE
// DEMANGLED: GlobalData::s_pSecuDataControl
Secu_DataControl* GlobalData::s_pSecuDataControl = 0;
CSecu_ProtectionField* GlobalData::s_pSecuProtectionField = 0;
CItemAmplifier* GlobalData::s_itemAmplifier_ = 0;
CTEA* GlobalData::s_pcryptTEA_MousePassword_ = 0;
CTEA* GlobalData::s_pcryptTEA_SecurityCard_ = 0;
CBusinessImpl* GlobalData::s_antibotBusinessImpl_ = 0;
IAntibotInterface* GlobalData::s_antibotChecker_ = 0;

// IDA_EA: 0x941F6C8 / 0x941F6CC
// IDA_NAME: g_alchemistInfo / g_alchemistInfo_0
// The original importer owns two adjacent four-byte BSS integers.  Preserve
// their exact linker identities as one contiguous eight-byte definition.
extern "C" int g_alchemistInfo[2]
    __asm__("g_alchemistInfo") = {0, 0};
extern "C" int g_alchemistInfo_0
    __asm__("g_alchemistInfo_0") __attribute__((alias("g_alchemistInfo")));

#if defined(__i386__)
static_assert(sizeof(g_alchemistInfo) == 0x08u,
              "alchemist importer BSS must remain two adjacent integers");
static_assert(sizeof(CBusinessImpl) == 0x04u,
              "old anti-bot callback object is one i386 vptr");
#endif


namespace {

std::unique_ptr<PacketPool>
    s_packet_pool_list[GlobalData::THREAD_FEATURE_DB + 1];
std::unique_ptr<SmallStreamPool> s_small_stream_pool;
std::unique_ptr<BigStreamPool> s_big_stream_pool;
bool s_server_stop = false;
unsigned long
    s_packet_pool_thread_id[GlobalData::THREAD_FEATURE_DB + 1] = {};

// GlobalData::Init in the old ELF opens the master connection, obtains the
// per-type records from db_connect, and only then initializes IPG.  Keeping
// this as one helper preserves that ordering and ensures handle 14 (Taiwan
// billing) is a real opened connection rather than a default map entry.
bool InitializeDatabaseConnections() {
    CEnvironment* const environment = G_CEnvironment();
    if (!environment || !GlobalData::s_db_mgr) {
        return false;
    }

    CDBConnector* const connector =
        new (std::nothrow) CDBConnector(GlobalData::s_db_mgr);
    GlobalData::s_db_connector = connector;
    if (!connector) {
        return false;
    }

    std::map<ENUM_DB_HANDLE_IDX, ENUM_SERVER_GROUP> groups;
    const ENUM_SERVER_GROUP server_group =
        static_cast<ENUM_SERVER_GROUP>(environment->get_server_group());
    // Preserve the original bootstrap's legacy type range 1..14.  Deployed
    // type 15 belongs to the technical-report service and is intentionally
    // neither repurposed nor newly opened by df_game_r.
    // Types 17/18/19/20 must be separate db_connect rows even when they target
    // the same schemas as legacy types 3/2/4/9.
    for (int handle = 1; handle <= 14; ++handle) {
        groups[static_cast<ENUM_DB_HANDLE_IDX>(handle)] = server_group;
    }
    groups[DB_HANDLE_FEATURE_DB3] = server_group;
    groups[DB_HANDLE_FEATURE_DB2] = server_group;
    groups[DB_HANDLE_FEATURE_DB4] = server_group;
    groups[DB_HANDLE_FEATURE_DB9] = server_group;

    DBConnInfoMap& connection_map =
        environment->db_conn_info_map();
    DBConnInfoMap typed_connections;
    typed_connections[DB_HANDLE_0] = connection_map[DB_HANDLE_0];
    if (!connector->Init(typed_connections[DB_HANDLE_0]) ||
        connector->GetConnInfo(groups, typed_connections) != 1 ||
        connector->Connect(typed_connections) != 1) {
        connector->Close();
        delete connector;
        GlobalData::s_db_connector = 0;
        return false;
    }
    for (DBConnInfoMap::const_iterator it =
             typed_connections.begin();
         it != typed_connections.end(); ++it) {
        connection_map[it->first] = it->second;
    }
    return true;
}

}  // namespace

CSystemTime GlobalData::s_systemTime_;
MsgQueueMgr* GlobalData::s_msgq_mgr = 0;
StreamPool* GlobalData::s_stream_pool = 0;

// IDA_EA: 0x941F7DC
// IDA_NAME: _ZN10GlobalData27g_IPMonitorLastDBAccessTimeE
// DEMANGLED: GlobalData::g_IPMonitorLastDBAccessTime
// The old object is BSS, so leave it under normal C++ zero initialization.
time_t GlobalData::g_IPMonitorLastDBAccessTime;

// IDA_EA: 0x941F7D8
// IDA_NAME: _ZN10GlobalData16g_blackIPMonitorE
// DEMANGLED: GlobalData::g_blackIPMonitor
WongWork::CBlackIPMonitor* GlobalData::g_blackIPMonitor = 0;

// IDA_EA: 0x941F7D0
// IDA_NAME: _ZN10GlobalData26g_autoPunishRuleBlackIPMgrE
WongWork::CAutoPunishRuleBlackIPMgr* GlobalData::g_autoPunishRuleBlackIPMgr = 0;

// IDA_EA: 0x941F7D4
// IDA_NAME: _ZN10GlobalData27g_autoPunishRuleHackTypeMgrE
WongWork::CAutoPunishRuleHackTypeMgr* GlobalData::g_autoPunishRuleHackTypeMgr = 0;

#if defined(__i386__)
static_assert(sizeof(CSystemTime) == 20,
              "IDA GlobalData::s_systemTime_ storage is 20 bytes");
static_assert(sizeof(time_t) == 4,
              "IDA GlobalData::g_IPMonitorLastDBAccessTime is a 32-bit time_t");
#endif

Cipher* GlobalData::s_secu_cipher_keystring_manager_[
    GlobalData::kSecuCipherKeyStringCount] = {};
// IDA_EA: 0x941F70C
// IDA_NAME: _ZN10GlobalData20s_securityCardCenterE
WongWork::CSecurityCardCenter* GlobalData::s_securityCardCenter = 0;
DBMgr* GlobalData::s_db_mgr = 0;
DBThread* GlobalData::s_db_thr = 0;
DBThread* GlobalData::s_db_thr_for_logDB = 0;
FeatureDBThread* GlobalData::s_feature_db_thread = 0;
DispatchThread* GlobalData::s_dispatch_thr = 0;
AcceptThread* GlobalData::s_accept_thr = 0;
NetworkThread* GlobalData::s_net_thr = 0;
UdpThread* GlobalData::s_udp_thr = 0;
MonitorTcpThread* GlobalData::s_monitor_tcp_thr = 0;
Mutex GlobalData::s_accept_lock;
std::queue<CUser*> GlobalData::s_accept_que;
WongWork::CLogGameChannel* GlobalData::s_pLogGameChannel = 0;
CDailyScheduleManager* GlobalData::s_DailyScheduleManager = 0;
CEventManager* GlobalData::s_event_manager = 0;
secretshop::CSecretShop* GlobalData::s_secret_shop = 0;

int dword_940BE84[GlobalData::kSecuCipherKeyStringCount * 4];
int* const dword_940BE88 = dword_940BE84 + 1;

// IDA_EA: 0x8299FA0
// IDA_NAME: _ZN10GlobalData4InitEv
// DEMANGLED: GlobalData::Init(void)
bool GlobalData::Init() {
    s_server_stop = false;
    s_systemTime_.update();
    std::fill(dword_940BE84,
              dword_940BE84 + GlobalData::kSecuCipherKeyStringCount * 4,
              -1);
    s_msgq_mgr = new MsgQueueMgr();
    if (!s_msgq_mgr) {
        std::cerr << "[GlobalData::Init] can't allocate msg queue memory"
                  << std::endl;
        return false;
    }
    s_stream_pool = new StreamPool();
    if (!s_stream_pool) {
        std::cerr << "[GlobalData::Init] can't allocate stream pool memory"
                  << std::endl;
        return false;
    }
    if (!s_db_mgr) {
        s_db_mgr = new DBMgr();
    }
    if (!InitializeDatabaseConnections()) {
        std::cerr << "[GlobalData::Init] DBConnector initialization failed"
                  << std::endl;
        return false;
    }

    // IDA_EA: 0x8299FA0 (original Init, immediately after all DB handles are
    // open).  The Taiwan account-login path dereferences this singleton before
    // it evaluates member_login, so it must be constructed with the real
    // handle-6 connection before either DB worker can consume signal 3.
    s_psimpleSSO = new WongWork::CSimpleSSO();
    s_psimpleSSO->init(
        s_db_mgr->GetDBHandle(DB_HANDLE_LOG_REF, SERVER_GROUP_DEFAULT));

    s_db_thr = new DBThread();
    if (!s_db_thr) {
        std::cerr << "[GlobalData::Init] Can't allocate db thread memory"
                  << std::endl;
        return false;
    }
    s_db_thr->setLogDB(false);

    s_db_thr_for_logDB = new DBThread();
    if (!s_db_thr_for_logDB) {
        std::cerr << "[GlobalData::Init] Can't allocate db thread memory, "
                  << "s_db_thr_for_logDB" << std::endl;
        return false;
    }
    s_db_thr_for_logDB->setLogDB(true);
    s_feature_db_thread = new (std::nothrow) FeatureDBThread();
    if (!s_feature_db_thread || !s_feature_db_thread->ready()) {
        std::cerr << "[GlobalData::Init] Can't allocate native 0828 DB "
                     "thread or register its dispatchers"
                  << std::endl;
        return false;
    }
    s_dispatch_thr = new DispatchThread();
    if (!s_dispatch_thr) {
        std::cerr << "[GlobalData::Init] Can't allocate dispatch thread memory"
                  << std::endl;
        return false;
    }
    s_accept_thr = new AcceptThread();
    if (!s_accept_thr) {
        return false;
    }
    s_net_thr = new NetworkThread();
    if (!s_net_thr) {
        return false;
    }
    // GlobalData::Init @0x8299FA0 constructs UdpThread immediately before
    // MonitorTcpThread.  Preserve that owner ordering so create_thread can
    // bind THREAD_UDP/pool 5 before THREAD_MONITOR/pool 6.
    s_udp_thr = new UdpThread();
    if (!s_udp_thr) {
        return false;
    }
    s_monitor_tcp_thr = new MonitorTcpThread();
    if (!s_monitor_tcp_thr) {
        return false;
    }
    if (!s_pLogGameChannel) {
        s_pLogGameChannel = new (std::nothrow) WongWork::CLogGameChannel();
    }
    if (s_pLogGameChannel) {
        s_pLogGameChannel->StartLogTimer();
    }
    // IDA_EA: 0x829B541..0x829B5E2.  The old bootstrap publishes two
    // distinct CeraShop owners immediately after starting the game-channel
    // log timer: an 0x08 CCeraShop used by special-item consumers and an
    // 0x20 ItemVendingMachine containing its own CCeraShop subobject.  CMD163
    // dereferences the former directly through GlobalData::s_pCeraShop.
    if (!::GlobalData::s_pCeraShop) {
        ::GlobalData::s_pCeraShop =
            new (std::nothrow) ::WongWork::CCeraShop();
    }
    if (!::GlobalData::s_pItemVendingMachine) {
        ::GlobalData::s_pItemVendingMachine =
            new (std::nothrow) ::ItemVendingMachine();
    }
    if (!s_DailyScheduleManager) {
        s_DailyScheduleManager = new (std::nothrow) CDailyScheduleManager();
    }
    if (!s_DailyScheduleManager) {
        return false;
    }
    // IDA_EA: 0x8299FA0 allocates the 0x1c online preliminary-team manager
    // immediately after the daily scheduler.  Inter slots 488/492 and the
    // normal logout chain share this one process-lifetime owner.
    if (!GlobalData::s_onlinePreliminaryTeamMgr) {
        GlobalData::s_onlinePreliminaryTeamMgr =
            new (std::nothrow) online_preliminary::COnlinePreliminaryTeamMgr();
    }
    if (!GlobalData::s_onlinePreliminaryTeamMgr) {
        std::cerr << "[GlobalData::Init] Can't allocate online preliminary "
                     "team manager memory"
                  << std::endl;
        return false;
    }
    if (!s_event_manager) {
        s_event_manager = new (std::nothrow) CEventManager();
    }
    if (!s_event_manager) {
        return false;
    }
    if (!::GlobalData::s_fatigueBatteryHandle_) {
        ::GlobalData::s_fatigueBatteryHandle_ =
            new (std::nothrow) ::CFatigueBatteryHandle();
    }
    if (!::GlobalData::s_fatigueBatteryHandle_) {
        std::cerr
            << "[GlobalData::Init] Can't allocate Fatigue Battery Handle memory"
            << std::endl;
        return false;
    }
    // IDA_EA: 0x8299FA0 allocates the concrete 0x64-byte shop immediately
    // after the fatigue-battery service and before any party can start a map.
    if (!::GlobalData::s_secret_shop) {
        ::GlobalData::s_secret_shop =
            new (std::nothrow) secretshop::CSecretShop();
    }
    if (!::GlobalData::s_secret_shop) {
        std::cerr << "[GlobalData::Init] Can't allocate SecretShop Handle memory"
                  << std::endl;
        return false;
    }
    // IDA_EA: 0x829A... constructs the 0x30E1C monitor-proxy manager before
    // the app entrypoint's first heartbeat iteration.
    if (!::GlobalData::s_monitor_proxy_mgr) {
        ::GlobalData::s_monitor_proxy_mgr =
            new (std::nothrow) CServerProxyMgr<CMonitorServerProxy>();
    }
    if (!::GlobalData::s_monitor_proxy_mgr) {
        std::cerr
            << "[GlobalData::Init] Can't allocate monitor server proxy memory"
            << std::endl;
        return false;
    }
    if (!::GlobalData::s_guild_proxy_mgr) {
        // IDA Init @0x829A9DA allocates 0x30E34, invokes the concrete
        // CServerProxyMgr<CGuildServerProxy> C2, then stores this exact
        // GlobalData member before the app configures per-group endpoints.
        ::GlobalData::s_guild_proxy_mgr =
            new (std::nothrow) CServerProxyMgr<CGuildServerProxy>();
    }
    if (!::GlobalData::s_guild_proxy_mgr) {
        std::cerr << "[GlobalData::Init] Can't allocate guild server proxy memory"
                  << std::endl;
        return false;
    }
    // IDA_EA: 0x829AA09..0x829AA1B.  The statistic proxy manager is a
    // concrete 0x20-byte object and must exist before the app entrypoint
    // installs the configured endpoint (or the per-group PvP endpoints).
    if (!::GlobalData::s_statistic_proxy_mgr) {
        ::GlobalData::s_statistic_proxy_mgr =
            new (std::nothrow) CServerProxyMgr<CStatisticServerProxy>();
    }
    if (!::GlobalData::s_statistic_proxy_mgr) {
        std::cerr << "[GlobalData::Init] Can't allocate statistic server proxy memory"
                  << std::endl;
        return false;
    }
    // IDA_EA: 0x829AA31..0x829AAAD.  The original bootstrap constructs the
    // Hades TCP/UDP proxy immediately after the statistic manager, then calls
    // the BaseServerProxy initialization entry that establishes its listener.
    // Install the CNetwork server-frame bridge before the Hades C2 can open
    // its TCP socket.  At this point both the message queue and StreamPool
    // were created at the head of GlobalData::Init.
    InstallHadesServerPacketBridge();
    InstallAuctionServerPacketBridge();
    if (!GlobalData::s_hades_proxy) {
        GlobalData::s_hades_proxy = new (std::nothrow) CHadesServerProxy(
            const_cast<char*>(G_CEnvironment()->get_udp_ip_hades()),
            G_CEnvironment()->get_tcp_port_hades(),
            G_CEnvironment()->get_udp_port_hades());
    }
    if (!GlobalData::s_hades_proxy) {
        std::cerr << "[GlobalData::Init] Can't allocate Hades server proxy memory"
                  << std::endl;
        return false;
    }
    if (!GlobalData::s_hades_proxy->BaseServerProxy::Init()) {
        std::cerr << "[GlobalData::Init] Fail Init (Hades server proxy)"
                  << std::endl;
        return false;
    }
    if (!::GlobalData::s_pcroom_proxy) {
        const char* const pcroom_server_ip =
            G_CEnvironment()->get_pcroom_server_ip();
        const int pcroom_server_port =
            G_CEnvironment()->get_pcroom_server_port();
        ::GlobalData::s_pcroom_proxy = new (std::nothrow) CPCRoomServerProxy(
            const_cast<char*>(pcroom_server_ip), pcroom_server_port);
    }
    if (!::GlobalData::s_pcroom_proxy) {
        std::cerr << "[GlobalData::Init] Can't allocate pcroom server proxy memory"
                  << std::endl;
        return false;
    }
    if (!s_community_proxy) {
        // Old Init @0x8299FA0 allocates exactly 0x30E04 bytes, calls the
        // CCommunityServerProxy C2 with the configured host/port, and stores
        // GlobalData::s_community_proxy before PvP buddy dispatch can use it.
        s_community_proxy =
            new (std::nothrow) ::CCommunityServerProxy(
                const_cast<char*>(G_CEnvironment()->get_community_server_ip()),
                G_CEnvironment()->get_community_server_port());
    }
    if (!s_community_proxy) {
        return false;
    }
    if (!::GlobalData::s_pGMAccounts_) {
        ::GlobalData::s_pGMAccounts_ =
            new (std::nothrow) ::WongWork::CGMAccounts();
    }
    if (!::GlobalData::s_pGMAccounts_) {
        return false;
    }
    // IDA 0x829C41F..0x829C532: after the GM-account store exists, Init
    // constructs the four-byte Gm_List (which loads gm_manifest), then the
    // 0x3c CGM_Manager used directly by world logout and GM-mode checks.
    cMyTrace gm_list_trace("static bool GlobalData::Init()", 7622, 9, true,
                           true);
    gm_list_trace("\t- Load GM List From DB");
    if (!::GlobalData::gmList) {
        ::GlobalData::gmList = new (std::nothrow) Gm_List();
    }
    if (!::GlobalData::gmList) {
        return false;
    }
    if (!::GlobalData::s_GM_Manager) {
        ::GlobalData::s_GM_Manager = new (std::nothrow) CGM_Manager();
    }
    if (!::GlobalData::s_GM_Manager) {
        cMyTrace trace("static bool GlobalData::Init()", 7638, 9, true,
                       true);
        trace("Can't allocate memory for a CGM_Manager");
        return false;
    }
    // IDA_EA: 0x8299FA0 allocates one byte, invokes the empty C2, and
    // publishes this command service immediately before SecuProtectionField.
    // The old Fini path has no matching destruction branch, so it remains a
    // process-lifetime allocation.
    if (!::GlobalData::s_pSecuDataControl) {
        ::GlobalData::s_pSecuDataControl = new ::Secu_DataControl();
    }
    if (!::GlobalData::s_pSecuProtectionField) {
        ::GlobalData::s_pSecuProtectionField =
            new (std::nothrow) ::CSecu_ProtectionField();
    }
    if (!::GlobalData::s_pSecuProtectionField) {
        return false;
    }
    if (!::GlobalData::g_autoPunishRuleBlackIPMgr) {
        // Old Init @0x829B673 allocates 0x18 bytes, invokes C2, stores the
        // exact pointer global, then calls reset() @0x829B69F.  Old Fini has
        // no reference to this allocation, so no speculative shutdown path is
        // added here.
        ::GlobalData::g_autoPunishRuleBlackIPMgr =
            new (std::nothrow) ::WongWork::CAutoPunishRuleBlackIPMgr();
    }
    if (!::GlobalData::g_autoPunishRuleBlackIPMgr) {
        return false;
    }
    ::GlobalData::g_autoPunishRuleBlackIPMgr->reset();
    // Old Init @0x829B685 allocates 0x70, invokes the named C2, stores
    // g_autoPunishRuleHackTypeMgr @0x941F7D4, then calls reset @0x829B6C6.
    // Old Fini has no reference to this allocation, so no speculative
    // destructor/lifetime route is introduced here.
    ::GlobalData::g_autoPunishRuleHackTypeMgr =
        new ::WongWork::CAutoPunishRuleHackTypeMgr();
    ::GlobalData::g_autoPunishRuleHackTypeMgr->reset();
    if (!::GlobalData::s_securityCardCenter) {
        // Raw Init @0x829BD9F allocates 0x488 bytes, invokes C2, writes
        // s_securityCardCenter @0x941F70C, then invokes init() @0x829BDDD.
        ::GlobalData::s_securityCardCenter =
            new ::WongWork::CSecurityCardCenter();
        ::GlobalData::s_securityCardCenter->init();
    }
    // IDA_EA: 0x829BDE2..0x829BE63.  Bootstrap initializes the local helper
    // first and publishes s_pIPGHelper only after Initialize succeeds.
    WongWork::IPG::CIPGHelper* const ipg_helper =
        new (std::nothrow) WongWork::IPG::CIPGHelper();
    if (ipg_helper->Initialize(1, 0x1388u) < 0) {
        // CIPGHelper has the original non-virtual D2 destructor.  The old
        // failure branch invokes it directly before operator delete.
        if (ipg_helper) {
            ipg_helper->~CIPGHelper();
            ::operator delete(ipg_helper);
        }
        std::puts("Fail to init IPG Module");
        return false;
    }
    ::GlobalData::s_pIPGHelper = ipg_helper;
    if (!::GlobalData::g_blackIPMonitor) {
        // Raw GlobalData::Init allocates 0x18 bytes, invokes C2, stores the
        // typed global, then calls reset().  The initial DB request itself is
        // deliberately deferred until DBTask/InterDispatcher/DispatchThread
        // are restored as their complete original tables.
        ::GlobalData::g_blackIPMonitor =
            new (std::nothrow) ::WongWork::CBlackIPMonitor();
    }
    if (!::GlobalData::g_blackIPMonitor) {
        return false;
    }
    ::GlobalData::g_blackIPMonitor->reset();
    if (!::GlobalData::s_ref_pvp_grade) {
        void* storage =
            ::operator new(sizeof(RefPvpGrade), std::nothrow);
        if (!storage) {
            return false;
        }
        ::GlobalData::s_ref_pvp_grade =
            static_cast<RefPvpGrade*>(storage);
        new ((::GlobalData::s_ref_pvp_grade)) RefPvpGrade();
    }
    if (!::GlobalData::s_itemAmplifier_) {
        ::GlobalData::s_itemAmplifier_ =
            new (std::nothrow) ::CItemAmplifier();
    }
    if (!::GlobalData::s_itemAmplifier_) {
        return false;
    }
    if (!::GlobalData::s_globalEffectManager_) {
        ::GlobalData::s_globalEffectManager_ = new ::CGlobalEffectManager();
    }
    if (!::GlobalData::s_timeGate_) {
        ::GlobalData::s_timeGate_ = new (std::nothrow) ::CTimeGate();
    }
    if (!::GlobalData::s_timeGate_) {
        return false;
    }
    // IDA_EA: 0x8299FA0: CTimeGate is followed by the concrete 0x34-byte
    // server-state owner; channel-load code relies on it being non-lazy.
    if (!::GlobalData::s_serverStateManager_) {
        ::GlobalData::s_serverStateManager_ =
            new ARAD::Arad_ServerStateManager();
    }
    // IDA_EA: 0x829C59A..0x829C718.  The statistic manager gates the
    // break-away system; only after both concrete allocations succeed does
    // old Init construct the following expert-job manager.
    if (!::GlobalData::s_statisticMgr) {
        ::GlobalData::s_statisticMgr = new (std::nothrow) ::CStatisticMgr();
    }
    if (!::GlobalData::s_statisticMgr) {
        std::cerr << "Can't allocate memory for a CStatisticMgr" << std::endl;
        return false;
    }
    if (!::GlobalData::s_BreakAwaySys) {
        ::GlobalData::s_BreakAwaySys =
            new (std::nothrow) ::break_away_prevent::CBreakAwayPreventSystem();
    }
    if (!::GlobalData::s_BreakAwaySys) {
        std::cerr << "Can't allocate memory for a s_BreakAwaySys" << std::endl;
        return false;
    }
    if (!GlobalData::s_ExpertJobMgr) {
        // Raw IDA Init allocates the exact four-byte manager between the
        // break-away system and village-object manager, invokes C2, and
        // stores GlobalData::s_ExpertJobMgr before gameplay starts.
        GlobalData::s_ExpertJobMgr =
            new (std::nothrow) ::expert_job::CExpertJobMgr();
    }
    // Raw GlobalData::Init allocates the 0x40 GM root after its command
    // dependencies are available.  The dispatcher remains detached until its
    // complete old command fallback is recovered, but the real lifetime and
    // named global must exist now for each registered command's ABI.
    if (!::GlobalData::s_GameMasterMgr) {
        ::GlobalData::s_GameMasterMgr =
            new (std::nothrow) game_master::CGameMasterMgr();
    }
    if (!::GlobalData::s_GameMasterMgr) return false;

    // IDA 0x829CC71..0x829CC80 records the current server time and passes
    // it to the channel-1 PvP-grade scheduler immediately after constructing
    // the game-master root, before the initial security DB requests.
    TimerUpdatePvPGrade::registNextTimer(
        static_cast<time_t>(::GlobalData::s_systemTime_.getCurSec()));

    // IDA_EA: 0x829CC7E..0x829CD71.  The original startup allocates the
    // concrete callback, loads libantisvrimport.so from the current game
    // directory, then invokes checker vtable slot +0x00 with the decimal
    // process-sequence text.  PacketDispatcher's outer +0x3C gate relies on
    // this object being live even for the first CMD1 before account login.
    if (!::GlobalData::s_antibotBusinessImpl_) {
        ::GlobalData::s_antibotBusinessImpl_ =
            new (std::nothrow) CBusinessImpl();
    }
    if (!::GlobalData::s_antibotBusinessImpl_) {
        std::cerr << "[GlobalData::Init] Can't allocate anti-bot business "
                     "memory"
                  << std::endl;
        return false;
    }
    ::GlobalData::s_antibotChecker_ = CreateAntiServerObj("./");
    if (!::GlobalData::s_antibotChecker_) {
        std::cerr << "[GlobalData::Init] Can't create anti-bot checker"
                  << std::endl;
        return false;
    }
    CEnvironment* const environment = G_CEnvironment();
    if (!environment) {
        return false;
    }
    char process_sequence_text[12] = {};
    std::snprintf(process_sequence_text,
                  sizeof(process_sequence_text),
                  "%d",
                  environment->getProcessSequence());
    // The original assumes that the external TSS IPC is always ready.  The
    // bundled plug-in can leave its IPC channel half-initialized when open()
    // fails, and its first ProcessAntiBotInput call then dereferences a null
    // shared-memory deque.  Fail the process initialization here instead of
    // allowing the main loop to enter that unsafe state.
    const int antibot_init_result =
        InitializeAntiBotChecker(::GlobalData::s_antibotChecker_,
                                 ::GlobalData::s_antibotBusinessImpl_,
                                 process_sequence_text);
    if (antibot_init_result != 0) {
        std::cerr << "[GlobalData::Init] Anti-bot checker initialization "
                     "failed, rc="
                  << antibot_init_result
                  << ", process_sequence=" << process_sequence_text
                  << std::endl;
        return false;
    }

    // IDA GlobalData::Init @ 0x8299FA0: once the GM manager has been
    // published, generate exactly GetTotalKeyLength() bytes with rand() %
    // 256 and initialise the process-wide cipher.  CUser::InitOnAccept then
    // sends this same raw key to the 0627 client before encrypted CMD traffic.
    Cipher* const protocol_cipher = G_Cipher();
    const int protocol_key_length = protocol_cipher->GetTotalKeyLength();
    if (protocol_key_length < 0 ||
        protocol_key_length > Cipher::kOriginalKeyCapacity) {
        return false;
    }
    unsigned char protocol_key[Cipher::kOriginalKeyCapacity];
    for (int index = 0; index < protocol_key_length; ++index) {
        protocol_key[index] = static_cast<unsigned char>(::rand() % 256);
    }
    if (!protocol_cipher->Initialize(protocol_key, protocol_key_length)) {
        return false;
    }

    if (!::GlobalData::g_ExtractionMgr) {
        ::GlobalData::g_ExtractionMgr =
            new (std::nothrow) expert_extraction::CExtractionMgr();
    }
    if (!::GlobalData::g_ExtractionMgr) return false;
    if (!GlobalData::s_ExpertJobMgr) {
        return false;
    }
    // IDA_EA: 0x8299FA0 owner order: ExpertJob -> VillageObject ->
    // VillageMonster -> InstanceRentalSystem -> RevengeDungeon.
    if (!::GlobalData::s_villageObjectMgr) {
        ::GlobalData::s_villageObjectMgr =
            new (std::nothrow) village_object::CVillageObjectMgr();
    }
    if (!::GlobalData::s_villageObjectMgr) {
        return false;
    }
    if (!::GlobalData::s_villageMonsterMgr) {
        ::GlobalData::s_villageMonsterMgr =
            new (std::nothrow) village_attacked::CVillageMonsterMgr();
    }
    if (!::GlobalData::s_villageMonsterMgr) {
        return false;
    }
    if (!::GlobalData::s_rentalSystem) {
        ::GlobalData::s_rentalSystem =
            new (std::nothrow) ::InstanceRentalSystem();
    }
    if (!::GlobalData::s_rentalSystem) {
        return false;
    }
    if (!::GlobalData::s_auction_proxy) {
        ::GlobalData::s_auction_proxy =
            new (std::nothrow) ::CAuctionServerProxy(
                const_cast<char*>(G_CEnvironment()->get_auction_server_ip()),
                G_CEnvironment()->get_auction_server_port(),
                const_cast<char*>("Auction Server"));
    }
    if (!::GlobalData::s_auction_proxy) {
        return false;
    }
    if (!::GlobalData::s_cera_auction_proxy) {
        ::GlobalData::s_cera_auction_proxy =
            new (std::nothrow) ::CCeraAuctionServerProxy(
                const_cast<char*>(
                    G_CEnvironment()->get_cera_auction_server_ip()),
                G_CEnvironment()->get_cera_auction_server_port(),
                const_cast<char*>("Cera Auction Server"));
    }
    if (!::GlobalData::s_cera_auction_proxy) {
        return false;
    }
    // IDA_EA: 0x8299FA0, original sequence 0x829ADAE..0x829AEA8.
    // The old bootstrap allocates exactly 0x208 bytes, invokes C2, publishes
    // GlobalData::s_power_manager, requires Init()==1, then queues the
    // PowerWar-info and ranker requests.  Quest selection dereferences this
    // object during Inter_LoadEtc, so a declaration-only global is not valid.
    if (!::GlobalData::s_power_manager) {
        ::GlobalData::s_power_manager = new (std::nothrow) CPowerManager();
    }
    if (!::GlobalData::s_power_manager) {
        std::cerr << "[GlobalData::Init] Can't allocate power manager memory"
                  << std::endl;
        return false;
    }
    if (::GlobalData::s_power_manager->Init() != 1) {
        std::cerr << "[GlobalData::Init] Fail Init (power manager memory)"
                  << std::endl;
        return false;
    }
    ::GlobalData::s_power_manager->LoadPowerWarInfo();
    ::GlobalData::s_power_manager->LoadRankerInfo();
    // IDA_EA: 0x829B14D..0x829B2AD.  The old bootstrap constructs the
    // process-wide channel proxy from the configured UDP endpoint before the
    // main loop reaches its periodic SendChannelInfo call.
    if (!::GlobalData::s_channel_proxy) {
        ::GlobalData::s_channel_proxy = new (std::nothrow) CmmChannelProxy(
            G_CEnvironment()->get_udp_ip_channel(),
            G_CEnvironment()->get_udp_port_channel());
    }
    if (!::GlobalData::s_channel_proxy) {
        std::cerr
            << "[GlobalData::Init] Can't allocate channel server proxy memory"
            << std::endl;
        return false;
    }
    if (!::GlobalData::s_channel_proxy->Init()) {
        std::cerr << "[GlobalData::Init] Fail Init (channel server proxy)"
                  << std::endl;
        return false;
    }
    if (!::GlobalData::s_double_check_proxy) {
        // Raw Init constructs this exact 0x10-byte UDP proxy from the
        // configured double-check endpoint and then opens its client socket.
        ::GlobalData::s_double_check_proxy =
            new (std::nothrow) ::CDoubleConnCheckServerProxy(
                G_CEnvironment()->get_udp_ip_doublecheck(),
                G_CEnvironment()->get_udp_port_doublecheck());
        if (!::GlobalData::s_double_check_proxy ||
            !::GlobalData::s_double_check_proxy->Init()) {
            return false;
        }
    }
    if (*G_CEnvironment()->get_school_server_ip()) {
        if (G_CEnvironment()->get_school_server_port() > 0) {
            const std::string school_server_ip(
                G_CEnvironment()->get_school_server_ip());
            const int school_server_port =
                G_CEnvironment()->get_school_server_port();
            ::GlobalData::s_school_proxy =
                new (std::nothrow) ::CSchoolServerProxy(school_server_ip,
                                                         school_server_port);
            if (!::GlobalData::s_school_proxy) {
                std::cerr
                    << "[GlobalData::Init] Can't allocate school server proxy memory"
                    << std::endl;
                return false;
            }
            if (!::GlobalData::s_school_proxy->Init()) {
                std::cerr
                    << "[GlobalData::Init] Fail Init (school server proxy)"
                    << std::endl;
                return false;
            }
        }
    }
    if (!::GlobalData::s_revengeDungeonMgr) {
        // The bootstrap path allocates this exact 0x08 village-state object
        // before dungeon admission and world-entry consumers observe it.
        ::GlobalData::s_revengeDungeonMgr =
            new (std::nothrow) ::village_attacked::CRevengeDungeon();
    }
    if (!::GlobalData::s_revengeDungeonMgr) {
        return false;
    }
    for (int index = THREAD_DISPATCH;
         index <= THREAD_FEATURE_DB; ++index) {
        if (!s_packet_pool_list[index].get()) {
            s_packet_pool_list[index].reset(new PacketPool());
        }
    }
    // IDA 0x829B1C5: the bootstrap/main thread owns packet pool slot 4
    // before it constructs the stream pools and begins creating startup
    // packets.  Without this binding PacketGuard silently has no owner on
    // the main thread and outgoing bootstrap packets diverge from old srv.
    if (!setCurrentPacketPool(static_cast<THREAD_TYPE>(::THREAD_MAIN),
                              static_cast<unsigned long>(pthread_self()))) {
        return false;
    }
    if (!s_small_stream_pool.get()) {
        s_small_stream_pool.reset(new SmallStreamPool());
    }
    if (!s_big_stream_pool.get()) {
        s_big_stream_pool.reset(new BigStreamPool());
    }
    // IDA_EA: 0x829AB20..0x829AB82.  These are distinct old CTEA instances;
    // login's encrypted eight-byte password uses the mouse-password key,
    // while security-card packets use their own chain/key object.
    if (!s_pcryptTEA_MousePassword_) {
        s_pcryptTEA_MousePassword_ = new CTEA();
        s_pcryptTEA_MousePassword_->Initialize("tjsghTlWkdajtwla", 16,
                                               CTEA::sm_chain0, 0, 0);
    }
    if (!s_pcryptTEA_SecurityCard_) {
        s_pcryptTEA_SecurityCard_ = new CTEA();
        s_pcryptTEA_SecurityCard_->Initialize("ejsvk@@@secucard", 16,
                                              CTEA::sm_chain0, 0, 0);
    }
    // Raw Init first creates the 0x30 StackableAction::Action router, then
    // allocates the two 0x14 polymorphic teleport objects.  All three have
    // process lifetime; CMD516 and CMD471 depend on this ordering.
    if (!::GlobalData::g_Actions) {
        ::GlobalData::g_Actions = new (std::nothrow) StackableAction::Action();
    }
    if (!::GlobalData::g_Actions) {
        return false;
    }

    // IDA 0x829CF3E..0x829CFCB: after publishing the stackable-action
    // router, startup requests the no-payload Tower-of-Despair APC reset.
    // The DB13 consumer owns signal 631; this is only its original runtime
    // producer and retains the full-width two-int queue header.
    {
        CStreamGuard guard(s_stream_pool->Acquire("App.cpp", 7904), true);
        (*guard) << 631;
        (*guard) << -1;
        s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }

    // IDA Init allocates the two 0x14 polymorphic teleport objects between
    // extraction setup and the random-option handle.  They have process
    // lifetime (Fini has no destruction branch) and CMD471 depends on both
    // existing before dispatch can resolve a destination.
    if (!::GlobalData::g_pTeleport) {
        ::GlobalData::g_pTeleport = new (std::nothrow) NPC_Teleport();
    }
    if (!::GlobalData::g_pTeleport) {
        return false;
    }
    if (!g_user_solo_teleport) {
        g_user_solo_teleport = new (std::nothrow) User_Teleport();
    }
    if (!g_user_solo_teleport) {
        return false;
    }
    // Raw Init @0x8299FA0 allocates 0x18 bytes with nothrow new, invokes
    // CRandomOptionItemHandle::C2, then stores this exact pointer before
    // ServiceRestrictManager starts policy initialization. Raw Fini has no
    // corresponding destruction branch, so the lifetime remains process-wide.
    if (!::GlobalData::g_randomOptionHandle) {
        ::GlobalData::g_randomOptionHandle =
            new (std::nothrow) random_option::CRandomOptionItemHandle();
    }
    if (!::GlobalData::g_randomOptionHandle) {
        return false;
    }
    ServiceRestrictManager* service_restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (!service_restrict_manager ||
        service_restrict_manager->initialize() != 1) {
        return false;
    }
    service_restrict_manager->savePolicy();
    service_restrict_manager->loadPolicy();
    // IDA 0x8299FA0 reaches RestrictGeolocation::initGeolocation directly
    // after the service-restriction save/load requests.  The call creates the
    // real GeoIP object and queues Taiwan inner code 4; omitting it means no
    // geo_allow_country query can ever produce the CN/HK/KR/MO/TW callbacks.
    if (!ARAD::Singleton<RestrictGeolocation>::Get()->initGeolocation()) {
        std::cerr << "Can't initialize GeoIP" << std::endl;
        return false;
    }
    return true;
}

// IDA_EA: 0x829D438
// IDA_NAME: _ZN10GlobalData11InitCaptchaEv
// DEMANGLED: GlobalData::InitCaptcha(void)
bool GlobalData::InitCaptcha() {
    return false;
}

// IDA_EA: 0x829D4B0
// IDA_NAME: _ZN10GlobalData19GenerateCaptchaDataEv
// DEMANGLED: GlobalData::GenerateCaptchaData(void)
void GlobalData::GenerateCaptchaData() {
}

// IDA_EA: 0x829DD82
// IDA_NAME: _ZN10GlobalData16initTournamentDBEPKc
// DEMANGLED: GlobalData::initTournamentDB(char const*)
bool GlobalData::initTournamentDB(const char* file_name) {
    GameWorld* const world = G_GameWorld();
    if (!world) {
        return false;
    }
    if (world->GetChannelType() != 7 &&
        !world->IsIntegratedPvPBaseChannel()) {
        return true;
    }

    CDBEnviroment db_environment;
    db_environment.set_file_name(file_name);
    if (!db_environment.load() || !s_db_connector) {
        return false;
    }

    CEnvironment* const environment = G_CEnvironment();
    TournamentConfigMap& tournament_configs =
        environment->tournament_config_map();
    ServerGroupDBConnInfoMap& server_group_connections =
        environment->server_group_db_conn_info_map();
    for (TournamentConfigMap::iterator config = tournament_configs.begin();
         config != tournament_configs.end(); ++config) {
        const ENUM_SERVER_GROUP server_group = config->first;
        std::map<ENUM_DB_HANDLE_IDX, ENUM_SERVER_GROUP> requested_handles;
        requested_handles.insert(std::make_pair(DB_HANDLE_2, server_group));
        requested_handles.insert(std::make_pair(DB_HANDLE_3, server_group));
        requested_handles.insert(std::make_pair(
            DB_HANDLE_FEATURE_DB3, server_group));
        requested_handles.insert(std::make_pair(
            DB_HANDLE_FEATURE_DB2, server_group));
        requested_handles.insert(std::make_pair(
            DB_HANDLE_FEATURE_DB4, server_group));
        requested_handles.insert(std::make_pair(
            DB_HANDLE_FEATURE_DB9, server_group));

        DBConnInfoMap& connection_info =
            server_group_connections[server_group];
        if (s_db_connector->GetConnInfo(requested_handles, connection_info) !=
                1 ||
            s_db_connector->ConnectWithServerGroup(connection_info) != 1) {
            return false;
        }
    }

    cMyTrace trace("static bool GlobalData::initTournamentDB(const char*)",
                   8362, 0);
    trace("This is Tournament PvP channel");
    s_db_mgr->activateGlobalDB();
    return true;
}

// IDA_EA: 0x829E0B2
// IDA_NAME: _ZN10GlobalData4FiniEv
// DEMANGLED: GlobalData::Fini(void)
bool GlobalData::Fini() {
    if (::GlobalData::s_ref_pvp_grade) {
        RefPvpGrade* pvp_grade = ::GlobalData::s_ref_pvp_grade;
        pvp_grade->~RefPvpGrade();
        ::operator delete(pvp_grade);
        ::GlobalData::s_ref_pvp_grade = 0;
    }
    s_server_stop = true;
    StopTaiwanDatabaseRuntime();
    // IDA_EA: 0x829E34E..0x829E36E.  The original destroys only the
    // CBusinessImpl callback; the dynamically loaded checker is retained for
    // process lifetime and is never deleted or dlclosed here.
    if (::GlobalData::s_antibotBusinessImpl_) {
        delete ::GlobalData::s_antibotBusinessImpl_;
        ::GlobalData::s_antibotBusinessImpl_ = 0;
    }
    ::GlobalData::s_antibotChecker_ = 0;
    for (int index = 0; index < kSecuCipherKeyStringCount; ++index) {
        delete s_secu_cipher_keystring_manager_[index];
        s_secu_cipher_keystring_manager_[index] = 0;
    }
    std::fill(dword_940BE84,
              dword_940BE84 + GlobalData::kSecuCipherKeyStringCount * 4,
              -1);
    if (GlobalData::s_db_connector) {
        GlobalData::s_db_connector->Close();
        delete GlobalData::s_db_connector;
        GlobalData::s_db_connector = 0;
    }
    delete s_feature_db_thread;
    s_feature_db_thread = 0;
    delete s_db_thr_for_logDB;
    s_db_thr_for_logDB = 0;
    delete s_db_thr;
    s_db_thr = 0;
    delete s_dispatch_thr;
    s_dispatch_thr = 0;
    delete s_accept_thr;
    s_accept_thr = 0;
    delete s_udp_thr;
    s_udp_thr = 0;
    delete s_net_thr;
    s_net_thr = 0;
    delete s_monitor_tcp_thr;
    s_monitor_tcp_thr = 0;

    // IDA_EA: 0x829E0B2 (original Fini).  Keep the singleton alive until the
    // DB workers are gone, then pair the Init allocation with its real C++
    // destructor while its MySQL handle still belongs to DBMgr.
    delete s_psimpleSSO;
    s_psimpleSSO = 0;

    delete s_db_mgr;
    s_db_mgr = 0;
    delete s_DailyScheduleManager;
    s_DailyScheduleManager = 0;
    delete GlobalData::s_onlinePreliminaryTeamMgr;
    GlobalData::s_onlinePreliminaryTeamMgr = 0;
    delete s_event_manager;
    s_event_manager = 0;
    delete ::GlobalData::s_monitor_proxy_mgr;
    ::GlobalData::s_monitor_proxy_mgr = 0;
    delete ::GlobalData::s_guild_proxy_mgr;
    ::GlobalData::s_guild_proxy_mgr = 0;
    delete ::GlobalData::s_statistic_proxy_mgr;
    ::GlobalData::s_statistic_proxy_mgr = 0;
    delete ::GlobalData::s_pGMAccounts_;
    ::GlobalData::s_pGMAccounts_ = 0;
    delete ::GlobalData::s_GM_Manager;
    ::GlobalData::s_GM_Manager = 0;
    delete ::GlobalData::s_pSecuProtectionField;
    ::GlobalData::s_pSecuProtectionField = 0;
    delete ::GlobalData::s_BreakAwaySys;
    ::GlobalData::s_BreakAwaySys = 0;
    delete GlobalData::s_ExpertJobMgr;
    GlobalData::s_ExpertJobMgr = 0;
    delete ::GlobalData::s_GameMasterMgr;
    // IDA_EA: 0x829E0B2: object then monster after ExpertJobMgr.
    delete ::GlobalData::s_villageObjectMgr;
    ::GlobalData::s_villageObjectMgr = 0;
    delete ::GlobalData::s_villageMonsterMgr;
    ::GlobalData::s_villageMonsterMgr = 0;

    ::GlobalData::s_GameMasterMgr = 0;
    delete ::GlobalData::g_ExtractionMgr;
    ::GlobalData::g_ExtractionMgr = 0;
    delete ::GlobalData::s_rentalSystem;
    ::GlobalData::s_rentalSystem = 0;
    delete ::GlobalData::s_auction_proxy;
    ::GlobalData::s_auction_proxy = 0;
    delete ::GlobalData::s_cera_auction_proxy;
    ::GlobalData::s_cera_auction_proxy = 0;
    delete ::GlobalData::s_double_check_proxy;
    ::GlobalData::s_double_check_proxy = 0;
    delete ::GlobalData::s_revengeDungeonMgr;
    ::GlobalData::s_revengeDungeonMgr = 0;
    s_big_stream_pool.reset();
    s_small_stream_pool.reset();
    delete s_stream_pool;
    for (int index = THREAD_DISPATCH;
         index <= THREAD_FEATURE_DB; ++index) {
        s_packet_pool_list[index].reset();
    }
    delete s_msgq_mgr;
    // IDA_EA: 0x829E2B5..0x829E2CA.  Fini destroys the vending-machine
    // aggregate immediately after the message-queue manager.  The standalone
    // s_pCeraShop has process lifetime in the old executable and deliberately
    // has no matching Fini branch.
    delete ::GlobalData::s_pItemVendingMachine;
    ::GlobalData::s_pItemVendingMachine = 0;
    for (int index = THREAD_DISPATCH;
         index <= THREAD_FEATURE_DB; ++index) {
        s_packet_pool_thread_id[index] = 0;
    }
    return true;
}

// IDA_EA: 0x829E4C4
// IDA_NAME: _ZN10GlobalData20setCurrentPacketPoolE11THREAD_TYPEm
// DEMANGLED: GlobalData::setCurrentPacketPool(THREAD_TYPE,unsigned long)
bool GlobalData::setCurrentPacketPool(THREAD_TYPE pool_type,
                                      unsigned long thread_id) {
    if (pool_type < ::THREAD_DISPATCH ||
        pool_type > ::THREAD_FEATURE_DB) {
        return false;
    }
    if (s_packet_pool_thread_id[pool_type] != 0) {
        return false;
    }
    s_packet_pool_thread_id[pool_type] = thread_id;
    if (s_packet_pool_list[pool_type].get() != 0) {
        s_packet_pool_list[pool_type]->setThreadID(thread_id);
    }
    return true;
}

bool GlobalData::server_stop() {
    return s_server_stop;
}

void GlobalData::set_server_stop(bool stop) {
    s_server_stop = stop;
}

CSystemTime* GlobalData::system_time() {
    return &s_systemTime_;
}

MsgQueueMgr* GlobalData::msgq_mgr() {
    return s_msgq_mgr;
}

PacketPool* GlobalData::packet_pool(THREAD_TYPE pool_type) {
    if (pool_type < ::THREAD_DISPATCH ||
        pool_type > ::THREAD_FEATURE_DB) {
        return 0;
    }
    return s_packet_pool_list[pool_type].get();
}

StreamPool* GlobalData::stream_pool() {
    return s_stream_pool;
}

SmallStreamPool* GlobalData::small_stream_pool() {
    return s_small_stream_pool.get();
}

BigStreamPool* GlobalData::big_stream_pool() {
    return s_big_stream_pool.get();
}

IAntibotInterface* GlobalData::antibot_checker() {
    return s_antibotChecker_;
}

void GlobalData::set_antibot_checker(IAntibotInterface* checker) {
    s_antibotChecker_ = checker;
}
