#include "old_game/player/core/character_growth_tables.h"

#include "old_data/PVF/character/datamanager/character_script_runtime.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"

#include <stdio.h>

extern STScriptFileList g_stCharacterScriptFileList;

// IDA_EA: 0x8349514
// IDA_NAME: _ZN11CCharacList4initEv
// DEMANGLED: CCharacList::init(void)
int CCharacList::init() {
    STCharacterScript script;

    BeginCharacterCreateEquipmentLoad();
    int result = 1;
    for (STScriptFileList::iterator it = g_stCharacterScriptFileList.begin();
         it != g_stCharacterScriptFileList.end(); ++it) {
        const char* const filename = it->second.c_str();
        if (!filename) {
            break;
        }
        if (ImportCharacterScript(&script, filename) != 1) {
            printf("Charac Script Error: %s\n", filename);
            result = 0;
            break;
        }
        characters_[script.character_job_].set_data(&script);
    }

    if (result == 1) {
        CommitCharacterCreateEquipmentLoad();
    } else {
        AbortCharacterCreateEquipmentLoad();
    }
    return result;
}
