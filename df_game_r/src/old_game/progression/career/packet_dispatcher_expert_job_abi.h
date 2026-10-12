#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_JOB_ABI_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_JOB_ABI_H_

class CUser;
class MSG_STATIC_COMMAND;
enum ENUM_EXPERT_JOB_TYPE : int;

namespace expert_job {
class CExpertJob;
class CExpertJobMgr;
}

// Declaration-only old-server ABI boundary for the expert-job manager and
// enchanter services consumed by inventory CMD172. No CExpertJobMgr or
// CEnchanter layout/provider is reconstructed here.
namespace oldsrv_expert_job_dispatch_abi {

// Old-server callee EA 0x822F8D4,
// _ZNK15CUserCharacInfo21GetCurCharacExpertJobEv.
// CUser's CUserCharacInfo base is proven at offset zero for this dispatch
// surface. The current broad CUser header exposes this pointer-valued member
// as int, so this exact-name declaration avoids any integer conversion.
// IDA global _ZN10GlobalData14s_ExpertJobMgrE.

// Old-server callee EA 0x849E1DC,
// _ZN10expert_job13CExpertJobMgr16AcquireExpertJobE20ENUM_EXPERT_JOB_TYPE.
// CEnchanter::C2 at 0x849F2C2 constructs CExpertJob at receiver+0. These
// aliases accept that proven base pointer as the exact physical receiver
// address, without declaring the CEnchanter derived layout.
// Old-server virtual target EA 0x849C7B6,
// _ZN10expert_job10CEnchanter14OnStartMissionEP5CUserS2_.
// Old-server callee EA 0x849DFCC,
// _ZN10expert_job10CEnchanter12OnTryEnchantEP5CUserP18MSG_STATIC_COMMAND.
}  // namespace oldsrv_expert_job_dispatch_abi

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_JOB_ABI_H_
