#ifndef OLDSRV_LOGIN_INTER_LOAD_TICKET_H_
#define OLDSRV_LOGIN_INTER_LOAD_TICKET_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit 99_misc_global/Inter_LoadTicket.  This is a base-only i386
// dispatcher and receives the complete DB_LoadTicket SIG_LOAD_TICKET record.
class Inter_LoadTicket : public ISigDispatcher {
public:
    // IDA_EA: 0x84C515C
    // IDA_NAME: _ZN16Inter_LoadTicket12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84E9EC8 / 0x84E9EE4 / 0x84E9F14
    Inter_LoadTicket();
    virtual ~Inter_LoadTicket();
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadTicket) == 0x04u,
              "Inter_LoadTicket must retain its original base-only ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_LOAD_TICKET_H_
