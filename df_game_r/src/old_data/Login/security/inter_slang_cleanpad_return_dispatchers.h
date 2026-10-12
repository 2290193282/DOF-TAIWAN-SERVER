#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_SLANG_CLEANPAD_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_SLANG_CLEANPAD_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA units: 99_misc_global/Inter_Add_Slang and
// 99_misc_global/Inter_LoadCleanPadPointReq.  Both objects only carry the
// ISigDispatcher vptr in the original i386 executable.
class Inter_Add_Slang : public ISigDispatcher {
public:
    Inter_Add_Slang();
    virtual ~Inter_Add_Slang();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadCleanPadPointReq : public ISigDispatcher {
public:
    Inter_LoadCleanPadPointReq();
    virtual ~Inter_LoadCleanPadPointReq();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadCleanPadPoint : public ISigDispatcher {
public:
    Inter_LoadCleanPadPoint();
    virtual ~Inter_LoadCleanPadPoint();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_Add_Slang) == 0x04u,
              "Inter_Add_Slang must retain the original base-only ABI");
static_assert(sizeof(Inter_LoadCleanPadPointReq) == 0x04u,
              "Inter_LoadCleanPadPointReq must retain the original base-only ABI");
static_assert(sizeof(Inter_LoadCleanPadPoint) == 0x04u,
              "Inter_LoadCleanPadPoint must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_INTER_SLANG_CLEANPAD_RETURN_DISPATCHERS_H_
