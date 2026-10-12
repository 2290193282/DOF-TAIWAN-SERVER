#include "old_data/PVF/event/present_response_script.h"


typedef char PresentResponceScript_Size_Must_Be_0x10[
    (sizeof(PresentResponceScript) == 0x10) ? 1 : -1];

// IDA_EA: 0x8585510
// IDA_NAME: _ZN21PresentResponceScriptD2Ev
// DEMANGLED: PresentResponceScript::~PresentResponceScript()
PresentResponceScript::~PresentResponceScript() {}

// IDA_EA: 0x8A00A86
// IDA_NAME: _ZN21PresentResponceScript5clearEv
// DEMANGLED: PresentResponceScript::clear(void)
PresentResponceScript* PresentResponceScript::clear() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    value_0C.clear();
    return this;
}

// IDA_EA: 0x8A00ABA
// IDA_NAME: _ZN21PresentResponceScriptC2Ev
// DEMANGLED: PresentResponceScript::PresentResponceScript(void)
PresentResponceScript::PresentResponceScript()
    : value_00(0), value_04(0), value_08(0), value_0C() {
    clear();
}

// IDA_EA: 0x8A02F24
// IDA_NAME: _ZN21PresentResponceScriptC2ERKS_
// DEMANGLED: PresentResponceScript::PresentResponceScript(PresentResponceScript const&)
PresentResponceScript::PresentResponceScript(
    const PresentResponceScript& other)
    : value_00(other.value_00),
      value_04(other.value_04),
      value_08(other.value_08),
      value_0C(other.value_0C) {}

// IDA_EA: 0x8A02FEA
// IDA_NAME: _ZN21PresentResponceScriptaSERKS_
// DEMANGLED: PresentResponceScript::operator=(PresentResponceScript const&)
PresentResponceScript& PresentResponceScript::operator=(
    const PresentResponceScript& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    value_08 = other.value_08;
    value_0C = other.value_0C;
    return *this;
}
