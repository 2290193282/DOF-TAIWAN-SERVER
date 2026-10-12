#ifndef OLDSRV_LOGIN_INTER_LAST_MSG_RESPONSE_H_
#define OLDSRV_LOGIN_INTER_LAST_MSG_RESPONSE_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 99_misc_global/Inter_LastMsgResponse.  The original receiver has
// no payload dependency: it accounts for a final DB acknowledgement and
// completes shutdown when every queued last-message reply has arrived.
class Inter_LastMsgResponse : public ISigDispatcher {
public:
    // IDA_EA: 0x84D0598
    // IDA_NAME: _ZN21Inter_LastMsgResponse12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_LastMsgResponse::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED6C8 / 0x84F9A98 / 0x84F9AC8
    Inter_LastMsgResponse();
    virtual ~Inter_LastMsgResponse();
};

#if defined(__i386__)
static_assert(sizeof(Inter_LastMsgResponse) == 0x04u,
              "Inter_LastMsgResponse must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_LAST_MSG_RESPONSE_H_
