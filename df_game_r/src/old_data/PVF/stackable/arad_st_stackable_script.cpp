#include "old_data/PVF/stackable/arad_st_stackable_script.h"


// IDA_EA: 0x89C33FA
// IDA_NAME: _ZN22Arad_STStackableScriptC2Ev
// DEMANGLED: Arad_STStackableScript::Arad_STStackableScript(void)
Arad_STStackableScript::Arad_STStackableScript()
    : value_08(), value_0C(), value_10() {
    AradStClear();
}

// IDA_EA: 0x89C348E
// IDA_NAME: _ZN22Arad_STStackableScript11AradStClearEv
// DEMANGLED: Arad_STStackableScript::AradStClear(void)
Arad_STStackableScript* Arad_STStackableScript::AradStClear() {
    value_00 = 0;
    value_04 = 0xFFu;
    value_08.clear();
    value_0C.clear();
    value_10.clear();
    return this;
}

// IDA_EA: 0x89C3A46
// IDA_NAME: _ZN22Arad_STStackableScriptD2Ev
// DEMANGLED: Arad_STStackableScript::~Arad_STStackableScript()
Arad_STStackableScript::~Arad_STStackableScript() {}

// IDA_EA: 0x89C4052
// IDA_NAME: _ZN22Arad_STStackableScriptaSERKS_
// DEMANGLED: Arad_STStackableScript::operator=(Arad_STStackableScript const&)
Arad_STStackableScript& Arad_STStackableScript::operator=(
    const Arad_STStackableScript& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    value_08 = other.value_08;
    value_0C = other.value_0C;
    value_10 = other.value_10;
    return *this;
}
