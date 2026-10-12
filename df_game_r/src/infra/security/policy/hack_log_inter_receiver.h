#ifndef OLDSRV_GLOBAL_HACK_LOG_INTER_RECEIVER_H_
#define OLDSRV_GLOBAL_HACK_LOG_INTER_RECEIVER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

// IDA unit: 99_misc_global/Inter_LoadDnfHackLog. This concrete DB reply
// receiver remains unregistered until the complete InterDispatcher table and
// its DispatchThread/DBTask route are restored together.
class Inter_LoadDnfHackLog : public ISigDispatcher {
public:
    Inter_LoadDnfHackLog();
    virtual ~Inter_LoadDnfHackLog();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


#endif  // OLDSRV_GLOBAL_HACK_LOG_INTER_RECEIVER_H_
