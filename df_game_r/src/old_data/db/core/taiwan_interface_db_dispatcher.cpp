#include "old_data/db/core/taiwan_interface_db_dispatcher.h"


// IDA_EA: 0x815DF66
// IDA_NAME: _ZN27TaiwanInterfaceDBDispatcherC2EN18TaiwanInternalPack1TE
// DEMANGLED: TaiwanInterfaceDBDispatcher::TaiwanInterfaceDBDispatcher(TaiwanInternalPack::T)
TaiwanInterfaceDBDispatcher::TaiwanInterfaceDBDispatcher(
    TaiwanInternalPack::T code)
    : code_(code) {
}

// IDA_EA: 0x815DF7E
// IDA_NAME: _ZN27TaiwanInterfaceDBDispatcherD2Ev
// DEMANGLED: TaiwanInterfaceDBDispatcher::~TaiwanInterfaceDBDispatcher()
// IDA_EA: 0x815DFA4
// IDA_NAME: _ZN27TaiwanInterfaceDBDispatcherD0Ev
// DEMANGLED: TaiwanInterfaceDBDispatcher::~TaiwanInterfaceDBDispatcher()
TaiwanInterfaceDBDispatcher::~TaiwanInterfaceDBDispatcher() {
}

// IDA_EA: 0x815DFC2
// IDA_NAME: _ZNK27TaiwanInterfaceDBDispatcher7getCodeEv
// DEMANGLED: TaiwanInterfaceDBDispatcher::getCode(void)const
int TaiwanInterfaceDBDispatcher::getCode() const {
    return static_cast<int>(code_);
}
