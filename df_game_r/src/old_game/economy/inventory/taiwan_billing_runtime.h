#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TAIWAN_BILLING_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TAIWAN_BILLING_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/inventory/ipg_data_abi.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/thread/mutex.h"

namespace Taiwan {

// IDA unit: 99_misc_global/Taiwan/BillTransaction.
struct BillTransaction {
    uint8_t status;                  // +0x00
    uint64_t transaction_id;         // +0x04

    // IDA_EA: 0x8175540
    // IDA_NAME: _ZN6Taiwan15BillTransaction4initEv
    // DEMANGLED: Taiwan::BillTransaction::init(void)
    BillTransaction* init();
};

struct EntranceQuery {
    // IDA_EA: 0x8173FFA
    // IDA_NAME: _ZN6Taiwan13EntranceQueryC2Ev
    // DEMANGLED: Taiwan::EntranceQuery::EntranceQuery(void)
    EntranceQuery();
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    int entrance;                    // +0x00
    int result_code;                 // +0x04
};

struct BalanceQuery {
    // IDA_EA: 0x8174030
    // IDA_NAME: _ZN6Taiwan12BalanceQueryC2EPKc
    // DEMANGLED: Taiwan::BalanceQuery::BalanceQuery(char const*)
    explicit BalanceQuery(const char* account_name);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    const char* account_name;        // +0x00
    uint32_t balance;                // +0x04
    int result_code;                 // +0x08
};

struct BalancePointQuery {
    // IDA_EA: 0x8174290
    // IDA_NAME: _ZN6Taiwan17BalancePointQueryC2EPKc
    // DEMANGLED: Taiwan::BalancePointQuery::BalancePointQuery(char const*)
    explicit BalancePointQuery(const char* account_name);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    const char* account_name;        // +0x00
    uint32_t balance;                // +0x04
    uint32_t point;                  // +0x08
    int result_code;                 // +0x0c
};

struct PurchaseQuery {
    // IDA_EA: 0x817454C
    // IDA_NAME: _ZN6Taiwan13PurchaseQueryC2EPKcS2_jjx
    // DEMANGLED: Taiwan::PurchaseQuery::PurchaseQuery(char const*,char const*,unsigned int,unsigned int,long long)
    PurchaseQuery(const char* account_name,
                  const char* character_name,
                  unsigned int product_id,
                  unsigned int price,
                  long long request_id);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    const char* account_name;        // +0x00
    const char* character_name;      // +0x04
    uint32_t product_id;             // +0x08
    long long request_id;            // +0x0c
    uint32_t price;                  // +0x14
    BillTransaction transaction;     // +0x18
    int result_code;                 // +0x24
};

struct GiftQuery {
    // IDA_EA: 0x81747FE
    // IDA_NAME: _ZN6Taiwan9GiftQueryC2EPKcS2_S2_jjx
    // DEMANGLED: Taiwan::GiftQuery::GiftQuery(char const*,char const*,char const*,unsigned int,unsigned int,long long)
    GiftQuery(const char* account_name,
              const char* sender_name,
              const char* recipient_name,
              unsigned int product_id,
              unsigned int price,
              long long request_id);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    const char* account_name;        // +0x00
    const char* sender_name;         // +0x04
    const char* recipient_name;      // +0x08
    long long request_id;            // +0x0c
    uint32_t product_id;             // +0x14
    uint32_t price;                  // +0x18
    int result_code;                 // +0x1c
    BillTransaction transaction;     // +0x20
};

struct ConfirmQuery {
    // IDA_EA: 0x8174AC6
    // IDA_NAME: _ZN6Taiwan12ConfirmQueryC2ERKNS_15BillTransactionE
    // DEMANGLED: Taiwan::ConfirmQuery::ConfirmQuery(Taiwan::BillTransaction const&)
    explicit ConfirmQuery(const BillTransaction& transaction_value);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    int result_code;                 // +0x00
    BillTransaction transaction;     // +0x04
};

struct CancelQuery {
    // IDA_EA: 0x8174CE2
    // IDA_NAME: _ZN6Taiwan11CancelQueryC2ERKNS_15BillTransactionE
    // DEMANGLED: Taiwan::CancelQuery::CancelQuery(Taiwan::BillTransaction const&)
    explicit CancelQuery(const BillTransaction& transaction_value);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    int result_code;                 // +0x00
    BillTransaction transaction;     // +0x04
};

struct ChargeQuery {
    // IDA_EA: 0x8174EFE
    // IDA_NAME: _ZN6Taiwan11ChargeQueryC2EPKcS2_S2_jc
    // DEMANGLED: Taiwan::ChargeQuery::ChargeQuery(char const*,char const*,char const*,unsigned int,char)
    ChargeQuery(const char* account_name,
                const char* character_name,
                const char* billing_route,
                unsigned int price,
                char charge_kind);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    const char* account_name;        // +0x00
    const char* character_name;      // +0x04
    const char* billing_route;       // +0x08
    uint32_t price;                  // +0x0c
    char charge_kind;                // +0x10
    int result_code;                 // +0x14
    BillTransaction transaction;     // +0x18
};

struct CeraPointQuery {
    // IDA_EA: 0x81751B6
    // IDA_NAME: _ZN6Taiwan14CeraPointQueryC2EPKcS2_PcS3_jccc
    // DEMANGLED: Taiwan::CeraPointQuery::CeraPointQuery(char const*,char const*,char *,char *,unsigned int,char,char,char)
    CeraPointQuery(const char* account_name,
                   const char* character_name,
                   char* point_value,
                   char* point_context,
                   unsigned int price,
                   char charge_kind,
                   char point_kind,
                   char command);
    bool setQuery(MySQL& database);
    bool execQuery(MySQL& database);
    bool getResult(MySQL& database);

    const char* account_name;        // +0x00
    const char* character_name;      // +0x04
    char* point_value;               // +0x08
    char* point_context;             // +0x0c
    uint32_t price;                  // +0x10
    char charge_kind;                // +0x14
    char point_kind;                 // +0x15
    char command;                    // +0x16
    int result_code;                 // +0x18
};

class TaiwanCash {
public:
    // IDA_EA: 0x8173608
    // IDA_NAME: _ZN6Taiwan10TaiwanCashC2Ehs
    // DEMANGLED: Taiwan::TaiwanCash::TaiwanCash(unsigned char,short)
    TaiwanCash(unsigned char service_kind, short billing_group);

    // IDA_EA: 0x817365A
    // IDA_NAME: _ZN6Taiwan10TaiwanCash15increaseRequestEv
    // DEMANGLED: Taiwan::TaiwanCash::increaseRequest(void)
    void increaseRequest();

    // IDA_EA: 0x817366E
    // IDA_NAME: _ZN6Taiwan10TaiwanCash15checkResultCodeEi
    // DEMANGLED: Taiwan::TaiwanCash::checkResultCode(int)
    bool checkResultCode(int result_code);

    // IDA_EA: 0x8173686
    // IDA_NAME: _ZN6Taiwan10TaiwanCash4initEPcjS1_S1_S1_
    // DEMANGLED: Taiwan::TaiwanCash::init(char *,unsigned int,char *,char *,char *)
    bool init(char* host,
              unsigned int port,
              char* database_name,
              char* user_name,
              char* password);

    int checkEntrance(const char* account_name, char& entrance);
    int getBalance(const char* account_name, int& balance);
    int getBalance(const char* account_name, int& balance, int& point);
    int purchase(unsigned long long request_id,
                 int product_id,
                 int price,
                 const char* account_name,
                 const char* character_name,
                 BillTransaction& transaction);
    int purchase(unsigned long long request_id,
                 int product_id,
                 int price,
                 const char* account_name,
                 const char* character_name,
                 BillTransaction& transaction,
                 short billing_group);
    int gift(unsigned long long request_id,
             int product_id,
             int price,
             const char* account_name,
             const char* sender_name,
             const char* recipient_name,
             const char* gift_message,
             BillTransaction& transaction);
    int confirm(const BillTransaction& transaction);
    int cancel(const BillTransaction& transaction);
    int charge(char* account_name,
               unsigned char charge_kind,
               unsigned char charge_subkind,
               unsigned int price,
               char* source,
               char* source_detail,
               char* source_extra,
               char* character_name,
               unsigned long long request_id,
               BillTransaction& transaction);
    int ceraPoint(char* account_name,
                  char* character_name,
                  char command,
                  char* point_value,
                  char* point_context,
                  char charge_kind,
                  char point_kind,
                  int price);

    template <typename Query>
    bool processBilling(Query& query);

    unsigned char service_kind;      // +0x00000
    short billing_group;             // +0x00002
    uint32_t request_count;          // +0x00004
    MySQL database;                  // +0x00008
    Mutex billing_mutex;             // +0x42094
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(BillTransaction) == 0x0cu,
              "BillTransaction IDA ABI size must stay 0x0c");
static_assert(offsetof(BillTransaction, transaction_id) == 0x04u,
              "BillTransaction transaction ID offset must stay 0x04");
static_assert(sizeof(EntranceQuery) == 0x08u,
              "EntranceQuery IDA ABI size must stay 0x08");
static_assert(sizeof(BalanceQuery) == 0x0cu,
              "BalanceQuery IDA ABI size must stay 0x0c");
static_assert(sizeof(BalancePointQuery) == 0x10u,
              "BalancePointQuery IDA ABI size must stay 0x10");
static_assert(sizeof(PurchaseQuery) == 0x28u,
              "PurchaseQuery IDA ABI size must stay 0x28");
static_assert(sizeof(GiftQuery) == 0x2cu,
              "GiftQuery IDA ABI size must stay 0x2c");
static_assert(sizeof(ConfirmQuery) == 0x10u,
              "ConfirmQuery IDA ABI size must stay 0x10");
static_assert(sizeof(CancelQuery) == 0x10u,
              "CancelQuery IDA ABI size must stay 0x10");
static_assert(sizeof(ChargeQuery) == 0x24u,
              "ChargeQuery IDA ABI size must stay 0x24");
static_assert(sizeof(CeraPointQuery) == 0x1cu,
              "CeraPointQuery IDA ABI size must stay 0x1c");
static_assert(offsetof(TaiwanCash, database) == 0x00008u,
              "TaiwanCash MySQL offset must stay 0x00008");
static_assert(offsetof(TaiwanCash, billing_mutex) == 0x42094u,
              "TaiwanCash mutex offset must stay 0x42094");
static_assert(sizeof(TaiwanCash) == 0x420acu,
              "TaiwanCash IDA ABI size must stay 0x420ac");
#endif

}  // namespace Taiwan

// IDA_EA: 0x8146669
// IDA_NAME: _Z21CheckBillingErrorCodei
// DEMANGLED: CheckBillingErrorCode(int)
// The point-auction split purchase accepts only the billing service's zero
// success code; keep this small global predicate distinct from TaiwanCash's
// private query-result checker.
int CheckBillingErrorCode(int result_code);

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TAIWAN_BILLING_RUNTIME_H_
