#ifndef OLDSRV_GLOBAL_INTER_CERA_IPG_RETURN_DISPATCHERS_A_H_
#define OLDSRV_GLOBAL_INTER_CERA_IPG_RETURN_DISPATCHERS_A_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Slots 71 and 72 are base-only ISigDispatcher descendants in the original
// i386 InterDispatcher table.  Their C2/D2/D0 bodies carry no state beyond
// the base vptr; all billing state remains in CIPGHelper and the CeraShop
// vending-machine transaction object.
class Inter_MonitorCeraUpdate : public ISigDispatcher {
public:
    Inter_MonitorCeraUpdate();
    virtual ~Inter_MonitorCeraUpdate();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_IPGResult : public ISigDispatcher {
public:
    Inter_IPGResult();
    virtual ~Inter_IPGResult();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorCeraUpdate) == 0x04u,
              "Inter_MonitorCeraUpdate must retain the base-only i386 ABI");
static_assert(sizeof(Inter_IPGResult) == 0x04u,
              "Inter_IPGResult must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_GLOBAL_INTER_CERA_IPG_RETURN_DISPATCHERS_A_H_
