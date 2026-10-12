#ifndef OLDSRV_GLOBAL_BLACK_IP_INTER_RECEIVERS_H_
#define OLDSRV_GLOBAL_BLACK_IP_INTER_RECEIVERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

// IDA units: 99_misc_global/Inter_BlackIPMonitor{Req,PartLoadReq} and
// Inter_BlackIPMonitor. These real receivers are intentionally not registered
// until the complete original 852-slot InterDispatcher constructor is ready.
class Inter_BlackIPMonitorReq : public ISigDispatcher {
public:
    Inter_BlackIPMonitorReq();
    virtual ~Inter_BlackIPMonitorReq();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_BlackIPMonitorPartLoadReq : public ISigDispatcher {
public:
    Inter_BlackIPMonitorPartLoadReq();
    virtual ~Inter_BlackIPMonitorPartLoadReq();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_BlackIPMonitor : public ISigDispatcher {
public:
    Inter_BlackIPMonitor();
    virtual ~Inter_BlackIPMonitor();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


#endif  // OLDSRV_GLOBAL_BLACK_IP_INTER_RECEIVERS_H_
