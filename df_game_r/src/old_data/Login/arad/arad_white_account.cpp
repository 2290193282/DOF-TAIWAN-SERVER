#include "old_data/Login/arad/arad_white_account.h"

namespace ARAD {

// IDA_EA: 0x81AB518
// IDA_NAME: _ZN4ARAD16AradWhiteAccountC2Ev
// DEMANGLED: ARAD::AradWhiteAccount::AradWhiteAccount(void)
AradWhiteAccount::AradWhiteAccount() : account_ids_() {
    Load();
}

// IDA_EA: 0x81AB55A
// IDA_NAME: _ZN4ARAD16AradWhiteAccountD2Ev
// DEMANGLED: ARAD::AradWhiteAccount::~AradWhiteAccount()
AradWhiteAccount::~AradWhiteAccount() {
    account_ids_.clear();
}

// IDA_EA: 0x81AB59C
// IDA_NAME: _ZN4ARAD16AradWhiteAccount14IsWhiteAccountEj
// DEMANGLED: ARAD::AradWhiteAccount::IsWhiteAccount(unsigned int)
bool AradWhiteAccount::IsWhiteAccount(uint32_t account_id) {
    return account_ids_.find(account_id) != account_ids_.end();
}

// IDA_EA: 0x81AB5F8
// IDA_NAME: _ZN4ARAD16AradWhiteAccount4LoadEv
// DEMANGLED: ARAD::AradWhiteAccount::Load(void)
int AradWhiteAccount::Load() {
    account_ids_.insert(2620016u);
    account_ids_.insert(2620020u);
    account_ids_.insert(2620003u);
    return 1;
}

}  // namespace ARAD
