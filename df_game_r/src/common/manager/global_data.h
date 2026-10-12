#ifndef OLDSRV_GLOBAL_GLOBAL_DATA_H_
#define OLDSRV_GLOBAL_GLOBAL_DATA_H_

#include <time.h>

namespace expert_extraction { class CExtractionMgr; }
namespace game_master { class CGameMasterMgr; }
#include <queue>

#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/concurrent/thread/mutex.h"

class CSystemTime;
class CmmChannelProxy;
class MsgQueueMgr;
class PacketPool;
class Cipher;
class CTEA;
class Secu_DataControl;
class StreamPool;
class SmallStreamPool;
class BigStreamPool;
class DBMgr;
class CDBConnector;
class DBThread;
class FeatureDBThread;
class DispatchThread;
class AcceptThread;
class NetworkThread;
class UdpThread;
class MonitorTcpThread;
class CUser;
class CDailyScheduleManager;
class STScriptFileList;
class CEventManager;
class CGM_Manager;
class Gm_List;
class CPowerManager;
class CRijndael;
class CHadesServerProxy;
namespace online_preliminary {
class COnlinePreliminaryTeamMgr;
}
class CFatigueBatteryHandle;
class CStatisticServerProxy;
class CStatisticMgr;
class CDoubleConnCheckServerProxy;
class CSchoolServerProxy;
class CGuildServerProxy;
class CPCRoomServerProxy;
class CCommunityServerProxy;
struct Event_Upgrade_Jar;
class TowerOfDespairMgr;
class AvatarRouletteServer;
class IAntibotInterface;
class CBusinessImpl;
namespace ARAD {
class Arad_ServerStateManager;
}
namespace village_attacked {
class CRevengeDungeon;
class CVillageMonsterMgr;
}
namespace village_object {
class CVillageObjectMgr;
}
namespace secretshop {
class CSecretShop;
}
namespace expert_job {
class CExpertJobMgr;
}
namespace ExtreamDungeon {
class CCompound_ExtreamDun_Item;
}
template <typename T>
class CServerProxyMgr;

// IDA_NAME: GlobalData::setCurrentPacketPool
// _ZN10GlobalData20setCurrentPacketPoolE11THREAD_TYPEm encodes this as a
// global enum, not as a nested GlobalData enum.
enum THREAD_TYPE {
    // IDA_EA: create_thread @ 0x829FD30.  These are packet-pool ownership
    // slots, not an arbitrary local ordering.  The old startup sequence
    // binds dispatch, accept, network, exchange/main, UDP, monitor, DB and
    // log-DB to exactly the indices below.
    THREAD_DISPATCH = 0,
    THREAD_ACCEPT = 1,
    THREAD_NETWORK = 2,
    THREAD_EXCHANGE = 3,
    THREAD_MAIN = 4,
    THREAD_UDP = 5,
    THREAD_MONITOR = 6,
    THREAD_DB = 7,
    THREAD_LOGDB = 8,
    THREAD_FEATURE_DB = 9,
};

namespace WongWork {
class CLogGameChannel;
class CCompoundAvatar;
class CSimpleSSO;
class CBlackIPMonitor;
class CSecurityCardCenter;
class CAutoPunishRuleBlackIPMgr;
class CAutoPunishRuleHackTypeMgr;
class CCapchaDataManager;
}
class NPC_Teleport;
namespace StackableAction {
class Action;
}
namespace WongWork {
namespace IPG {
class CIPGHelper;
}
}
namespace random_option {
class CRandomOptionItemHandle;
}
namespace break_away_prevent {
class CBreakAwayPreventSystem;
}

// IDA_EA: GlobalData::Init @0x8299FA0, Fini @0x829E0B2,
// setCurrentPacketPool @0x829E4C4.
// IDA: GlobalData owns process-wide singletons used by df_game_r. This header
// currently exposes only the old-server base globals that have been restored
// into compilable source. Subsystem managers are added back only when their
// corresponding IDA units are restored.
class GlobalData {
public:
    // IDA_DATA: _ZN10GlobalData15g_ExtractionMgrE.  CMD416 reaches this
    // process-lifetime manager after expert-job scripts are initialized.
    static expert_extraction::CExtractionMgr* g_ExtractionMgr;
    // IDA_DATA: _ZN10GlobalData15s_GameMasterMgrE.
    static game_master::CGameMasterMgr* s_GameMasterMgr;
    // Keep existing source spelling while preserving the global old ABI type.
    typedef ::THREAD_TYPE THREAD_TYPE;
    enum {
        THREAD_DISPATCH = ::THREAD_DISPATCH,
        THREAD_ACCEPT = ::THREAD_ACCEPT,
        THREAD_NETWORK = ::THREAD_NETWORK,
        THREAD_EXCHANGE = ::THREAD_EXCHANGE,
        THREAD_MAIN = ::THREAD_MAIN,
        THREAD_UDP = ::THREAD_UDP,
        THREAD_MONITOR = ::THREAD_MONITOR,
        THREAD_DB = ::THREAD_DB,
        THREAD_LOGDB = ::THREAD_LOGDB,
        THREAD_FEATURE_DB = ::THREAD_FEATURE_DB,
    };

    enum {
        kSecuCipherKeyStringCount = 5000,
    };

    // IDA: GlobalData::Init @0x8299FA0.
    static bool Init();

    // IDA: GlobalData::InitCaptcha @0x829D438.
    static bool InitCaptcha();

    // IDA: GlobalData::GenerateCaptchaData @0x829D4B0.
    static void GenerateCaptchaData();

    // IDA: GlobalData::initTournamentDB @0x829DD82.
    static bool initTournamentDB(const char* file_name);

    // IDA: GlobalData::Fini @0x829E0B2.
    static bool Fini();

    // IDA: GlobalData::setCurrentPacketPool @0x829E4C4.
    static bool setCurrentPacketPool(THREAD_TYPE pool_type,
                                     unsigned long thread_id);

    // IDA globals: GlobalData::s_server_stop and GlobalData::s_systemTime_.
    static bool server_stop();
    static void set_server_stop(bool stop);
    static CSystemTime* system_time();
    static MsgQueueMgr* msgq_mgr();
    static PacketPool* packet_pool(THREAD_TYPE pool_type);
    static StreamPool* stream_pool();
    static SmallStreamPool* small_stream_pool();
    static BigStreamPool* big_stream_pool();

    // Old login/password protocol globals.  GlobalData::Init creates both
    // concrete CTEA objects with their original protocol keys before the
    // accept/network threads can dispatch a client packet.
    static CTEA* s_pcryptTEA_MousePassword_;
    static CTEA* s_pcryptTEA_SecurityCard_;

    // IDA data: the old 32-bit binary owns these canonical process globals.
    // _ZN10GlobalData13s_systemTime_E is a 20-byte embedded CSystemTime at
    // 0x941F714; the two pool/queue entries below are 4-byte pointers at
    // 0x940BD68 and 0x940BD6C.
    static CSystemTime s_systemTime_;
    static MsgQueueMgr* s_msgq_mgr;
    static StreamPool* s_stream_pool;
    // IDA_DATA: _ZN10GlobalData19s_capchaDataManagerE.
    // The old process embeds the real deque-backed captcha manager.
    static WongWork::CCapchaDataManager s_capchaDataManager;

    // IDA_EA: 0x941F7DC
    // IDA_NAME: _ZN10GlobalData27g_IPMonitorLastDBAccessTimeE
    // DEMANGLED: GlobalData::g_IPMonitorLastDBAccessTime
    // BSS time boundary consumed by DB_BlackIPMonitorPartLoad and updated by
    // Inter_BlackIPMonitor after it applies the final returned record.
    static time_t g_IPMonitorLastDBAccessTime;

    // IDA_EA: 0x941F7D8
    // IDA_NAME: _ZN10GlobalData16g_blackIPMonitorE
    // DEMANGLED: GlobalData::g_blackIPMonitor
    // The process-wide 0x18-byte CBlackIPMonitor is constructed/reset by
    // GlobalData::Init before the original BlackIP DB request is eligible to
    // be enabled.  Its response receivers live in the global provider.
    static WongWork::CBlackIPMonitor* g_blackIPMonitor;

    // IDA_EA: 0x941F7D0
    // IDA_NAME: _ZN10GlobalData26g_autoPunishRuleBlackIPMgrE
    // DEMANGLED: GlobalData::g_autoPunishRuleBlackIPMgr
    // Init allocates the exact 0x18-byte rule map, invokes C2, stores this
    // pointer, then clears it before the DB reload return can arrive.
    static WongWork::CAutoPunishRuleBlackIPMgr* g_autoPunishRuleBlackIPMgr;

    // IDA_EA: 0x941F7D4
    // IDA_NAME: _ZN10GlobalData27g_autoPunishRuleHackTypeMgrE
    // DEMANGLED: GlobalData::g_autoPunishRuleHackTypeMgr
    // Init allocates the real 0x70-byte rule manager, calls C2, stores this
    // pointer, then calls reset.  Its B-owned type is shared by DB/Inter and
    // hack/trade consumers; no opaque compatibility slot remains.
    static WongWork::CAutoPunishRuleHackTypeMgr* g_autoPunishRuleHackTypeMgr;

    // IDA_EA: GlobalData::Init @0x829CC7E..0x829CD71; Fini @0x829E34E.
    // IDA_NAME: _ZN10GlobalData22s_antibotBusinessImpl_E /
    //           _ZN10GlobalData17s_antibotChecker_E
    // The checker is owned by libantisvrimport.so for process lifetime.  Its
    // concrete callback object is the real four-byte CBusinessImpl created
    // immediately before the plug-in factory call; no synthetic checker
    // layout is declared in this executable.
    static CBusinessImpl* s_antibotBusinessImpl_;
    static IAntibotInterface* s_antibotChecker_;

    // Keep narrow access through the opaque plug-in boundary.  The helpers
    // are used by packet code after the exact Init lifecycle above runs.
    static IAntibotInterface* antibot_checker();
    static void set_antibot_checker(IAntibotInterface* checker);

    // IDA: GlobalData::s_secu_cipher_keystring_manager_ used by
    // InitCipherKeyString @0x82A025A and packet security-card paths.
    static Cipher* s_secu_cipher_keystring_manager_[kSecuCipherKeyStringCount];

    // IDA_EA: 0x941F70C
    // IDA_NAME: _ZN10GlobalData20s_securityCardCenterE
    // DEMANGLED: GlobalData::s_securityCardCenter
    // GlobalData::Init allocates the exact 0x488-byte CSecurityCardCenter,
    // stores this pointer, then calls its init() before security-card packet
    // dispatch and DB-return paths are used.
    static WongWork::CSecurityCardCenter* s_securityCardCenter;

    // IDA: GlobalData::s_db_mgr constructed in GlobalData::Init @0x8299FA0
    // and destroyed in GlobalData::Fini @0x829E0B2.
    static DBMgr* s_db_mgr;
    static CDBConnector* s_db_connector;
    // IDA_DATA: _ZN10GlobalData12s_psimpleSSOE.  The login/DB chain owns the
    // concrete SSO object while GlobalData owns its process-wide pointer.
    static WongWork::CSimpleSSO* s_psimpleSSO;

    // IDA data: the two 4-byte DBThread pointers are at 0x940BDB0 and
    // 0x940BDB4. Init constructs and configures the normal then log thread.
    static DBThread* s_db_thr;
    static DBThread* s_db_thr_for_logDB;
    static FeatureDBThread* s_feature_db_thread;
    // IDA_DATA: _ZN10GlobalData14s_dispatch_thrE.  The 12-byte worker owns
    // DISPATCH_Q and is constructed with the two DB workers in Init.
    static DispatchThread* s_dispatch_thr;
    // IDA data consumed by AcceptThread/NetworkThread.  The acceptor owns
    // socket creation; the network worker is the sole epoll registrar.
    static AcceptThread* s_accept_thr;
    static NetworkThread* s_net_thr;
    // IDA_DATA: _ZN10GlobalData9s_udp_thrE.  The process UDP listener owns
    // the Hades FE6/FE7 ingress subset until the full server-packet router
    // is restored.
    static UdpThread* s_udp_thr;
    // IDA_DATA: _ZN10GlobalData17s_monitor_tcp_thrE.  The restored subset
    // advances Hades TCP receive/send readiness only.
    static MonitorTcpThread* s_monitor_tcp_thr;
    static Mutex s_accept_lock;
    static std::queue<CUser*> s_accept_que;

    // IDA: GlobalData::s_pLogGameChannel allocated in GlobalData::Init
    // @0x829B503..0x829B53C, then StartLogTimer is called.
    static WongWork::CLogGameChannel* s_pLogGameChannel;

    // IDA: GlobalData::s_DailyScheduleManager allocated in GlobalData::Init
    // @0x829B730..0x829B754 and destroyed in GlobalData::Fini.
    static CDailyScheduleManager* s_DailyScheduleManager;

    // IDA: GlobalData::s_event_manager allocated in GlobalData::Init
    // @0x829C2D9..0x829C2FA and consumed by event runtime paths.
    static CEventManager* s_event_manager;
    // IDA_DATA: _ZN10GlobalData6gmListE.  Init creates this concrete
    // Gm_List before CGM_Manager so the original gm_manifest load has its
    // CGMAccounts prerequisite and channel-1 sync path.
    static Gm_List* gmList;
    // IDA_DATA: _ZN10GlobalData13s_secret_shopE.  Init constructs the
    // concrete 0x64-byte secretshop::CSecretShop before party dungeon start.
    static secretshop::CSecretShop* s_secret_shop;
    // IDA_DATA: _ZN10GlobalData12s_GM_ManagerE.  This is only the original
    // process-wide pointer slot; CGM_Manager construction and destruction
    // remain in the owner lifecycle.
    static CGM_Manager* s_GM_Manager;
    // IDA_DATA: _ZN10GlobalData23s_fatigueBatteryHandle_E.  Init allocates
    // the exact stateless one-byte service; old Fini does not destroy it.
    static CFatigueBatteryHandle* s_fatigueBatteryHandle_;
    // IDA_DATA: _ZN10GlobalData20s_pEvent_UpGrade_JarE.  B owns the concrete
    // PVF/event type and initialization; the process storage remains A-owned.
    static Event_Upgrade_Jar* s_pEvent_UpGrade_Jar;
    // IDA_DATA: _ZN10GlobalData15s_power_managerE.  C-owned world/PvP
    // state machines consume the typed manager; process storage remains in
    // the global runtime provider.
    static CPowerManager* s_power_manager;
    static expert_job::CExpertJobMgr* s_ExpertJobMgr;
    static online_preliminary::COnlinePreliminaryTeamMgr*
        s_onlinePreliminaryTeamMgr;
    static CRijndael* s_pcryptRijndael_MousePassword_;
    static ExtreamDungeon::CCompound_ExtreamDun_Item* s_pCompoundBindCube;
    static CHadesServerProxy* s_hades_proxy;
    static int g_GoldControlDateFlag;

    // IDA_DATA: _ZN10GlobalData20s_pTowerOfDespairMgrE.  CDataManager::init
    // allocates the exact 0x35C-byte manager and TOD character state reads its
    // minimum-level field through the canonical typed pointer.
    static TowerOfDespairMgr* s_pTowerOfDespairMgr;

public:
    static InstanceRentalSystem* s_rentalSystem;
    static CGlobalEffectManager* s_globalEffectManager_;
    static CAuctionServerProxy* s_auction_proxy;
    static CCeraAuctionServerProxy* s_cera_auction_proxy;
    // IDA_EA: GlobalData::Init @0x8299FA0 writes old
    // _ZN10GlobalData11s_timeGate_E after allocating CTimeGate.
    static CTimeGate* s_timeGate_;
    // IDA_DATA: _ZN10GlobalData18s_villageObjectMgrE.  The village-object
    // registry is a real typed process singleton used by private-store
    // creation/removal and Area time-gate registration.
    static village_object::CVillageObjectMgr* s_villageObjectMgr;
    // IDA_EA: GlobalData::Init/Fini @0x8299FA0/@0x829E0B2 allocate and
    // destroy old _ZN10GlobalData15s_ref_pvp_gradeE.
    static RefPvpGrade* s_ref_pvp_grade;
    // IDA_EA: GlobalData::Init @0x8299FA0 writes these old globals in the
    // CeraShop/monitor-proxy branches. A currently exposes storage only; the
    // concrete manager/object lifecycle remains owner work.
    static CServerProxyMgr<CMonitorServerProxy>* s_monitor_proxy_mgr;
    // IDA_DATA: _ZN10GlobalData17s_guild_proxy_mgrE.  Init/Fini own the
    // concrete CServerProxyMgr<CGuildServerProxy>; callers select the real
    // per-server-group proxy through its typed member ABI.
    static CServerProxyMgr<CGuildServerProxy>* s_guild_proxy_mgr;
    // IDA_DATA: _ZN10GlobalData21s_statistic_proxy_mgrE. Statistic packet
    // producers select their per-server-group proxy through this manager.
    static CServerProxyMgr<CStatisticServerProxy>* s_statistic_proxy_mgr;
    // IDA_DATA: _ZN10GlobalData15s_channel_proxyE. The channel subsystem
    // owns construction from the real channel configuration; MainLoop only
    // drives its periodic information send through this typed pointer.
    static CmmChannelProxy* s_channel_proxy;
    // IDA_DATA: _ZN10GlobalData17s_community_proxyE.  PvP buddy and
    // cross-channel message paths share this single TCP proxy.
    static CCommunityServerProxy* s_community_proxy;
    // IDA_DATA: _ZN10GlobalData14s_pcroom_proxyE.  The object is a real C
    // gameplay-side CPCRoomServerProxy, created from the configured endpoint
    // by GlobalData::Init.  Old GlobalData::Fini does not destroy this slot.
    static CPCRoomServerProxy* s_pcroom_proxy;
    // IDA_EA: 0x941F734
    // IDA_NAME: _ZN10GlobalData12s_pIPGHelperE
    static WongWork::IPG::CIPGHelper* s_pIPGHelper;

    // IDA_DATA: _ZN10GlobalData9g_ActionsE.  Init creates this 0x30-byte
    // StackableAction router before the teleport/random-option block; CMD516
    // calls the object directly and must never synthesize a per-packet table.
    static StackableAction::Action* g_Actions;

    // IDA_DATA: _ZN10GlobalData11g_pTeleportE.  Init creates the concrete
    // NPC_Teleport before the anonymous User_Teleport peer and before random
    // option services; CMD471 selects this object for NPC destinations.
    static NPC_Teleport* g_pTeleport;

    // IDA_EA: GlobalData::Init @0x8299FA0 writes this pointer after it
    // constructs the 0x18-byte random-option handle. The object is consumed
    // by CeraShop auto-unseal and random-option packet dispatch paths.
    // IDA_NAME: _ZN10GlobalData20g_randomOptionHandleE
    // DEMANGLED: GlobalData::g_randomOptionHandle
    static random_option::CRandomOptionItemHandle* g_randomOptionHandle;

    // IDA_DATA: _ZN10GlobalData14s_statisticMgrE.  GlobalData::Init
    // allocates the 0x30-byte CStatisticMgr and stores its pointer here.
    static CStatisticMgr* s_statisticMgr;
    // IDA_DATA: _ZN10GlobalData14s_BreakAwaySysE.  The old init chain creates
    // the 0x32c manager immediately after CStatisticMgr; DB signal 321 and
    // Inter_LoginInfo consume this exact process-global provider.
    static break_away_prevent::CBreakAwayPreventSystem* s_BreakAwaySys;
    // IDA_EA: GlobalData::Init @0x8299FA0 allocates the 0x10-byte UDP proxy,
    // invokes Init, and Fini releases it.
    // IDA_DATA: _ZN10GlobalData20s_double_check_proxyE
    static CDoubleConnCheckServerProxy* s_double_check_proxy;
    // IDA_EA: GlobalData::Init @0x8299FA0 conditionally allocates the
    // 0x10-byte school UDP proxy from a non-empty IP and positive port, then
    // calls Init.  GlobalData::Fini has no reference to this old pointer.
    // IDA_DATA: _ZN10GlobalData14s_school_proxyE
    static CSchoolServerProxy* s_school_proxy;
    static WongWork::CCeraShop* s_pCeraShop;
    // IDA_DATA: _ZN10GlobalData17s_pCompoundAvatarE.  DataManager owns the
    // concrete object and creates it before importing CompoundAvatar.etc.
    static WongWork::CCompoundAvatar* s_pCompoundAvatar;
    // IDA_EA: GlobalData::Init/Fini @0x8299FA0/@0x829E0B2 allocate and
    // destroy old _ZN10GlobalData14s_pGMAccounts_E.
    static WongWork::CGMAccounts* s_pGMAccounts_;
    static ItemVendingMachine* s_pItemVendingMachine;
    // IDA_EA: 0x941F7C8
    // IDA_NAME: _ZN10GlobalData18s_pSecuDataControlE
    // DEMANGLED: GlobalData::s_pSecuDataControl
    static Secu_DataControl* s_pSecuDataControl;
    static CSecu_ProtectionField* s_pSecuProtectionField;
    static CItemAmplifier* s_itemAmplifier_;

    // IDA_EA: 0x941F824
    // IDA_NAME: _ZN10GlobalData17s_pAvatarRouletteE
    // DEMANGLED: GlobalData::s_pAvatarRoulette
    // The ARAD internal-return route passes this typed process object to
    // AvatarRouletteServer::Pickup; ownership and construction remain in the
    // PVF/DataManager lifecycle.
    static AvatarRouletteServer* s_pAvatarRoulette;

    // IDA_EA: 0x941F7F0
    // IDA_NAME: _ZN10GlobalData21s_serverStateManager_E
    // DEMANGLED: GlobalData::s_serverStateManager_
    // The channel-owned ARAD state manager is constructed by the original
    // GlobalData lifecycle; this declaration provides only its typed ABI slot.
    static ARAD::Arad_ServerStateManager* s_serverStateManager_;

    // IDA_DATA: _ZN10GlobalData19s_revengeDungeonMgrE.  Gameplay owns the
    // pointed-to CRevengeDungeon state; GlobalData only owns the process-wide
    // storage/lifetime landing.
    static village_attacked::CRevengeDungeon* s_revengeDungeonMgr;

    // IDA_DATA: _ZN10GlobalData19s_villageMonsterMgrE.  This is the typed
    // village-attacked manager pointer consumed by character-load packets and
    // world/event flows; declaring it here removes the former void* bridge.
    static village_attacked::CVillageMonsterMgr* s_villageMonsterMgr;
};

// IDA data: dword_940BE84 / dword_940BE88 are 16-byte-stride key group/index
// records consumed by Sanicova::CPad::GetKeyIdx @0x8599872.
extern int dword_940BE84[GlobalData::kSecuCipherKeyStringCount * 4];
extern int* const dword_940BE88;

// IDA globals selected by SelectKeyString @0x82A002A. Their population remains
// tied to the real RDAR/PVF script-list import units; A exposes only the public
// global landing and fail-closed selection behavior.
extern STScriptFileList g_stStackableScriptFileList;
extern STScriptFileList g_stEquipmentScriptFileList;
extern STScriptFileList g_stMonsterScriptFileList;
extern STScriptFileList g_questScriptFileList;
extern STScriptFileList g_stPassiveObjectScriptFileList;


#endif  // OLDSRV_GLOBAL_GLOBAL_DATA_H_
