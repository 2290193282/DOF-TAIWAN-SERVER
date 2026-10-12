#include "infra/network/packet/message_base.h"


// IDA_EA: 0x812C076
// IDA_NAME: _ZN8MSG_BASEC2Ev
// DEMANGLED: MSG_BASE::MSG_BASE(void)
MSG_BASE::MSG_BASE() {}

// IDA_EA: 0x812530C
// IDA_NAME: _ZN9ParamBaseC2Ev
// DEMANGLED: ParamBase::ParamBase(void)
ParamBase::ParamBase() {}

// IDA_EA: 0x8235160
// IDA_NAME: _ZN9ParamBaseC2ERKS_
// DEMANGLED: ParamBase::ParamBase(ParamBase const&)
ParamBase::ParamBase(const ParamBase& rhs) {
    (void)rhs;
}

// IDA_EA: 0x81252C8
// IDA_NAME: _ZN9ParamBaseD2Ev
// DEMANGLED: ParamBase::~ParamBase()
// IDA_EA: 0x81252EE
// IDA_NAME: _ZN9ParamBaseD0Ev
// DEMANGLED: ParamBase::~ParamBase() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
ParamBase::~ParamBase() {}
