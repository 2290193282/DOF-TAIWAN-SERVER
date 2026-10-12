#ifndef OLDSRV_SECURITY_INTER_SECURITY_RETURN_DISPATCHERS_H_
#define OLDSRV_SECURITY_INTER_SECURITY_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Slot 628.  The reply changes security-card/CleanPad state through the
// process-lifetime Secu_DataControl service.
class Inter_UpdateSecuServiceState : public ISigDispatcher {
public:
    Inter_UpdateSecuServiceState();
    virtual ~Inter_UpdateSecuServiceState();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_UpdateSecuServiceState) == 0x04u,
              "Inter_UpdateSecuServiceState must remain base-only");
#endif

#endif  // OLDSRV_SECURITY_INTER_SECURITY_RETURN_DISPATCHERS_H_
