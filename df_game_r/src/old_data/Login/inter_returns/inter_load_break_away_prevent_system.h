#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_BREAK_AWAY_PREVENT_SYSTEM_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_BREAK_AWAY_PREVENT_SYSTEM_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 11_event_runtime/Inter_LoadBreakAwayPreventSystem.
// InterDispatcher owns this base-only object at outer signal slot 321.
class Inter_LoadBreakAwayPreventSystem : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDF18 / 0x84F8286 / 0x84F82B6.
    Inter_LoadBreakAwayPreventSystem();
    virtual ~Inter_LoadBreakAwayPreventSystem();

    // IDA_EA: 0x84D928C.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadBreakAwayPreventSystem) == 0x04u,
              "Inter_LoadBreakAwayPreventSystem must stay base-only");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_BREAK_AWAY_PREVENT_SYSTEM_H_
