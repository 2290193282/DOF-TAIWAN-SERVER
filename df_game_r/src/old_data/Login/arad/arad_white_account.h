#ifndef OLDSRV_OLD_DATA_LOGIN_ARAD_ARAD_WHITE_ACCOUNT_H_
#define OLDSRV_OLD_DATA_LOGIN_ARAD_ARAD_WHITE_ACCOUNT_H_

#include <stdint.h>

#include <set>

namespace ARAD {

class AradWhiteAccount {
public:
    // IDA_EA: 0x81AB518
    // IDA_NAME: _ZN4ARAD16AradWhiteAccountC2Ev
    // DEMANGLED: ARAD::AradWhiteAccount::AradWhiteAccount(void)
    AradWhiteAccount();

    // IDA_EA: 0x81AB55A
    // IDA_NAME: _ZN4ARAD16AradWhiteAccountD2Ev
    // DEMANGLED: ARAD::AradWhiteAccount::~AradWhiteAccount()
    ~AradWhiteAccount();

    // IDA_EA: 0x81AB59C
    // IDA_NAME: _ZN4ARAD16AradWhiteAccount14IsWhiteAccountEj
    // DEMANGLED: ARAD::AradWhiteAccount::IsWhiteAccount(unsigned int)
    bool IsWhiteAccount(uint32_t account_id);

    // IDA_EA: 0x81AB5F8
    // IDA_NAME: _ZN4ARAD16AradWhiteAccount4LoadEv
    // DEMANGLED: ARAD::AradWhiteAccount::Load(void)
    int Load();

private:
    std::set<uint32_t> account_ids_;
};

}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_LOGIN_ARAD_ARAD_WHITE_ACCOUNT_H_
