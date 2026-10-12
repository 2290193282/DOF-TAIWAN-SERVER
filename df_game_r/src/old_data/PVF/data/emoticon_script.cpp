#include "old_data/PVF/data/emoticon_script.h"

namespace EmoticonType {

typedef char EmoticonScript_Size_Must_Be_0x14[
    (sizeof(EmoticonScript) == 0x14) ? 1 : -1];

// IDA_EA: 0x80E602E
// IDA_NAME: _ZN12EmoticonType14EmoticonScriptC2Ev
// DEMANGLED: EmoticonType::EmoticonScript::EmoticonScript(void)
EmoticonScript::EmoticonScript() : value_0C(), value_10() {}

// IDA_EA: 0x80E607A
// IDA_NAME: _ZN12EmoticonType14EmoticonScriptD2Ev
// DEMANGLED: EmoticonType::EmoticonScript::~EmoticonScript()
EmoticonScript::~EmoticonScript() {}

}  // namespace EmoticonType
