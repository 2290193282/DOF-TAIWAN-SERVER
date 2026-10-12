#ifndef OLDSRV_OLD_DATA_PVF_CHARACTER_EXPERT_JOB_SCRIPT_LIST_H_
#define OLDSRV_OLD_DATA_PVF_CHARACTER_EXPERT_JOB_SCRIPT_LIST_H_

#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"


extern STScriptFileList g_expertJobScriptFileList;
extern char g_expertJobScriptBaseDirectory[0x100];

// IDA_EA: 0x896B0E4 / 0x896AE21.  These are the binary-RDAR importers used
// by CExpertJobList::init; their complete PVF provider is B-owned.
int ImportExpertJobScript(STExpertJobScript* script,
                          const char* filename)
    __asm__("_Z21ImportExpertJobScriptP17STExpertJobScriptPKc");
int ImportExpertJobEtcScript(STExpertJobEtcScript& script,
                             const char* filename)
    __asm__("_Z24ImportExpertJobEtcScriptR20STExpertJobEtcScriptPKc");

// IDA_EA: 0x896D830
// IDA_NAME: _Z19initExpertJobScriptPcS_
// DEMANGLED: initExpertJobScript(char*,char*)
bool initExpertJobScript(char* base_dir, char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_CHARACTER_EXPERT_JOB_SCRIPT_LIST_H_
