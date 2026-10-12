#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_TRANSACTION_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_TRANSACTION_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <ctime>
#include <map>
#include <vector>

namespace nexon {
namespace cash {

class Transaction;
class UserBalance;
struct CashTransactionLayoutVerifier;

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
// This manager owns only the in-flight transaction map.  Its static sequence
// starts at the old image value 0xffffffff, so the first generated ID is zero.
class TransactionManager {
public:
    // IDA_EA: 0x81B0128.
    static TransactionManager* instance();

    // IDA_EA: 0x81B04F8.
    TransactionManager();
    // IDA_EA: 0x81B0544.
    ~TransactionManager();

    // IDA_EA: 0x81B01BC.
    void addTransaction(Transaction* transaction);
    // IDA_EA: 0x81B021C.
    int removeTransaction(Transaction* transaction);
    // IDA_EA: 0x81B0244.
    void cancel(uint32_t transaction_id);
    // IDA_EA: 0x81B0298.
    void confirm(uint32_t transaction_id);
    // IDA_EA: 0x81B0444.
    Transaction* getTransaction(uint32_t transaction_id);
    // IDA_EA: 0x81B04CC.
    static uint32_t getNextTransactionID();

private:
    friend struct CashTransactionLayoutVerifier;

    std::map<uint32_t, Transaction*> transactions_;  // +0x00
    static uint32_t tx_id_;
};

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
class Transaction {
public:
    // IDA_EA: 0x81B02EC.
    Transaction(UserBalance* user, int amount);

    // IDA_EA: 0x81B0336.
    int execute();
    // IDA_EA: 0x81B0398.
    int confirm();
    // IDA_EA: 0x81B03B4.
    int cancel();
    // IDA_EA: 0x81B050C.
    uint32_t id() const;
    // IDA_EA: 0x81B0516.
    UserBalance* getUser() const;
    // IDA_EA: 0x81B1656.
    bool expired() const;

private:
    friend struct CashTransactionLayoutVerifier;

    uint32_t id_;           // +0x00
    int amount_;            // +0x04
    time_t created_time_;   // +0x08
    UserBalance* user_;     // +0x0c
};

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
// The first word is not read by this function family.  It remains a typed
// scalar rather than an invented semantic field or opaque backing storage.
class UserBalance {
public:
    // IDA_EA: 0x81B0522.
    int balance() const;
    // IDA_EA: 0x81B052E.
    UserBalance* balance(int amount_delta);

    // IDA_EA: 0x81B147C.
    void expire();

    // IDA_EA: 0x81B1530.
    // The old body directly returns vector::erase(remove(...), end()).  On
    // i386 this iterator is a one-word object returned in EAX.
    std::vector<Transaction*>::iterator removeTransaction(
        Transaction* transaction);

    // IDA_EA: 0x81B15CA.
    void addNewTransaction(Transaction* transaction);

private:
    friend struct CashTransactionLayoutVerifier;

    uint32_t unobserved_prefix_;               // +0x00
    int balance_;                              // +0x04
    std::vector<Transaction*> transactions_;  // +0x08
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct CashTransactionLayoutVerifier {
    static_assert(sizeof(TransactionManager) == 0x18u,
                  "TransactionManager old i386 ABI must stay 0x18");
    static_assert(offsetof(TransactionManager, transactions_) == 0x00u,
                  "TransactionManager map must remain at +0");
    static_assert(sizeof(Transaction) == 0x10u,
                  "Transaction old i386 ABI must stay 0x10");
    static_assert(offsetof(Transaction, id_) == 0x00u,
                  "Transaction id must remain at +0");
    static_assert(offsetof(Transaction, amount_) == 0x04u,
                  "Transaction amount must remain at +4");
    static_assert(offsetof(Transaction, created_time_) == 0x08u,
                  "Transaction creation time must remain at +8");
    static_assert(offsetof(Transaction, user_) == 0x0cu,
                  "Transaction user must remain at +12");
    static_assert(sizeof(UserBalance) == 0x14u,
                  "UserBalance old i386 ABI must stay 0x14");
    static_assert(offsetof(UserBalance, unobserved_prefix_) == 0x00u,
                  "UserBalance unobserved word must remain at +0");
    static_assert(offsetof(UserBalance, balance_) == 0x04u,
                  "UserBalance balance must remain at +4");
    static_assert(offsetof(UserBalance, transactions_) == 0x08u,
                  "UserBalance transactions must remain at +8");
};
#endif

}  // namespace cash
}  // namespace nexon

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_TRANSACTION_RUNTIME_H_
