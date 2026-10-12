#ifndef OLDSRV_GLOBAL_EXPERT_JOB_SCRIPT_ABI_H_
#define OLDSRV_GLOBAL_EXPERT_JOB_SCRIPT_ABI_H_

#include "old_game/progression/quest/quest_skill_expert_job.h"

// CDataManager::init @0x8358D67 owns a real CExpertJobList allocation.  Its
// 0x48 i386 ABI is asserted at the type provider; callers allocate the type
// itself rather than repeating decompiler-recovered byte counts.

// IDA_EA: 0x8371796 / 0x83717FE / 0x849F47C
// The real STExpertJobEtcScript methods are declared by the owning gameplay
// header and implemented one-for-one in quest_skill_expert_job.cpp.


#endif  // OLDSRV_GLOBAL_EXPERT_JOB_SCRIPT_ABI_H_
