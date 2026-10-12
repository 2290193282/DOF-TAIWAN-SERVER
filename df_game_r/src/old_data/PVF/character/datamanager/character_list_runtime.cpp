#include "old_game/player/core/character_growth_tables.h"
#include "old_game/player/core/character_script_growth_data.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"

#include <stdio.h>


extern STScriptFileList g_stCharacterScriptFileList;

extern int ImportCharacterScript(STCharacterScript* script, const char* filename)
    __asm__("_Z21ImportCharacterScriptP17STCharacterScriptPKc");
