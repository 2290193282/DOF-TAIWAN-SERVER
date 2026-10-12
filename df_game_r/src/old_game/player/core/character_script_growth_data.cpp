#include "old_game/player/core/character_script_growth_data.h"


typedef char STSecondGrowTypeData_Size_Must_Be_0x100[
    (sizeof(STCharacterScript::STSecondGrowTypeData) == 0x100) ? 1 : -1];

// IDA_EA: 0x83678A0
// IDA_NAME: _ZN17STCharacterScript20STSecondGrowTypeDataC2Ev
// DEMANGLED: STCharacterScript::STSecondGrowTypeData::STSecondGrowTypeData(void)
STCharacterScript::STSecondGrowTypeData::STSecondGrowTypeData() {
    clear();
}

// IDA_EA: 0x8367A28
// IDA_NAME: _ZN17STCharacterScript20STSecondGrowTypeData5clearEv
// DEMANGLED: STCharacterScript::STSecondGrowTypeData::clear(void)
bool STCharacterScript::STSecondGrowTypeData::clear() {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 1;
        if (index > 1) {
            break;
        }
        status[index].Clear();
        skill_values[index].clear();
        etc_stats[index].clear();
    }
    return result;
}

// IDA_EA: 0x8367E3E
// IDA_NAME: _ZN17STCharacterScript20STSecondGrowTypeDataD2Ev
// DEMANGLED: STCharacterScript::STSecondGrowTypeData::~STSecondGrowTypeData()
STCharacterScript::STSecondGrowTypeData::~STSecondGrowTypeData() {}
