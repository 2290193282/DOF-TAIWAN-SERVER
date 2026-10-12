#ifndef OLDSRV_GLOBAL_GUILD_POWER_PARAMETER_SCRIPT_ABI_H_
#define OLDSRV_GLOBAL_GUILD_POWER_PARAMETER_SCRIPT_ABI_H_


// Declaration-only A/public bridge to B-owned Guild/Power script values.
// CDataManager::CDataManager @0x8355B30 constructs these embedded objects at
// +0xA50C and +0xA628; CDataManager::~CDataManager @0x8356B68 destroys them
// in reverse order. RDAR importers and behavior stay in 12_pvf_datamanager.
class GuildParameterScript;
class PowerParameterScript;


#endif  // OLDSRV_GLOBAL_GUILD_POWER_PARAMETER_SCRIPT_ABI_H_
