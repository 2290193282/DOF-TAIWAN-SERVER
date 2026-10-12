#include "old_data/PVF/event/present_response_script_exception.h"


typedef char PresentResponceScriptException_Size_Must_Be_0x08[
    (sizeof(PresentResponceScriptException) == 0x08) ? 1 : -1];

// IDA_EA: 0x858553A
// IDA_NAME: _ZN30PresentResponceScriptExceptionD2Ev
// DEMANGLED: PresentResponceScriptException::~PresentResponceScriptException()
PresentResponceScriptException::~PresentResponceScriptException() {}

// IDA_EA: 0x8A00B02
// IDA_NAME: _ZN30PresentResponceScriptException5clearEv
// DEMANGLED: PresentResponceScriptException::clear(void)
PresentResponceScriptException* PresentResponceScriptException::clear() {
    value_00 = 0;
    value_04.clear();
    return this;
}

// IDA_EA: 0x8A00B22
// IDA_NAME: _ZN30PresentResponceScriptExceptionC2Ev
// DEMANGLED: PresentResponceScriptException::PresentResponceScriptException(void)
PresentResponceScriptException::PresentResponceScriptException()
    : value_00(0), value_04() {
    clear();
}

// IDA_EA: 0x8A03322
// IDA_NAME: _ZN30PresentResponceScriptExceptionC2ERKS_
// DEMANGLED: PresentResponceScriptException::PresentResponceScriptException(PresentResponceScriptException const&)
PresentResponceScriptException::PresentResponceScriptException(
    const PresentResponceScriptException& other)
    : value_00(other.value_00), value_04(other.value_04) {}

// IDA_EA: 0x8A033D0
// IDA_NAME: _ZN30PresentResponceScriptExceptionaSERKS_
// DEMANGLED: PresentResponceScriptException::operator=(PresentResponceScriptException const&)
PresentResponceScriptException& PresentResponceScriptException::operator=(
    const PresentResponceScriptException& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    return *this;
}
