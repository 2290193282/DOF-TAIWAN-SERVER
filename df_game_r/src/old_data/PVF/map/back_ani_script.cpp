#include "old_data/PVF/map/back_ani_script.h"


typedef char BackAniScript_Size_Must_Be_0x14[(sizeof(BackAniScript) == 0x14) ? 1 : -1];

// IDA_EA: 0x83D95CC
// IDA_NAME: _ZN13BackAniScriptC2ERKS_
// DEMANGLED: BackAniScript::BackAniScript(BackAniScript const&)
BackAniScript::BackAniScript(const BackAniScript& rhs)
    : animation_name(rhs.animation_name),
      layer(rhs.layer),
      flag_08(rhs.flag_08),
      flag_09(rhs.flag_09),
      flag_0a(rhs.flag_0a),
      value_0c(rhs.value_0c),
      value_10(rhs.value_10) {}

// IDA_EA: 0x83DC880
// IDA_NAME: _ZN13BackAniScriptD2Ev
// DEMANGLED: BackAniScript::~BackAniScript()
BackAniScript::~BackAniScript() {}

// IDA_EA: 0x89E884A
// IDA_NAME: _ZN13BackAniScript5clearEv
// DEMANGLED: BackAniScript::clear(void)
BackAniScript* BackAniScript::clear() {
    animation_name = "";
    layer = 1;
    flag_08 = 0;
    flag_09 = 0;
    flag_0a = 0;
    value_0c = 0;
    value_10 = 0;
    return this;
}

// IDA_EA: 0x89E8898
// IDA_NAME: _ZN13BackAniScriptC2Ev
// DEMANGLED: BackAniScript::BackAniScript(void)
BackAniScript::BackAniScript() : animation_name() {
    clear();
}

// IDA_EA: 0x89EA572
// IDA_NAME: _ZN13BackAniScriptaSERKS_
// DEMANGLED: BackAniScript::operator=(BackAniScript const&)
BackAniScript& BackAniScript::operator=(const BackAniScript& rhs) {
    animation_name = rhs.animation_name;
    layer = rhs.layer;
    flag_08 = rhs.flag_08;
    flag_09 = rhs.flag_09;
    flag_0a = rhs.flag_0a;
    value_0c = rhs.value_0c;
    value_10 = rhs.value_10;
    return *this;
}
