#ifndef OLDSRV_OLD_GAME_USERINFO_INTER_CHANGE_CHARAC_NAME_RETURN_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_INTER_CHANGE_CHARAC_NAME_RETURN_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_ReqChangeCharacName : public ISigDispatcher {
public:
    Inter_ReqChangeCharacName();
    virtual ~Inter_ReqChangeCharacName();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_ReqChangeCharacName) == 0x04u,
              "Inter_ReqChangeCharacName must retain original base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INTER_CHANGE_CHARAC_NAME_RETURN_DISPATCHER_H_
