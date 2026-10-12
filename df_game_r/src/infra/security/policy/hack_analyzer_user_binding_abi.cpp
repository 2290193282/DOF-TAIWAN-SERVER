#include "infra/security/policy/hack_analyzer_public_abi.h"


// IDA_EA: 0x80F79C0
// IDA_NAME: _ZN8WongWork13CHackAnalyzer8setUserPEP5CUser
// DEMANGLED: WongWork::CHackAnalyzer::setUserP(CUser *)
WongWork::CHackAnalyzer* WongWork::CHackAnalyzer::setUserP(CUser* user) {
    user_ = user;
    return this;
}

// IDA_EA: 0x80F79CE
// IDA_NAME: _ZN8WongWork13CHackAnalyzer8getUserPEv
// DEMANGLED: WongWork::CHackAnalyzer::getUserP(void)
CUser* WongWork::CHackAnalyzer::getUserP() {
    return user_ ? user_ : 0;
}
