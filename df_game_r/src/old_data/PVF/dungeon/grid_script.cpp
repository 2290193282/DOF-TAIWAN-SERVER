#include "old_data/PVF/dungeon/grid_script.h"


typedef char GridScript_Size_Must_Be_0x28[(sizeof(GridScript) == 0x28) ? 1 : -1];

// IDA_EA: 0x83D281C
// IDA_NAME: _ZN10GridScriptC2ERKS_
// DEMANGLED: GridScript::GridScript(GridScript const&)
GridScript::GridScript(const GridScript& rhs)
    : grid_type(rhs.grid_type),
      values_04(rhs.values_04),
      values_10(rhs.values_10),
      values_1c(rhs.values_1c) {}

// IDA_EA: 0x83DC5BE
// IDA_NAME: _ZN10GridScriptD2Ev
// DEMANGLED: GridScript::~GridScript()
GridScript::~GridScript() {}

// IDA_EA: 0x83ED958
// IDA_NAME: _ZN10GridScriptaSERKS_
// DEMANGLED: GridScript::operator=(GridScript const&)
GridScript& GridScript::operator=(const GridScript& rhs) {
    grid_type = rhs.grid_type;
    values_04 = rhs.values_04;
    values_10 = rhs.values_10;
    values_1c = rhs.values_1c;
    return *this;
}

// IDA_EA: 0x88F2902
// IDA_NAME: _ZN10GridScript5clearEv
// DEMANGLED: GridScript::clear(void)
void GridScript::clear() {
    grid_type = 0;
    values_04.clear();
    values_10.clear();
    values_1c.clear();
}

// IDA_EA: 0x88F293E
// IDA_NAME: _ZN10GridScriptC2Ev
// DEMANGLED: GridScript::GridScript(void)
GridScript::GridScript() : grid_type(0), values_04(), values_10(), values_1c() {
    clear();
}
