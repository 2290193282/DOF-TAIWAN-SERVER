#include "old_game/economy/mall/cash_transaction_runtime.h"

#include <algorithm>
#include <cstdio>
#include <new>

namespace nexon {
namespace cash {

// Old image data at 0x9407D90 is 0xffffffff.  Keeping that initializer is
// required because the old pre-increment makes the first assigned ID zero.
uint32_t TransactionManager::tx_id_ = 0xffffffffu;

// IDA_EA: 0x81B0128
// IDA_NAME: _ZN5nexon4cash18TransactionManager8instanceEv
// DEMANGLED: nexon::cash::TransactionManager::instance(void)
TransactionManager* TransactionManager::instance() {
    static TransactionManager i;
    return &i;
}

// IDA_EA: 0x81B01BC
// IDA_NAME: _ZN5nexon4cash18TransactionManager14addTransactionEPNS0_11TransactionE
// DEMANGLED: nexon::cash::TransactionManager::addTransaction(nexon::cash::Transaction *)
void TransactionManager::addTransaction(Transaction* transaction) {
    transactions_.insert(std::make_pair(transaction->id(), transaction));
}

// IDA_EA: 0x81B021C
// IDA_NAME: _ZN5nexon4cash18TransactionManager17removeTransactionEPNS0_11TransactionE
// DEMANGLED: nexon::cash::TransactionManager::removeTransaction(nexon::cash::Transaction *)
int TransactionManager::removeTransaction(Transaction* transaction) {
    return transactions_.erase(transaction->id());
}

// IDA_EA: 0x81B0244
// IDA_NAME: _ZN5nexon4cash18TransactionManager6cancelEj
// DEMANGLED: nexon::cash::TransactionManager::cancel(unsigned int)
void TransactionManager::cancel(uint32_t transaction_id) {
    Transaction* transaction = getTransaction(transaction_id);
    if (transaction) {
        transaction->cancel();
        transaction->getUser()->removeTransaction(transaction);
        ::operator delete(transaction);
    }
}

// IDA_EA: 0x81B0298
// IDA_NAME: _ZN5nexon4cash18TransactionManager7confirmEj
// DEMANGLED: nexon::cash::TransactionManager::confirm(unsigned int)
void TransactionManager::confirm(uint32_t transaction_id) {
    Transaction* transaction = getTransaction(transaction_id);
    if (transaction) {
        transaction->confirm();
        transaction->getUser()->removeTransaction(transaction);
        ::operator delete(transaction);
    }
}

// IDA_EA: 0x81B02EC
// IDA_NAME: _ZN5nexon4cash11TransactionC2EPNS0_11UserBalanceEi
// DEMANGLED: nexon::cash::Transaction::Transaction(nexon::cash::UserBalance *,int)
Transaction::Transaction(UserBalance* user, int amount)
    : id_(0),
      amount_(amount),
      created_time_(0),
      user_(user) {
    id_ = TransactionManager::getNextTransactionID();
    created_time_ = std::time(0);
}

// IDA_EA: 0x81B0336
// IDA_NAME: _ZN5nexon4cash11Transaction7executeEv
// DEMANGLED: nexon::cash::Transaction::execute(void)
int Transaction::execute() {
    if (user_->balance() < amount_) {
        return 3;
    }
    user_->balance(-amount_);
    TransactionManager::instance()->addTransaction(this);
    return 0;
}

// IDA_EA: 0x81B0398
// IDA_NAME: _ZN5nexon4cash11Transaction7confirmEv
// DEMANGLED: nexon::cash::Transaction::confirm(void)
int Transaction::confirm() {
    return TransactionManager::instance()->removeTransaction(this);
}

// IDA_EA: 0x81B03B4
// IDA_NAME: _ZN5nexon4cash11Transaction6cancelEv
// DEMANGLED: nexon::cash::Transaction::cancel(void)
int Transaction::cancel() {
    user_->balance(amount_);
    return TransactionManager::instance()->removeTransaction(this);
}

// IDA_EA: 0x81B0444
// IDA_NAME: _ZN5nexon4cash18TransactionManager14getTransactionEj
// DEMANGLED: nexon::cash::TransactionManager::getTransaction(unsigned int)
Transaction* TransactionManager::getTransaction(uint32_t transaction_id) {
    std::map<uint32_t, Transaction*>::iterator found =
        transactions_.find(transaction_id);
    if (found == transactions_.end()) {
        return 0;
    }
    return found->second;
}

// IDA_EA: 0x81B04CC
// IDA_NAME: _ZN5nexon4cash18TransactionManager20getNextTransactionIDEv
// DEMANGLED: nexon::cash::TransactionManager::getNextTransactionID(void)
uint32_t TransactionManager::getNextTransactionID() {
    return ++tx_id_;
}

// IDA_EA: 0x81B04F8
// IDA_NAME: _ZN5nexon4cash18TransactionManagerC2Ev
// DEMANGLED: nexon::cash::TransactionManager::TransactionManager(void)
TransactionManager::TransactionManager()
    : transactions_() {}

// IDA_EA: 0x81B050C
// IDA_NAME: _ZNK5nexon4cash11Transaction2idEv
// DEMANGLED: nexon::cash::Transaction::id(void)const
uint32_t Transaction::id() const {
    return id_;
}

// IDA_EA: 0x81B0516
// IDA_NAME: _ZNK5nexon4cash11Transaction7getUserEv
// DEMANGLED: nexon::cash::Transaction::getUser(void)const
UserBalance* Transaction::getUser() const {
    return user_;
}

// IDA_EA: 0x81B0522
// IDA_NAME: _ZNK5nexon4cash11UserBalance7balanceEv
// DEMANGLED: nexon::cash::UserBalance::balance(void)const
int UserBalance::balance() const {
    return balance_;
}

// IDA_EA: 0x81B052E
// IDA_NAME: _ZN5nexon4cash11UserBalance7balanceEi
// DEMANGLED: nexon::cash::UserBalance::balance(int)
UserBalance* UserBalance::balance(int amount_delta) {
    balance_ += amount_delta;
    return this;
}

// IDA_EA: 0x81B0544
// IDA_NAME: _ZN5nexon4cash18TransactionManagerD2Ev
// DEMANGLED: nexon::cash::TransactionManager::~TransactionManager()
TransactionManager::~TransactionManager() {}

// IDA_EA: 0x81B147C
// IDA_NAME: _ZN5nexon4cash11UserBalance6expireEv
// DEMANGLED: nexon::cash::UserBalance::expire(void)
void UserBalance::expire() {
    for (std::vector<Transaction*>::iterator it = transactions_.begin();
         it != transactions_.end();) {
        Transaction* transaction = *it;
        if (transaction->expired()) {
            transaction->cancel();
            it = transactions_.erase(it);
        } else {
            ++it;
        }
    }
}

// IDA_EA: 0x81B1530
// IDA_NAME: _ZN5nexon4cash11UserBalance17removeTransactionEPNS0_11TransactionE
// DEMANGLED: nexon::cash::UserBalance::removeTransaction(nexon::cash::Transaction *)
std::vector<Transaction*>::iterator UserBalance::removeTransaction(
    Transaction* transaction) {
    return transactions_.erase(
        std::remove(transactions_.begin(), transactions_.end(), transaction),
        transactions_.end());
}

// IDA_EA: 0x81B15CA
// IDA_NAME: _ZN5nexon4cash11UserBalance17addNewTransactionEPNS0_11TransactionE
// DEMANGLED: nexon::cash::UserBalance::addNewTransaction(nexon::cash::Transaction *)
void UserBalance::addNewTransaction(Transaction* transaction) {
    std::printf(" addNewTx: %p\n", transaction);
    transactions_.push_back(transaction);
}

// IDA_EA: 0x81B1656
// IDA_NAME: _ZNK5nexon4cash11Transaction7expiredEv
// DEMANGLED: nexon::cash::Transaction::expired(void)const
bool Transaction::expired() const {
    return std::time(0) - created_time_ > 15;
}

}  // namespace cash
}  // namespace nexon
