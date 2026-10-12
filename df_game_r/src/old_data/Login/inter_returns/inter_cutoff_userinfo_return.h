#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_CUTOFF_USERINFO_RETURN_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_CUTOFF_USERINFO_RETURN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 06_userinfo_stat_inventory/Inter_CutOffCallUserInfo.
class Inter_CutOffCallUserInfo : public ISigDispatcher {
public:
    Inter_CutOffCallUserInfo();
    virtual ~Inter_CutOffCallUserInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CutOffCallUserInfo) == 0x04u,
              "Inter_CutOffCallUserInfo must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_CUTOFF_USERINFO_RETURN_H_
