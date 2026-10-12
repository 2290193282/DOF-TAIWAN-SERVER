#include "old_data/PVF/data/script_hash_param.h"


// IDA_EA: 0x826538C
// IDA_NAME: _ZN15ScriptHashParamC2Ev
// DEMANGLED: ScriptHashParam::ScriptHashParam(void)
ScriptHashParam::ScriptHashParam()
    : ParamBase(), script_type(0), script_index(0), script_hash(0) {}

// IDA_EA: 0x82659D2
// IDA_NAME: _ZN15ScriptHashParamD2Ev
// DEMANGLED: ScriptHashParam::~ScriptHashParam()
// IDA_EA: 0x8265A02
// IDA_NAME: _ZN15ScriptHashParamD0Ev
// DEMANGLED: ScriptHashParam::~ScriptHashParam() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
ScriptHashParam::~ScriptHashParam() {}
