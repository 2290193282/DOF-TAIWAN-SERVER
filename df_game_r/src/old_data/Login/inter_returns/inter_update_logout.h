#ifndef OLDSRV_LOGIN_INTER_UPDATE_LOGOUT_H_
#define OLDSRV_LOGIN_INTER_UPDATE_LOGOUT_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 99_misc_global/Inter_UpdateLogout.
// The DB signal payload is SIG_UPDATE_LOGOUT (139 bytes); this receiver uses
// m_id at +0 and server_id at +0x0E.  Its packet payload is typed in
// protocol/udp_packets.h rather than represented by an opaque byte tail.
class Inter_UpdateLogout : public ISigDispatcher {
public:
    // IDA_EA: 0x84CE548
    // IDA_NAME: _ZN18Inter_UpdateLogout12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_UpdateLogout::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84EB02C / 0x84EB048 / 0x84EB078
    Inter_UpdateLogout();
    virtual ~Inter_UpdateLogout();
};

#if defined(__i386__)
static_assert(sizeof(Inter_UpdateLogout) == 0x04u,
              "Inter_UpdateLogout must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_UPDATE_LOGOUT_H_
