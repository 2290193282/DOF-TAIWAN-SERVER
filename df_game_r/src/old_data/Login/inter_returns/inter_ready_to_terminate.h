#ifndef OLDSRV_LOGIN_INTER_READY_TO_TERMINATE_H_
#define OLDSRV_LOGIN_INTER_READY_TO_TERMINATE_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit 99_misc_global/Inter_ReadyToTerminate.  It converts the monitor
// reply into the original outer shutdown signal 63 on the DB queue.
class Inter_ReadyToTerminate : public ISigDispatcher {
public:
    // IDA_EA: 0x84C7D2C
    // IDA_NAME: _ZN22Inter_ReadyToTerminate12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84EA0DA / 0x84EA0F6 / 0x84EA126
    Inter_ReadyToTerminate();
    virtual ~Inter_ReadyToTerminate();
};

#if defined(__i386__)
static_assert(sizeof(Inter_ReadyToTerminate) == 0x04u,
              "Inter_ReadyToTerminate must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_READY_TO_TERMINATE_H_
