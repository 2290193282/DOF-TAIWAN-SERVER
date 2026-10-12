#ifndef OLDSRV_OLD_GAME_USERINFO_INTER_SET_SECOUN_USER_DATA_H_
#define OLDSRV_OLD_GAME_USERINFO_INTER_SET_SECOUN_USER_DATA_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

// IDA unit: 99_misc_global/Inter_SetSecounUserData.  Keep the historical
// spelling: it is part of the original C++ symbol and InterDispatcher table.
class Inter_SetSecounUserData : public ISigDispatcher {
public:
    Inter_SetSecounUserData();
    virtual ~Inter_SetSecounUserData();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_SetSecounUserData) == 0x04u,
              "Inter_SetSecounUserData must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INTER_SET_SECOUN_USER_DATA_H_
