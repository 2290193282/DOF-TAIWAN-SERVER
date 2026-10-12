#include "common/manager/taiwan_interface_inter_dispatcher.h"


// IDA_EA: 0x8161E32
// IDA_NAME: _ZN30TaiwanInterfaceInterDispatcherC2EN18TaiwanInternalPack1TE
// DEMANGLED: TaiwanInterfaceInterDispatcher::TaiwanInterfaceInterDispatcher(TaiwanInternalPack::T)
TaiwanInterfaceInterDispatcher::TaiwanInterfaceInterDispatcher(
    TaiwanInternalPack::T code)
    : code_(code) {}

// IDA_EA: 0x8161E4A
// IDA_NAME: _ZN30TaiwanInterfaceInterDispatcherD2Ev
// DEMANGLED: TaiwanInterfaceInterDispatcher::~TaiwanInterfaceInterDispatcher()
// IDA_EA: 0x8161E70
// IDA_NAME: _ZN30TaiwanInterfaceInterDispatcherD0Ev
// DEMANGLED: TaiwanInterfaceInterDispatcher::~TaiwanInterfaceInterDispatcher()
TaiwanInterfaceInterDispatcher::~TaiwanInterfaceInterDispatcher() {}

// IDA_EA: 0x8161E8E
// IDA_NAME: _ZNK30TaiwanInterfaceInterDispatcher7getCodeEv
// DEMANGLED: TaiwanInterfaceInterDispatcher::getCode(void)const
int TaiwanInterfaceInterDispatcher::getCode() const {
    return code_;
}
