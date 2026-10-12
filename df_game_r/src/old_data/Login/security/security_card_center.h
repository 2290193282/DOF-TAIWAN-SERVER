#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_SECURITY_CARD_CENTER_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_SECURITY_CARD_CENTER_H_

#include "infra/security/crypto/crypto_rijndael.h"

#include <list>
#include <stdint.h>

namespace WongWork {

// IDA: CSecurityCardCenter.  The old GlobalData::Init path allocates 0x488
// bytes, constructs this object, and invokes init().  Raw C2 constructs the
// CRijndael base then a std::list<unsigned char> at old offset +0x47C;
// _updateTimeCount writes the uint16_t at +0x484.
class CSecurityCardCenter : public CRijndael {
public:
    // IDA_EA: 0x82A6654
    // IDA_NAME: _ZN8WongWork19CSecurityCardCenterC2Ev
    CSecurityCardCenter();

    // IDA_EA: 0x85FE026
    // IDA_NAME: _ZN8WongWork19CSecurityCardCenter16_updateTimeCountEv
    CSecurityCardCenter* _updateTimeCount();

    // IDA_EA: 0x85FE05E
    // IDA_NAME: _ZN8WongWork19CSecurityCardCenter4initEv
    CSecurityCardCenter* init();

    // IDA_EA: 0x85FE0EC
    // IDA_NAME: _ZN8WongWork19CSecurityCardCenter17generateServerKeyEPc
    char* generateServerKey(char* destination);

    // IDA_EA: 0x85FE17E
    // IDA_NAME: _ZN8WongWork19CSecurityCardCenter15generateCertKeyEPc
    char* generateCertKey(char* destination);

    // IDA_EA: 0x85FE1BA
    // IDA_NAME: _ZNK8WongWork19CSecurityCardCenter16generateCardDataEPc
    char* generateCardData(char* destination) const;

    // IDA_EA: 0x85FE2E2
    // IDA_NAME: _ZN8WongWork19CSecurityCardCenter13encryptStringEPKcPc
    char* encryptString(const char* source, char* destination);

    // IDA_EA: 0x85FE444
    // IDA_NAME: _ZNK8WongWork19CSecurityCardCenter16checkPhoneNumberEPKc
    int checkPhoneNumber(const char* source) const;

private:
    std::list<unsigned char> character_set_;  // IDA old +0x47C.
    uint16_t time_count_;                     // IDA old +0x484.
};

#if defined(__i386__)
static_assert(sizeof(CSecurityCardCenter) == 0x488u,
              "CSecurityCardCenter old object size");
#endif

}  // namespace WongWork

#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_SECURITY_CARD_CENTER_H_
