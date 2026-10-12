#ifndef OLDSRV_GLOBAL_SHARED_SERVER_MESSAGE_INTER_RECEIVER_H_
#define OLDSRV_GLOBAL_SHARED_SERVER_MESSAGE_INTER_RECEIVER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

// IDA unit: 99_misc_global/Inter_LoadServerMessage.  This concrete DB reply
// receiver stays unregistered until the original InterDispatcher table and its
// DBTask/DispatchThread route are restored together.
class Inter_LoadServerMessage : public ISigDispatcher {
public:
    Inter_LoadServerMessage();
    virtual ~Inter_LoadServerMessage();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


#endif  // OLDSRV_GLOBAL_SHARED_SERVER_MESSAGE_INTER_RECEIVER_H_
