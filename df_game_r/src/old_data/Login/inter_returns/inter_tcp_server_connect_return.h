#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_TCP_SERVER_CONNECT_RETURN_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_TCP_SERVER_CONNECT_RETURN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_TcpServerConnect : public ISigDispatcher {
public:
    Inter_TcpServerConnect();
    virtual ~Inter_TcpServerConnect();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_TcpServerConnect) == 0x04u,
              "Inter_TcpServerConnect must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_TCP_SERVER_CONNECT_RETURN_H_
