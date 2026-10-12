#ifndef OLDSRV_PROTOCOL_INTER_DISPATCHER_ARAD_ONLY_H_
#define OLDSRV_PROTOCOL_INTER_DISPATCHER_ARAD_ONLY_H_

// This adapter is the outer InterDispatcher receiver for the ARAD internal
// return route.  It owns the inner-code map but deliberately does not install
// itself into InterDispatcher; the complete 852-slot InterDispatcher lifecycle
// is outside this unregistered SR08 source slice.
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_data/db/legacy_dispatch/arad_internal_dispatcher.h"

class CUser;

// IDA unit: 13_database_dispatch/Inter_DispatcherAradOnly.
//
// i386 layout (0x1c): ISigDispatcher at +0x00 and the complete
// ARAD::DISPATCHER::Arad_InternalDispatcher map owner at +0x04.  The latter
// owns the concrete ARAD-return leaves; this class only removes the four-byte
// inner-code prefix before forwarding to one of them.
class Inter_DispatcherAradOnly : public ISigDispatcher {
public:
    // IDA_EA: 0x84E7EB8
    // IDA_NAME: _ZN24Inter_DispatcherAradOnlyC2Ev
    // DEMANGLED: Inter_DispatcherAradOnly::Inter_DispatcherAradOnly(void)
    Inter_DispatcherAradOnly();

    // IDA_EA: 0x84E7F2C
    // IDA_NAME: _ZN24Inter_DispatcherAradOnly12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_DispatcherAradOnly::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84F5A1C
    // IDA_NAME: _ZN24Inter_DispatcherAradOnlyD2Ev
    // DEMANGLED: Inter_DispatcherAradOnly::~Inter_DispatcherAradOnly()
    // IDA_EA: 0x84F5A7E
    // IDA_NAME: _ZN24Inter_DispatcherAradOnlyD0Ev
    // DEMANGLED: Inter_DispatcherAradOnly::~Inter_DispatcherAradOnly()
    virtual ~Inter_DispatcherAradOnly();

private:
    ARAD::DISPATCHER::Arad_InternalDispatcher arad_dispatcher_;
};

#if defined(__i386__)
static_assert(sizeof(ISigDispatcher) == 0x04u,
              "ISigDispatcher must remain a one-vptr i386 base");
static_assert(sizeof(ARAD::ENUM_INTERNALPACKET_JPN) == 0x04u,
              "ARAD inner packet code must remain the recovered dword");
static_assert(sizeof(Inter_DispatcherAradOnly) == 0x1cu,
              "Inter_DispatcherAradOnly must retain the old i386 layout");
#endif

#endif  // OLDSRV_PROTOCOL_INTER_DISPATCHER_ARAD_ONLY_H_
