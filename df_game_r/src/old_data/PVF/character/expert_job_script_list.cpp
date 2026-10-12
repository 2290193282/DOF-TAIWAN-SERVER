#include "old_data/PVF/character/expert_job_script_list.h"

#include "old_data/PVF/common/rdar_script.h"

#include <cstdio>
#include <cstring>
#include <new>
#include <utility>


STScriptFileList g_expertJobScriptFileList;
char g_expertJobScriptBaseDirectory[0x100];

// IDA_EA: 0x896D830
// IDA_NAME: _Z19initExpertJobScriptPcS_
// DEMANGLED: initExpertJobScript(char*,char*)
bool initExpertJobScript(char* base_dir, char* filename) {
    std::strncpy(g_expertJobScriptBaseDirectory, base_dir, 0x100);
    if (g_expertJobScriptBaseDirectory[
            std::strlen(g_expertJobScriptBaseDirectory) - 1] != '\\' &&
        g_expertJobScriptBaseDirectory[
            std::strlen(g_expertJobScriptBaseDirectory) - 1] != '/') {
        const size_t length =
            std::strlen(g_expertJobScriptBaseDirectory);
        std::memcpy(g_expertJobScriptBaseDirectory + length, "\\", 2);
    }
    return loadRDARScriptListFile(&g_expertJobScriptFileList,
                                  g_expertJobScriptBaseDirectory,
                                  filename,
                                  0) == 1;
}

// IDA_EA: 0x896D8EC
// IDA_NAME: _ZN14CExpertJobList4initEv
// DEMANGLED: CExpertJobList::init(void)
bool CExpertJobList::init() {
    const char* filename = 0;
    for (STScriptFileList::iterator it = g_expertJobScriptFileList.begin();
         it != g_expertJobScriptFileList.end(); ++it) {
        filename = it->second.c_str();
        if (filename == 0) {
            break;
        }

        STExpertJobScript* script =
            new (::operator new(sizeof(STExpertJobScript))) STExpertJobScript();
        if (ImportExpertJobScript(script, filename) != 1) {
            std::printf("expert job Script Error: %s\n", filename);
            script->~STExpertJobScript();
            ::operator delete(script);
            return false;
        }

        const int index =
            g_expertJobScriptFileList.FindIndexByFullScan(filename);
        expert_job_scripts.insert(std::make_pair(index, script));
    }

    if (ImportExpertJobEtcScript(etc_script, "expertjob.etc") == 1) {
        return true;
    }
    std::printf("ExpertJob ETC Script Error : %s\n", filename);
    return false;
}

// IDA_EA: 0x896DAC6
// IDA_NAME: _ZN14CExpertJobList7destroyEv
// DEMANGLED: CExpertJobList::destroy(void)
bool CExpertJobList::destroy() {
    for (std::map<int, STExpertJobScript*>::iterator it =
             expert_job_scripts.begin();
         it != expert_job_scripts.end(); ++it) {
        STExpertJobScript* script = it->second;
        if (script != 0) {
            script->~STExpertJobScript();
            ::operator delete(script);
        }
    }
    return false;
}
