#ifndef OLDSRV_OLD_GAME_EVENT_INTER_LEVELUP_SUPPORT_RETURN_DISPATCHER_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_LEVELUP_SUPPORT_RETURN_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// DBTask slot 848 reply receiver.  Its source was originally folded into
// InterDispatcher.cpp; the event subsystem owns the restored provider.
class Inter_LoadEventLevelupSupport : public ISigDispatcher {
public:
    Inter_LoadEventLevelupSupport();
    virtual ~Inter_LoadEventLevelupSupport();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadEventLevelupSupport) == 0x04u,
              "Inter_LoadEventLevelupSupport must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_LEVELUP_SUPPORT_RETURN_DISPATCHER_H_
