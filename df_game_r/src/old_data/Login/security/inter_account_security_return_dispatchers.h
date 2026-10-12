#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_ACCOUNT_SECURITY_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_ACCOUNT_SECURITY_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Original InterDispatcher account-security return consumers.  They are
// base-only i386 ISigDispatcher derivatives; their bodies belong to the
// account/security flow rather than to the InterDispatcher root table.
class Inter_SaveClientHackLog : public ISigDispatcher {
public:
    Inter_SaveClientHackLog();
    virtual ~Inter_SaveClientHackLog();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_CodeChecksumRange : public ISigDispatcher {
public:
    Inter_CodeChecksumRange();
    virtual ~Inter_CodeChecksumRange();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MouseRegister : public ISigDispatcher {
public:
    Inter_MouseRegister();
    virtual ~Inter_MouseRegister();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ChagneUserHandicap : public ISigDispatcher {
public:
    Inter_ChagneUserHandicap();
    virtual ~Inter_ChagneUserHandicap();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_SaveClientHackLog) == 0x04u,
              "Inter_SaveClientHackLog must retain the original base-only ABI");
static_assert(sizeof(Inter_CodeChecksumRange) == 0x04u,
              "Inter_CodeChecksumRange must retain the original base-only ABI");
static_assert(sizeof(Inter_MouseRegister) == 0x04u,
              "Inter_MouseRegister must retain the original base-only ABI");
static_assert(sizeof(Inter_ChagneUserHandicap) == 0x04u,
              "Inter_ChagneUserHandicap must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_ACCOUNT_SECURITY_RETURN_DISPATCHERS_H_
