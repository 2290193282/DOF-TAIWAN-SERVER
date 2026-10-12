#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOGIN_INFO_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOGIN_INFO_H_

// IDA unit: 02_login_account/Inter_LoginInfo.
// The outer InterDispatcher C2 places this ISigDispatcher-derived object at
// slot 3.  It consumes the full SIG_LOGIN_DATA reply emitted after CMD1; no
// reduced login-result facade is ABI-compatible with that transaction.

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_LoginInfo : public ISigDispatcher {
public:
    // IDA_EA: 0x84E9F9C / 0x84E9FB8 / 0x84E9FE8
    // IDA_NAME: _ZN15Inter_LoginInfoC2Ev / D2Ev / D0Ev
    Inter_LoginInfo();
    virtual ~Inter_LoginInfo();

    // IDA_EA: 0x84C5294
    // IDA_NAME: _ZN15Inter_LoginInfo12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoginInfo) == 0x04u,
              "Inter_LoginInfo must remain the base-only i386 dispatcher");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOGIN_INFO_H_
