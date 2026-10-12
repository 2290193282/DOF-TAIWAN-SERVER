#ifndef OLDSRV_GLOBAL_INTER_GAME_SERVER_REGISTRATION_RECEIVERS_H_
#define OLDSRV_GLOBAL_INTER_GAME_SERVER_REGISTRATION_RECEIVERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_ReqGameServerInfo : public ISigDispatcher {
public:
    Inter_ReqGameServerInfo();
    virtual ~Inter_ReqGameServerInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorRegistGameServerAck : public ISigDispatcher {
public:
    Inter_MonitorRegistGameServerAck();
    virtual ~Inter_MonitorRegistGameServerAck();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_ReqGameServerInfo) == 0x04u,
              "Inter_ReqGameServerInfo must remain base-only");
static_assert(sizeof(Inter_MonitorRegistGameServerAck) == 0x04u,
              "Inter_MonitorRegistGameServerAck must remain base-only");
#endif

#endif  // OLDSRV_GLOBAL_INTER_GAME_SERVER_REGISTRATION_RECEIVERS_H_
