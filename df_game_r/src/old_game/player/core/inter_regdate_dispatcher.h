#ifndef OLDSRV_OLD_GAME_USERINFO_INTER_REGDATE_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_INTER_REGDATE_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Slot 673: DB_GetUserRegdate response enters the new-member anti-hack
// history classifier.
class Inter_GetUserRegdate : public ISigDispatcher {
public:
    Inter_GetUserRegdate();
    virtual ~Inter_GetUserRegdate();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_GetUserRegdate) == 0x04u,
              "Inter_GetUserRegdate must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INTER_REGDATE_DISPATCHER_H_
