#ifndef OLDSRV_LOGIN_INTER_ACTION_POINT_RETURNS_H_
#define OLDSRV_LOGIN_INTER_ACTION_POINT_RETURNS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

namespace APSystem {

class Inter_LoadActionPoint : public ISigDispatcher {
public:
    // IDA_EA: 0x8124D2A
    // IDA_NAME: _ZN8APSystem21Inter_LoadActionPoint12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED410 / 0x8125584 / 0x81255B4
    Inter_LoadActionPoint();
    virtual ~Inter_LoadActionPoint();
};

class Inter_EmptyActionPoint : public ISigDispatcher {
public:
    // IDA_EA: 0x8124DB0
    // IDA_NAME: _ZN8APSystem22Inter_EmptyActionPoint12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED42C / 0x81255D2 / 0x8125602
    Inter_EmptyActionPoint();
    virtual ~Inter_EmptyActionPoint();
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadActionPoint) == 0x04u,
              "Inter_LoadActionPoint must retain its base-only i386 ABI");
static_assert(sizeof(Inter_EmptyActionPoint) == 0x04u,
              "Inter_EmptyActionPoint must retain its base-only i386 ABI");
#endif

}  // namespace APSystem

#endif  // OLDSRV_LOGIN_INTER_ACTION_POINT_RETURNS_H_
