#ifndef OLDSRV_OLD_DATA_DB_CORE_TAIWAN_INTERFACE_DB_DISPATCHER_H_
#define OLDSRV_OLD_DATA_DB_CORE_TAIWAN_INTERFACE_DB_DISPATCHER_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"


class Stream;


// Old database-side Taiwan internal-dispatch ABI.  The vtable is exactly
// [D2, D0, pure dispatch] and the object is [vptr, TaiwanInternalPack::T].
class TaiwanInterfaceDBDispatcher {
public:
    // IDA_EA: 0x815DF66
    // IDA_NAME: _ZN27TaiwanInterfaceDBDispatcherC2EN18TaiwanInternalPack1TE
    // DEMANGLED: TaiwanInterfaceDBDispatcher::TaiwanInterfaceDBDispatcher(TaiwanInternalPack::T)
    explicit TaiwanInterfaceDBDispatcher(TaiwanInternalPack::T code);

    // IDA_EA: 0x815DF7E
    // IDA_NAME: _ZN27TaiwanInterfaceDBDispatcherD2Ev
    // DEMANGLED: TaiwanInterfaceDBDispatcher::~TaiwanInterfaceDBDispatcher()
    // IDA_EA: 0x815DFA4
    // IDA_NAME: _ZN27TaiwanInterfaceDBDispatcherD0Ev
    // DEMANGLED: TaiwanInterfaceDBDispatcher::~TaiwanInterfaceDBDispatcher()
    virtual ~TaiwanInterfaceDBDispatcher();

    // Called through vtable +8 by DB_DispatcherTaiwan::dispatch @0x844CA48.
    // The old base vtable contains __cxa_pure_virtual here; no base body exists.
    virtual int dispatch(int signal, int user_idx, Stream* stream) = 0;

    // IDA_EA: 0x815DFC2
    // IDA_NAME: _ZNK27TaiwanInterfaceDBDispatcher7getCodeEv
    // DEMANGLED: TaiwanInterfaceDBDispatcher::getCode(void)const
    int getCode() const;

private:
    TaiwanInternalPack::T code_;
};


#if defined(__i386__)
static_assert(sizeof(TaiwanInterfaceDBDispatcher) == 0x08u,
              "Taiwan DB interface dispatcher must retain its i386 8-byte ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_CORE_TAIWAN_INTERFACE_DB_DISPATCHER_H_
