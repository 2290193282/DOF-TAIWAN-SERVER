#ifndef OLDSRV_GLOBAL_INTER_HADES_DISCONNECT_DISPATCHER_H_
#define OLDSRV_GLOBAL_INTER_HADES_DISCONNECT_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_HadesDisConnect : public ISigDispatcher {
public:
    // IDA_EA: 0x84EEACC
    // IDA_NAME: _ZN21Inter_HadesDisConnectC2Ev
    Inter_HadesDisConnect();

    // IDA_EA: 0x84F64F8 (D2), 0x84F6528 (D0)
    virtual ~Inter_HadesDisConnect();

    // IDA_EA: 0x84E4342
    // IDA_NAME: _ZN21Inter_HadesDisConnect12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Inter_HadesDisConnect) == 0x04u,
              "Inter_HadesDisConnect must remain base-only");
#endif

#endif  // OLDSRV_GLOBAL_INTER_HADES_DISCONNECT_DISPATCHER_H_
