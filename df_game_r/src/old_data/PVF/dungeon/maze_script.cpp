#include "old_data/PVF/dungeon/maze_script.h"

// IDA_EA: 0x822ADC2
// IDA_NAME: _ZNK10MazeScript8getGridREii
// DEMANGLED: MazeScript::getGridR(int,int)const
const GridScript& MazeScript::getGridR(int x, int y) const {
    return grids[x + y * grid_width];
}

// IDA_EA: 0x8376D8A
// IDA_NAME: _ZN10MazeScriptC2ERKS_
// DEMANGLED: MazeScript::MazeScript(MazeScript const&)
MazeScript::MazeScript(const MazeScript& rhs)
    : grid_width(rhs.grid_width),
      grid_height(rhs.grid_height),
      grids(rhs.grids),
      values_14(rhs.values_14),
      values_20(rhs.values_20),
      value_2c(rhs.value_2c),
      value_30(rhs.value_30),
      value_34(rhs.value_34),
      ridable(rhs.ridable),
      value_4c(rhs.value_4c),
      value_50(rhs.value_50),
      value_54(rhs.value_54),
      clear_conditions(rhs.clear_conditions),
      maze_index(rhs.maze_index),
      value_68(rhs.value_68),
      value_6c(rhs.value_6c),
      value_70(rhs.value_70) {}

// IDA_EA: 0x8376F00
// IDA_NAME: _ZN10MazeScriptD2Ev
// DEMANGLED: MazeScript::~MazeScript()
MazeScript::~MazeScript() {}

// IDA_EA: 0x83A4832
// IDA_NAME: _ZN10MazeScriptaSERKS_
// DEMANGLED: MazeScript::operator=(MazeScript const&)
MazeScript& MazeScript::operator=(const MazeScript& rhs) {
    grid_width = rhs.grid_width;
    grid_height = rhs.grid_height;
    grids = rhs.grids;
    values_14 = rhs.values_14;
    values_20 = rhs.values_20;
    value_2c = rhs.value_2c;
    value_30 = rhs.value_30;
    value_34 = rhs.value_34;
    ridable = rhs.ridable;
    value_4c = rhs.value_4c;
    value_50 = rhs.value_50;
    value_54 = rhs.value_54;
    clear_conditions = rhs.clear_conditions;
    maze_index = rhs.maze_index;
    value_68 = rhs.value_68;
    value_6c = rhs.value_6c;
    value_70 = rhs.value_70;
    return *this;
}

// IDA_EA: 0x88F2A76
// IDA_NAME: _ZN10MazeScript5clearEv
// DEMANGLED: MazeScript::clear(void)
MazeScript* MazeScript::clear() {
    grid_width = 1;
    grid_height = 1;
    grids.clear();
    values_14.clear();
    values_20.clear();
    value_2c = -1;
    value_30 = -1;
    value_34 = -1;
    value_4c = -1;
    value_50 = -1;
    value_54 = -1;
    clear_conditions.clear();
    maze_index = 0;
    value_68 = 0;
    value_6c = 0;
    value_70 = 0;
    ridable.clear();
    return this;
}

// IDA_EA: 0x88F2B4A
// IDA_NAME: _ZN10MazeScriptC2Ev
// DEMANGLED: MazeScript::MazeScript(void)
MazeScript::MazeScript()
    : grid_width(1), grid_height(1), grids(), values_14(), values_20(),
      value_2c(-1), value_30(-1), value_34(-1), ridable(), value_4c(-1),
      value_50(-1), value_54(-1), clear_conditions(), maze_index(0),
      value_68(0), value_6c(0), value_70(0) {
    clear();
}

// IDA_EA: 0x88F2C2A
// IDA_NAME: _ZN10MazeScript7getGridEii
// DEMANGLED: MazeScript::getGrid(int,int)
GridScript& MazeScript::getGrid(int x, int y) {
    return grids[x + y * grid_width];
}

// IDA_EA: 0x88F29D2
// IDA_NAME: _ZN21DungeonClearCondition5clearEv
// DEMANGLED: DungeonClearCondition::clear(void)
DungeonClearCondition* DungeonClearCondition::clear() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    return this;
}

// IDA_EA: 0x88F29F4
// IDA_NAME: _ZN21DungeonClearConditionC2Ev
// DEMANGLED: DungeonClearCondition::DungeonClearCondition(void)
DungeonClearCondition::DungeonClearCondition() {
    clear();
}

// IDA_EA: 0x88F7CD0
// IDA_NAME: _ZN21DungeonClearConditionD2Ev
// DEMANGLED: DungeonClearCondition::~DungeonClearCondition()
DungeonClearCondition::~DungeonClearCondition() {}

// IDA_EA: 0x830E468
// IDA_NAME: _ZN13RidableObjectC2Ev
// DEMANGLED: RidableObject::RidableObject(void)
RidableObject::RidableObject() {
    clear();
}

// IDA_EA: 0x830E47C
// IDA_NAME: _ZN13RidableObject5clearEv
// DEMANGLED: RidableObject::clear(void)
RidableObject* RidableObject::clear() {
    value_00 = -1;
    value_04 = -1;
    value_08 = -1;
    value_0c = -1;
    value_10 = -1;
    value_14 = 200;
    return this;
}

// IDA_EA: 0x8312844
// IDA_NAME: _ZN13RidableObjectD2Ev
// DEMANGLED: RidableObject::~RidableObject()
RidableObject::~RidableObject() {}

// IDA_EA: 0x836A18A
// IDA_NAME: _ZN13RidableScriptD2Ev
// DEMANGLED: RidableScript::~RidableScript()
RidableScript::~RidableScript() {}

// IDA_EA: 0x8376D52
// IDA_NAME: _ZN13RidableScriptC2ERKS_
// DEMANGLED: RidableScript::RidableScript(RidableScript const&)
RidableScript::RidableScript(const RidableScript& rhs)
    : value_00(rhs.value_00),
      value_04(rhs.value_04),
      objects(rhs.objects) {}

// IDA_EA: 0x83A47F8
// IDA_NAME: _ZN13RidableScriptaSERKS_
// DEMANGLED: RidableScript::operator=(RidableScript const&)
RidableScript& RidableScript::operator=(const RidableScript& rhs) {
    value_00 = rhs.value_00;
    value_04 = rhs.value_04;
    objects = rhs.objects;
    return *this;
}

// IDA_EA: 0x88F2A08
// IDA_NAME: _ZN13RidableScriptC2Ev
// DEMANGLED: RidableScript::RidableScript(void)
RidableScript::RidableScript() : objects() {
    clear();
}

// IDA_EA: 0x88F2A50
// IDA_NAME: _ZN13RidableScript5clearEv
// DEMANGLED: RidableScript::clear(void)
RidableScript* RidableScript::clear() {
    value_00 = 0;
    value_04 = 0;
    objects.clear();
    return this;
}
