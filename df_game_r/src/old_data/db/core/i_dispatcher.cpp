#include "old_data/db/core/i_dispatcher.h"


// IDA_EA: 0x82308F8
// IDA_NAME: _ZN11IDispatcherC2Ev
// DEMANGLED: IDispatcher::IDispatcher(void)
IDispatcher::IDispatcher() {}

// IDA_EA: 0x8125408
// IDA_NAME: _ZN11IDispatcherD2Ev
// DEMANGLED: IDispatcher::~IDispatcher()
// IDA_EA: 0x812542E
// IDA_NAME: _ZN11IDispatcherD0Ev
// DEMANGLED: IDispatcher::~IDispatcher()
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
IDispatcher::~IDispatcher() {}
