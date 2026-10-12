#ifndef OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_INTER_RECEIVERS_H_
#define OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_INTER_RECEIVERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

// IDA units: 99_misc_global/Inter_WebNoticeIngameAdvertisement and
// 99_misc_global/Inter_ReloadIngameAdvertisement.  These concrete endpoints
// remain unregistered until the complete original InterDispatcher table is
// restored atomically.
class Inter_WebNoticeIngameAdvertisement : public ISigDispatcher {
public:
    Inter_WebNoticeIngameAdvertisement();
    virtual ~Inter_WebNoticeIngameAdvertisement();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ReloadIngameAdvertisement : public ISigDispatcher {
public:
    Inter_ReloadIngameAdvertisement();
    virtual ~Inter_ReloadIngameAdvertisement();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};


#endif  // OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_INTER_RECEIVERS_H_
