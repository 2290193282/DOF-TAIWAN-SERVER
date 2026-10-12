#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_BREAKAWAY_RETURN_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_BREAKAWAY_RETURN_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 11_event_runtime/Inter_LoadBreakAwayPreventMessage.
class Inter_LoadBreakAwayPreventMessage : public ISigDispatcher {
public:
    Inter_LoadBreakAwayPreventMessage();
    virtual ~Inter_LoadBreakAwayPreventMessage();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadBreakAwayPreventMessage) == 0x04u,
              "Inter_LoadBreakAwayPreventMessage must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_BREAKAWAY_RETURN_DISPATCHER_H_
