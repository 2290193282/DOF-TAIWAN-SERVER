#include "old_data/PVF/npc/interactive_dialog_script.h"


typedef char InteractiveDialogScript_Size_Must_Be_0x34[
    (sizeof(InteractiveDialogScript) == 0x34) ? 1 : -1];

// IDA_EA: 0x83D7FA4
// IDA_NAME: _ZN23InteractiveDialogScriptC2ERKS_
// DEMANGLED: InteractiveDialogScript::InteractiveDialogScript(InteractiveDialogScript const&)
InteractiveDialogScript::InteractiveDialogScript(
    const InteractiveDialogScript& other)
    : value_00(other.value_00),
      value_04(other.value_04),
      value_08(other.value_08),
      value_0C(other.value_0C),
      value_18(other.value_18),
      value_1C(other.value_1C),
      value_20(other.value_20),
      value_24(other.value_24),
      value_28(other.value_28) {}

// IDA_EA: 0x83DCA8E
// IDA_NAME: _ZN23InteractiveDialogScriptD2Ev
// DEMANGLED: InteractiveDialogScript::~InteractiveDialogScript()
InteractiveDialogScript::~InteractiveDialogScript() {}

// IDA_EA: 0x8A0098A
// IDA_NAME: _ZN23InteractiveDialogScript5clearEv
// DEMANGLED: InteractiveDialogScript::clear(void)
int InteractiveDialogScript::clear() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    value_0C.clear();
    value_18 = 0;
    value_1C = 0;
    value_20 = 0;
    value_24.clear();
    value_28.clear();
    return 0;
}

// IDA_EA: 0x8A009F2
// IDA_NAME: _ZN23InteractiveDialogScriptC2Ev
// DEMANGLED: InteractiveDialogScript::InteractiveDialogScript(void)
InteractiveDialogScript::InteractiveDialogScript()
    : value_0C(), value_24(), value_28() {
    clear();
}

// IDA_EA: 0x8A02B96
// IDA_NAME: _ZN23InteractiveDialogScriptaSERKS_
// DEMANGLED: InteractiveDialogScript::operator=(InteractiveDialogScript const&)
InteractiveDialogScript& InteractiveDialogScript::operator=(
    const InteractiveDialogScript& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    value_08 = other.value_08;
    value_0C = other.value_0C;
    value_18 = other.value_18;
    value_1C = other.value_1C;
    value_20 = other.value_20;
    value_24 = other.value_24;
    value_28 = other.value_28;
    return *this;
}
