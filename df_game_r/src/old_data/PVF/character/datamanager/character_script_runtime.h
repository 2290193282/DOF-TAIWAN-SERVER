#ifndef OLDSRV_DATAMANAGER_CHARACTER_SCRIPT_RUNTIME_H_
#define OLDSRV_DATAMANAGER_CHARACTER_SCRIPT_RUNTIME_H_

#include "old_game/player/core/character_script_growth_data.h"

#include <cstdio>
#include <stdint.h>
#include <utility>
#include <vector>


struct CharacterCreateEquipmentEntry {
    CharacterCreateEquipmentEntry();
    CharacterCreateEquipmentEntry(int32_t equipment_type_value,
                                  int32_t item_id_value);

    int32_t equipment_type;
    int32_t item_id;
};

// Character.lst loading is transactional: every imported job writes to the
// staging table and only CCharacList::init publishes it after the complete
// list succeeds.  Standalone imports publish one proven script atomically.
void BeginCharacterCreateEquipmentLoad();
void CommitCharacterCreateEquipmentLoad();
void AbortCharacterCreateEquipmentLoad();
bool FindCharacterCreateEquipment(int character_job,
                                  int equipment_type,
                                  int* item_id);

int ImportCharacterScript(STCharacterScript* script, const char* filename);
int exportCharacterScript(STCharacterScript* script, const char* src);
int exportCharacterStatusScript(FILE* stream, STRDCharacterStatus* status);
size_t exportCharacterGiveSkillScript(
    FILE* stream,
    std::vector<std::pair<int, int> >* skills);

#endif  // OLDSRV_DATAMANAGER_CHARACTER_SCRIPT_RUNTIME_H_
