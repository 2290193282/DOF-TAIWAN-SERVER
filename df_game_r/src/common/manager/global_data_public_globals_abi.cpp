#include "common/manager/global_data.h"

#include "common/manager/gm_list_abi.h"
#include "old_game/gm/core/cgm_manager_abi.h"
#include "common/manager/fatigue_battery_handle_abi.h"
#include "infra/security/policy/hades_server_proxy_abi.h"
#include "infra/security/crypto/global_data_legacy_crypto.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"

// Public old GlobalData pointer slots. These definitions provide the original
// data-symbol ABI; concrete process-lifetime construction remains with the
// owning GlobalData lifecycle.

// IDA_DATA: GlobalData::s_villageObjectMgr
// IDA_NAME: _ZN10GlobalData18s_villageObjectMgrE
// DEMANGLED: GlobalData::s_villageObjectMgr
village_object::CVillageObjectMgr* GlobalData::s_villageObjectMgr = 0;

// IDA_DATA: GlobalData::s_villageMonsterMgr
// IDA_NAME: _ZN10GlobalData19s_villageMonsterMgrE
// DEMANGLED: GlobalData::s_villageMonsterMgr
village_attacked::CVillageMonsterMgr* GlobalData::s_villageMonsterMgr = 0;

// IDA_DATA: GlobalData::s_ExpertJobMgr
// IDA_NAME: _ZN10GlobalData14s_ExpertJobMgrE
// DEMANGLED: GlobalData::s_ExpertJobMgr
expert_job::CExpertJobMgr* GlobalData::s_ExpertJobMgr = 0;

// IDA_DATA: GlobalData::s_onlinePreliminaryTeamMgr
// IDA_NAME: _ZN10GlobalData26s_onlinePreliminaryTeamMgrE
// DEMANGLED: GlobalData::s_onlinePreliminaryTeamMgr
online_preliminary::COnlinePreliminaryTeamMgr*
    GlobalData::s_onlinePreliminaryTeamMgr = 0;

// IDA_DATA: GlobalData::s_GM_Manager
// IDA_NAME: _ZN10GlobalData12s_GM_ManagerE
// DEMANGLED: GlobalData::s_GM_Manager
CGM_Manager* GlobalData::s_GM_Manager = 0;

// IDA_DATA: GlobalData::s_power_manager
// IDA_NAME: _ZN10GlobalData15s_power_managerE
// DEMANGLED: GlobalData::s_power_manager
CPowerManager* GlobalData::s_power_manager = 0;

// IDA_DATA: GlobalData::s_fatigueBatteryHandle_
// IDA_NAME: _ZN10GlobalData23s_fatigueBatteryHandle_E
// DEMANGLED: GlobalData::s_fatigueBatteryHandle_
CFatigueBatteryHandle* GlobalData::s_fatigueBatteryHandle_ = 0;


// IDA_DATA: GlobalData::s_pcryptRijndael_MousePassword_
// IDA_NAME: _ZN10GlobalData31s_pcryptRijndael_MousePassword_E
// DEMANGLED: GlobalData::s_pcryptRijndael_MousePassword_
CRijndael* GlobalData::s_pcryptRijndael_MousePassword_ = 0;

// IDA_DATA: GlobalData::gmList
// IDA_NAME: _ZN10GlobalData6gmListE
// DEMANGLED: GlobalData::gmList
Gm_List* GlobalData::gmList = 0;

// IDA_DATA: GlobalData::s_pCompoundBindCube
// IDA_NAME: _ZN10GlobalData19s_pCompoundBindCubeE
// DEMANGLED: GlobalData::s_pCompoundBindCube
ExtreamDungeon::CCompound_ExtreamDun_Item*
    GlobalData::s_pCompoundBindCube = 0;

// IDA_DATA: GlobalData::s_pEvent_UpGrade_Jar
// IDA_NAME: _ZN10GlobalData20s_pEvent_UpGrade_JarE
// DEMANGLED: GlobalData::s_pEvent_UpGrade_Jar
Event_Upgrade_Jar* GlobalData::s_pEvent_UpGrade_Jar = 0;

// IDA_DATA: GlobalData::s_pCompoundAvatar
// IDA_NAME: _ZN10GlobalData17s_pCompoundAvatarE
// DEMANGLED: GlobalData::s_pCompoundAvatar
WongWork::CCompoundAvatar* GlobalData::s_pCompoundAvatar = 0;

// IDA_DATA: GlobalData::s_pAvatarRoulette
// IDA_NAME: _ZN10GlobalData17s_pAvatarRouletteE
// DEMANGLED: GlobalData::s_pAvatarRoulette
AvatarRouletteServer* GlobalData::s_pAvatarRoulette = 0;

// IDA_DATA: GlobalData::s_serverStateManager_
// IDA_NAME: _ZN10GlobalData21s_serverStateManager_E
// DEMANGLED: GlobalData::s_serverStateManager_
ARAD::Arad_ServerStateManager* GlobalData::s_serverStateManager_ = 0;

// IDA_DATA: GlobalData::s_channel_proxy
// IDA_NAME: _ZN10GlobalData15s_channel_proxyE
// DEMANGLED: GlobalData::s_channel_proxy
CmmChannelProxy* GlobalData::s_channel_proxy = 0;

// IDA_DATA: GlobalData::s_hades_proxy
// IDA_NAME: _ZN10GlobalData13s_hades_proxyE
// DEMANGLED: GlobalData::s_hades_proxy
CHadesServerProxy* GlobalData::s_hades_proxy = 0;

// IDA_DATA: GlobalData::g_GoldControlDateFlag
// IDA_NAME: _ZN10GlobalData21g_GoldControlDateFlagE
// DEMANGLED: GlobalData::g_GoldControlDateFlag
int GlobalData::g_GoldControlDateFlag = 0;
