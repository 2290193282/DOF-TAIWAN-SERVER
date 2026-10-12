#include "old_game/economy/inventory/taiwan_billing_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/concurrent/thread/guard_mutex.h"

namespace {

// IDA_DATA: byte_8B84775 @ 0x8B84775 is the empty billing-route string used
// only by TaiwanCash::charge before it constructs ChargeQuery.
const char kDefaultBillingRoute[] = "";

}  // namespace

namespace Taiwan {

// IDA_EA: 0x8175540
// IDA_NAME: _ZN6Taiwan15BillTransaction4initEv
// DEMANGLED: Taiwan::BillTransaction::init(void)
BillTransaction* BillTransaction::init() {
    status = 0;
    transaction_id = 0;
    return this;
}

// IDA_EA: 0x8173FFA
// IDA_NAME: _ZN6Taiwan13EntranceQueryC2Ev
// DEMANGLED: Taiwan::EntranceQuery::EntranceQuery(void)
EntranceQuery::EntranceQuery() : entrance(0), result_code(0) {}

// IDA_EA: 0x8174012
// IDA_NAME: _ZN6Taiwan13EntranceQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::EntranceQuery::setQuery(MySQL &)
bool EntranceQuery::setQuery(MySQL& database) {
    (void)database;
    return true;
}

// IDA_EA: 0x817401C
// IDA_NAME: _ZN6Taiwan13EntranceQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::EntranceQuery::execQuery(MySQL &)
bool EntranceQuery::execQuery(MySQL& database) {
    (void)database;
    return true;
}

// IDA_EA: 0x8174026
// IDA_NAME: _ZN6Taiwan13EntranceQuery9getResultER5MySQL
// DEMANGLED: Taiwan::EntranceQuery::getResult(MySQL &)
bool EntranceQuery::getResult(MySQL& database) {
    (void)database;
    return true;
}

// IDA_EA: 0x8174030
// IDA_NAME: _ZN6Taiwan12BalanceQueryC2EPKc
// DEMANGLED: Taiwan::BalanceQuery::BalanceQuery(char const*)
BalanceQuery::BalanceQuery(const char* account_name_value)
    : account_name(account_name_value), balance(0), result_code(0) {}

// IDA_EA: 0x8174052
// IDA_NAME: _ZN6Taiwan12BalanceQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::BalanceQuery::setQuery(MySQL &)
bool BalanceQuery::setQuery(MySQL& database) {
    return database.set_query(
        "call usp_balance('%s', @out_balance, @out_result)", account_name);
}

// IDA_EA: 0x8174076
// IDA_NAME: _ZN6Taiwan12BalanceQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::BalanceQuery::execQuery(MySQL &)
bool BalanceQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x8174092
// IDA_NAME: _ZN6Taiwan12BalanceQuery9getResultER5MySQL
// DEMANGLED: Taiwan::BalanceQuery::getResult(MySQL &)
bool BalanceQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_balance, @out_result")) {
        throw 278;
    }
    if (!database.exec(true)) {
        throw 281;
    }
    if (!database.fetch()) {
        throw 284;
    }
    if (!database.get_int(0, balance)) {
        throw 288;
    }
    if (!database.get_int(1, result_code)) {
        throw 291;
    }
    return true;
}

// IDA_EA: 0x8174290
// IDA_NAME: _ZN6Taiwan17BalancePointQueryC2EPKc
// DEMANGLED: Taiwan::BalancePointQuery::BalancePointQuery(char const*)
BalancePointQuery::BalancePointQuery(const char* account_name_value)
    : account_name(account_name_value), balance(0), point(0), result_code(0) {}

// IDA_EA: 0x81742BC
// IDA_NAME: _ZN6Taiwan17BalancePointQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::BalancePointQuery::setQuery(MySQL &)
bool BalancePointQuery::setQuery(MySQL& database) {
    return database.set_query(
        "call usp_balance_point('%s', @out_balance, @out_point, @out_result)",
        account_name);
}

// IDA_EA: 0x81742E0
// IDA_NAME: _ZN6Taiwan17BalancePointQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::BalancePointQuery::execQuery(MySQL &)
bool BalancePointQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x81742FC
// IDA_NAME: _ZN6Taiwan17BalancePointQuery9getResultER5MySQL
// DEMANGLED: Taiwan::BalancePointQuery::getResult(MySQL &)
bool BalancePointQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_balance, @out_point, @out_result")) {
        throw 331;
    }
    if (!database.exec(true)) {
        throw 334;
    }
    if (!database.fetch()) {
        throw 337;
    }
    if (!database.get_int(0, balance)) {
        throw 341;
    }
    if (!database.get_int(1, point)) {
        throw 344;
    }
    if (!database.get_int(2, result_code)) {
        throw 347;
    }
    return true;
}

// IDA_EA: 0x817454C
// IDA_NAME: _ZN6Taiwan13PurchaseQueryC2EPKcS2_jjx
// DEMANGLED: Taiwan::PurchaseQuery::PurchaseQuery(char const*,char const*,unsigned int,unsigned int,long long)
PurchaseQuery::PurchaseQuery(const char* account_name_value,
                             const char* character_name_value,
                             unsigned int product_id_value,
                             unsigned int price_value,
                             long long request_id_value)
    : account_name(account_name_value),
      character_name(character_name_value),
      product_id(product_id_value),
      request_id(request_id_value),
      price(price_value),
      result_code(0) {
    transaction.init();
}

// IDA_EA: 0x81745AA
// IDA_NAME: _ZN6Taiwan13PurchaseQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::PurchaseQuery::setQuery(MySQL &)
bool PurchaseQuery::setQuery(MySQL& database) {
    return database.set_query(
        "call usp_purchase('%s', '%s', %u, %u, @out_tran, @out_code)",
        account_name,
        character_name,
        product_id,
        price);
}

// IDA_EA: 0x81745F2
// IDA_NAME: _ZN6Taiwan13PurchaseQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::PurchaseQuery::execQuery(MySQL &)
bool PurchaseQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x817460E
// IDA_NAME: _ZN6Taiwan13PurchaseQuery9getResultER5MySQL
// DEMANGLED: Taiwan::PurchaseQuery::getResult(MySQL &)
bool PurchaseQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_tran, @out_code")) {
        throw 391;
    }
    if (!database.exec(true)) {
        throw 394;
    }
    if (!database.fetch()) {
        throw 397;
    }
    if (!database.get_ulonglong(0, transaction.transaction_id)) {
        throw 400;
    }
    if (!database.get_int(1, result_code)) {
        throw 403;
    }
    return true;
}

// IDA_EA: 0x81747FE
// IDA_NAME: _ZN6Taiwan9GiftQueryC2EPKcS2_S2_jjx
// DEMANGLED: Taiwan::GiftQuery::GiftQuery(char const*,char const*,char const*,unsigned int,unsigned int,long long)
GiftQuery::GiftQuery(const char* account_name_value,
                     const char* sender_name_value,
                     const char* recipient_name_value,
                     unsigned int product_id_value,
                     unsigned int price_value,
                     long long request_id_value)
    : account_name(account_name_value),
      sender_name(sender_name_value),
      recipient_name(recipient_name_value),
      request_id(request_id_value),
      product_id(product_id_value),
      price(price_value),
      result_code(0) {
    transaction.init();
}

// IDA_EA: 0x8174866
// IDA_NAME: _ZN6Taiwan9GiftQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::GiftQuery::setQuery(MySQL &)
bool GiftQuery::setQuery(MySQL& database) {
    return database.set_query(
        "call usp_gift('%s', '%s', '%s', %u, %u, @out_tran, @out_code)",
        account_name,
        sender_name,
        recipient_name,
        product_id,
        price);
}

// IDA_EA: 0x81748BA
// IDA_NAME: _ZN6Taiwan9GiftQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::GiftQuery::execQuery(MySQL &)
bool GiftQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x81748D6
// IDA_NAME: _ZN6Taiwan9GiftQuery9getResultER5MySQL
// DEMANGLED: Taiwan::GiftQuery::getResult(MySQL &)
bool GiftQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_tran, @out_code")) {
        throw 447;
    }
    if (!database.exec(true)) {
        throw 450;
    }
    if (!database.fetch()) {
        throw 453;
    }
    if (!database.get_ulonglong(0, transaction.transaction_id)) {
        throw 456;
    }
    if (!database.get_int(1, result_code)) {
        throw 459;
    }
    return true;
}

// IDA_EA: 0x8174AC6
// IDA_NAME: _ZN6Taiwan12ConfirmQueryC2ERKNS_15BillTransactionE
// DEMANGLED: Taiwan::ConfirmQuery::ConfirmQuery(Taiwan::BillTransaction const&)
ConfirmQuery::ConfirmQuery(const BillTransaction& transaction_value)
    : result_code(0), transaction(transaction_value) {}

// IDA_EA: 0x8174AEC
// IDA_NAME: _ZN6Taiwan12ConfirmQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::ConfirmQuery::setQuery(MySQL &)
bool ConfirmQuery::setQuery(MySQL& database) {
    return database.set_query("call usp_confirm(%s, @out_code)",
                              NumberToString(transaction.transaction_id, 0));
}

// IDA_EA: 0x8174B28
// IDA_NAME: _ZN6Taiwan12ConfirmQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::ConfirmQuery::execQuery(MySQL &)
bool ConfirmQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x8174B44
// IDA_NAME: _ZN6Taiwan12ConfirmQuery9getResultER5MySQL
// DEMANGLED: Taiwan::ConfirmQuery::getResult(MySQL &)
bool ConfirmQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_code")) {
        throw 494;
    }
    if (!database.exec(true)) {
        throw 497;
    }
    if (!database.fetch()) {
        throw 500;
    }
    if (!database.get_int(0, result_code)) {
        throw 503;
    }
    return true;
}

// IDA_EA: 0x8174CE2
// IDA_NAME: _ZN6Taiwan11CancelQueryC2ERKNS_15BillTransactionE
// DEMANGLED: Taiwan::CancelQuery::CancelQuery(Taiwan::BillTransaction const&)
CancelQuery::CancelQuery(const BillTransaction& transaction_value)
    : result_code(0), transaction(transaction_value) {}

// IDA_EA: 0x8174D08
// IDA_NAME: _ZN6Taiwan11CancelQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::CancelQuery::setQuery(MySQL &)
bool CancelQuery::setQuery(MySQL& database) {
    return database.set_query("call usp_cancel(%s, @out_code)",
                              NumberToString(transaction.transaction_id, 0));
}

// IDA_EA: 0x8174D44
// IDA_NAME: _ZN6Taiwan11CancelQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::CancelQuery::execQuery(MySQL &)
bool CancelQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x8174D60
// IDA_NAME: _ZN6Taiwan11CancelQuery9getResultER5MySQL
// DEMANGLED: Taiwan::CancelQuery::getResult(MySQL &)
bool CancelQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_code")) {
        throw 538;
    }
    if (!database.exec(true)) {
        throw 541;
    }
    if (!database.fetch()) {
        throw 544;
    }
    if (!database.get_int(0, result_code)) {
        throw 547;
    }
    return true;
}

// IDA_EA: 0x8174EFE
// IDA_NAME: _ZN6Taiwan11ChargeQueryC2EPKcS2_S2_jc
// DEMANGLED: Taiwan::ChargeQuery::ChargeQuery(char const*,char const*,char const*,unsigned int,char)
ChargeQuery::ChargeQuery(const char* account_name_value,
                         const char* character_name_value,
                         const char* billing_route_value,
                         unsigned int price_value,
                         char charge_kind_value)
    : account_name(account_name_value),
      character_name(character_name_value),
      billing_route(billing_route_value),
      price(price_value),
      charge_kind(charge_kind_value),
      result_code(0) {
    transaction.init();
}

// IDA_EA: 0x8174F52
// IDA_NAME: _ZN6Taiwan11ChargeQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::ChargeQuery::setQuery(MySQL &)
bool ChargeQuery::setQuery(MySQL& database) {
    return database.set_query(
        "call usp_recharge('%s', '%s', '%s', %u, %d, @out_tran, @out_code)",
        account_name,
        character_name,
        billing_route,
        price,
        charge_kind);
}

// IDA_EA: 0x8174FAA
// IDA_NAME: _ZN6Taiwan11ChargeQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::ChargeQuery::execQuery(MySQL &)
bool ChargeQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x8174FC6
// IDA_NAME: _ZN6Taiwan11ChargeQuery9getResultER5MySQL
// DEMANGLED: Taiwan::ChargeQuery::getResult(MySQL &)
bool ChargeQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_tran, @out_code")) {
        throw 588;
    }
    if (!database.exec(true)) {
        throw 591;
    }
    if (!database.fetch()) {
        throw 594;
    }
    if (!database.get_ulonglong(0, transaction.transaction_id)) {
        throw 597;
    }
    if (!database.get_int(1, result_code)) {
        throw 600;
    }
    return true;
}

// IDA_EA: 0x81751B6
// IDA_NAME: _ZN6Taiwan14CeraPointQueryC2EPKcS2_PcS3_jccc
// DEMANGLED: Taiwan::CeraPointQuery::CeraPointQuery(char const*,char const*,char *,char *,unsigned int,char,char,char)
CeraPointQuery::CeraPointQuery(const char* account_name_value,
                               const char* character_name_value,
                               char* point_value_value,
                               char* point_context_value,
                               unsigned int price_value,
                               char charge_kind_value,
                               char point_kind_value,
                               char command_value)
    : account_name(account_name_value),
      character_name(character_name_value),
      point_value(point_value_value),
      point_context(point_context_value),
      price(price_value),
      charge_kind(charge_kind_value),
      point_kind(point_kind_value),
      command(command_value),
      result_code(0) {}

// IDA_EA: 0x8175224
// IDA_NAME: _ZN6Taiwan14CeraPointQuery8setQueryER5MySQL
// DEMANGLED: Taiwan::CeraPointQuery::setQuery(MySQL &)
bool CeraPointQuery::setQuery(MySQL& database) {
    return database.set_query(
        "call usp_point_process('%s', '%s', '%c', %u, %d, %d , %d, @out_code)",
        account_name,
        character_name,
        command,
        price,
        charge_kind,
        StringToNumber(point_value),
        point_kind);
}

// IDA_EA: 0x81752B0
// IDA_NAME: _ZN6Taiwan14CeraPointQuery9execQueryER5MySQL
// DEMANGLED: Taiwan::CeraPointQuery::execQuery(MySQL &)
bool CeraPointQuery::execQuery(MySQL& database) {
    return database.exec(true);
}

// IDA_EA: 0x81752CC
// IDA_NAME: _ZN6Taiwan14CeraPointQuery9getResultER5MySQL
// DEMANGLED: Taiwan::CeraPointQuery::getResult(MySQL &)
bool CeraPointQuery::getResult(MySQL& database) {
    if (!database.set_query("select @out_code")) {
        throw 642;
    }
    if (!database.exec(true)) {
        throw 645;
    }
    if (!database.fetch()) {
        throw 648;
    }
    if (!database.get_int(0, result_code)) {
        throw 651;
    }
    return true;
}

// IDA_EA: 0x817555C
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_13EntranceQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::EntranceQuery>(Taiwan::EntranceQuery &)
template <>
bool TaiwanCash::processBilling<EntranceQuery>(EntranceQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x81755F8
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_12BalanceQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::BalanceQuery>(Taiwan::BalanceQuery &)
template <>
bool TaiwanCash::processBilling<BalanceQuery>(BalanceQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x81756B4
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_17BalancePointQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::BalancePointQuery>(Taiwan::BalancePointQuery &)
template <>
bool TaiwanCash::processBilling<BalancePointQuery>(BalancePointQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x8175770
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_13PurchaseQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::PurchaseQuery>(Taiwan::PurchaseQuery &)
template <>
bool TaiwanCash::processBilling<PurchaseQuery>(PurchaseQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x817582C
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_9GiftQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::GiftQuery>(Taiwan::GiftQuery &)
template <>
bool TaiwanCash::processBilling<GiftQuery>(GiftQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x81758E8
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_12ConfirmQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::ConfirmQuery>(Taiwan::ConfirmQuery &)
template <>
bool TaiwanCash::processBilling<ConfirmQuery>(ConfirmQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x81759A4
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_11CancelQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::CancelQuery>(Taiwan::CancelQuery &)
template <>
bool TaiwanCash::processBilling<CancelQuery>(CancelQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x8175A60
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_11ChargeQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::ChargeQuery>(Taiwan::ChargeQuery &)
template <>
bool TaiwanCash::processBilling<ChargeQuery>(ChargeQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x8175B1C
// IDA_NAME: _ZN6Taiwan10TaiwanCash14processBillingINS_14CeraPointQueryEEEbRT_
// DEMANGLED: bool Taiwan::TaiwanCash::processBilling<Taiwan::CeraPointQuery>(Taiwan::CeraPointQuery &)
template <>
bool TaiwanCash::processBilling<CeraPointQuery>(CeraPointQuery& query) {
    Guard<Mutex> guard(&billing_mutex);
    increaseRequest();
    if (query.setQuery(database) && query.execQuery(database)) {
        return query.getResult(database);
    }
    return false;
}

// IDA_EA: 0x8173608
// IDA_NAME: _ZN6Taiwan10TaiwanCashC2Ehs
// DEMANGLED: Taiwan::TaiwanCash::TaiwanCash(unsigned char,short)
TaiwanCash::TaiwanCash(unsigned char service_kind_value, short billing_group_value)
    : database(), billing_mutex() {
    request_count = 0;
    service_kind = service_kind_value;
    billing_group = billing_group_value;
}

// IDA_EA: 0x817365A
// IDA_NAME: _ZN6Taiwan10TaiwanCash15increaseRequestEv
// DEMANGLED: Taiwan::TaiwanCash::increaseRequest(void)
void TaiwanCash::increaseRequest() {
    ++request_count;
}

// IDA_EA: 0x817366E
// IDA_NAME: _ZN6Taiwan10TaiwanCash15checkResultCodeEi
// DEMANGLED: Taiwan::TaiwanCash::checkResultCode(int)
bool TaiwanCash::checkResultCode(int result_code_value) {
    return result_code_value == 0;
}

// IDA_EA: 0x8173686
// IDA_NAME: _ZN6Taiwan10TaiwanCash4initEPcjS1_S1_S1_
// DEMANGLED: Taiwan::TaiwanCash::init(char *,unsigned int,char *,char *,char *)
bool TaiwanCash::init(char* host,
                      unsigned int port,
                      char* database_name,
                      char* user_name,
                      char* password) {
    database.init();
    return database.open(host, port, database_name, user_name, password);
}

// IDA_EA: 0x81736CE
// IDA_NAME: _ZN6Taiwan10TaiwanCash13checkEntranceEPKcRc
// DEMANGLED: Taiwan::TaiwanCash::checkEntrance(char const*,char &)
int TaiwanCash::checkEntrance(const char* account_name, char& entrance) {
    (void)account_name;
    (void)entrance;
    EntranceQuery query;
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    cMyTrace trace("checkEntrance", 52, 0);
    trace("[Taiwan, Billing] EntranceQuery Entrance:%d", query.result_code);
    return 0;
}

// IDA_EA: 0x8173766
// IDA_NAME: _ZN6Taiwan10TaiwanCash10getBalanceEPKcRi
// DEMANGLED: Taiwan::TaiwanCash::getBalance(char const*,int &)
int TaiwanCash::getBalance(const char* account_name, int& balance_value) {
    BalanceQuery query(account_name);
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    balance_value = static_cast<int>(query.balance);
    cMyTrace trace("getBalance", 70, 0);
    trace("[Taiwan, Billing] BalanceQuery Balance:%d, Result:%d",
          balance_value,
          query.result_code);
    return 0;
}

// IDA_EA: 0x817381A
// IDA_NAME: _ZN6Taiwan10TaiwanCash10getBalanceEPKcRiS3_
// DEMANGLED: Taiwan::TaiwanCash::getBalance(char const*,int &,int &)
int TaiwanCash::getBalance(const char* account_name,
                           int& balance_value,
                           int& point_value) {
    BalancePointQuery query(account_name);
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    balance_value = static_cast<int>(query.balance);
    point_value = static_cast<int>(query.point);
    cMyTrace trace("getBalance", 90, 0);
    trace("[Taiwan, Billing] BalancePointQuery Balance:%d, Point:%d, Result:%d",
          balance_value,
          point_value,
          query.result_code);
    return 0;
}

// IDA_EA: 0x81738E6
// IDA_NAME: _ZN6Taiwan10TaiwanCash8purchaseEyiiPKcS2_RNS_15BillTransactionE
// DEMANGLED: Taiwan::TaiwanCash::purchase(unsigned long long,int,int,char const*,char const*,Taiwan::BillTransaction &)
int TaiwanCash::purchase(unsigned long long request_id_value,
                         int product_id_value,
                         int price_value,
                         const char* account_name,
                         const char* character_name,
                         BillTransaction& transaction_value) {
    PurchaseQuery query(account_name,
                        character_name,
                        static_cast<unsigned int>(product_id_value),
                        static_cast<unsigned int>(price_value),
                        static_cast<long long>(request_id_value));
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    transaction_value = query.transaction;
    cMyTrace trace("purchase", 110, 0);
    trace("[Taiwan, Billing] PurchaseQuery Account:%s, Charac:%s, Price:%d, Transaction:%s",
          account_name,
          character_name,
          price_value,
          NumberToString(transaction_value.transaction_id, 0));
    return 0;
}

// IDA_EA: 0x8173A00
// IDA_NAME: _ZN6Taiwan10TaiwanCash8purchaseEyiiPKcS2_RNS_15BillTransactionEs
// DEMANGLED: Taiwan::TaiwanCash::purchase(unsigned long long,int,int,char const*,char const*,Taiwan::BillTransaction &,short)
int TaiwanCash::purchase(unsigned long long request_id_value,
                         int product_id_value,
                         int price_value,
                         const char* account_name,
                         const char* character_name,
                         BillTransaction& transaction_value,
                         short billing_group_value) {
    (void)billing_group_value;
    PurchaseQuery query(account_name,
                        character_name,
                        static_cast<unsigned int>(product_id_value),
                        static_cast<unsigned int>(price_value),
                        static_cast<long long>(request_id_value));
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    transaction_value = query.transaction;
    cMyTrace trace("purchase", 129, 0);
    trace("[Taiwan, Billing] PurchaseQuery Account:%s, Charac:%s, Price:%d, Transaction:%s",
          account_name,
          character_name,
          price_value,
          NumberToString(transaction_value.transaction_id, 0));
    return 0;
}

// IDA_EA: 0x8173B20
// IDA_NAME: _ZN6Taiwan10TaiwanCash4giftEyiiPKcS2_S2_S2_RNS_15BillTransactionE
// DEMANGLED: Taiwan::TaiwanCash::gift(unsigned long long,int,int,char const*,char const*,char const*,char const*,Taiwan::BillTransaction &)
int TaiwanCash::gift(unsigned long long request_id_value,
                     int product_id_value,
                     int price_value,
                     const char* account_name,
                     const char* sender_name,
                     const char* recipient_name,
                     const char* gift_message,
                     BillTransaction& transaction_value) {
    (void)gift_message;
    GiftQuery query(account_name,
                    sender_name,
                    recipient_name,
                    static_cast<unsigned int>(product_id_value),
                    static_cast<unsigned int>(price_value),
                    static_cast<long long>(request_id_value));
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    transaction_value = query.transaction;
    cMyTrace trace("gift", 149, 0);
    trace("[Taiwan, Billing] GiftQuery Account:%s, SendCharac:%s, RecvCharac:%s, Price:%d, Transaction:%s",
          account_name,
          sender_name,
          recipient_name,
          price_value,
          NumberToString(transaction_value.transaction_id, 0));
    return 0;
}

// IDA_EA: 0x8173C48
// IDA_NAME: _ZN6Taiwan10TaiwanCash7confirmERKNS_15BillTransactionE
// DEMANGLED: Taiwan::TaiwanCash::confirm(Taiwan::BillTransaction const&)
int TaiwanCash::confirm(const BillTransaction& transaction_value) {
    if (!transaction_value.transaction_id) {
        return 0;
    }
    ConfirmQuery query(transaction_value);
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    cMyTrace trace("confirm", 167, 0);
    trace("[Taiwan, Billing] ConfirmQuery Transaction:%s",
          NumberToString(transaction_value.transaction_id, 0));
    return 0;
}

// IDA_EA: 0x8173D1C
// IDA_NAME: _ZN6Taiwan10TaiwanCash6cancelERKNS_15BillTransactionE
// DEMANGLED: Taiwan::TaiwanCash::cancel(Taiwan::BillTransaction const&)
int TaiwanCash::cancel(const BillTransaction& transaction_value) {
    if (!transaction_value.transaction_id) {
        return 0;
    }
    CancelQuery query(transaction_value);
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    cMyTrace trace("cancel", 185, 0);
    trace("[Taiwan, Billing] CancelQuery Transaction:%s",
          NumberToString(transaction_value.transaction_id, 0));
    return 0;
}

// IDA_EA: 0x8173DF0
// IDA_NAME: _ZN6Taiwan10TaiwanCash6chargeEPchhjS1_S1_S1_S1_yRNS_15BillTransactionE
// DEMANGLED: Taiwan::TaiwanCash::charge(char *,unsigned char,unsigned char,unsigned int,char *,char *,char *,char *,unsigned long long,Taiwan::BillTransaction &)
int TaiwanCash::charge(char* account_name,
                       unsigned char charge_kind_value,
                       unsigned char charge_subkind,
                       unsigned int price_value,
                       char* source,
                       char* source_detail,
                       char* source_extra,
                       char* character_name,
                       unsigned long long request_id_value,
                       BillTransaction& transaction_value) {
    (void)charge_subkind;
    (void)source;
    (void)source_detail;
    (void)source_extra;
    (void)request_id_value;
    ChargeQuery query(account_name,
                      character_name,
                      kDefaultBillingRoute,
                      price_value,
                      static_cast<char>(charge_kind_value));
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    transaction_value = query.transaction;
    cMyTrace trace("charge", 206, 0);
    trace("[Taiwan, Billing] ChargeQuery Account:%s, Price:%d, Transaction:%s",
          account_name,
          price_value,
          NumberToString(transaction_value.transaction_id, 0));
    return 0;
}

// IDA_EA: 0x8173F0C
// IDA_NAME: _ZN6Taiwan10TaiwanCash9ceraPointEPcS1_cS1_S1_cci
// DEMANGLED: Taiwan::TaiwanCash::ceraPoint(char *,char *,char,char *,char *,char,char,int)
int TaiwanCash::ceraPoint(char* account_name,
                          char* character_name,
                          char command_value,
                          char* point_value,
                          char* point_context,
                          char charge_kind_value,
                          char point_kind_value,
                          int price_value) {
    CeraPointQuery query(account_name,
                          character_name,
                          point_value,
                          point_context,
                          static_cast<unsigned int>(price_value),
                          charge_kind_value,
                          point_kind_value,
                          command_value);
    if (!processBilling(query)) {
        return 4;
    }
    if (!checkResultCode(query.result_code)) {
        return query.result_code;
    }
    cMyTrace trace("ceraPoint", 224, 0);
    trace("[Taiwan, Billing] CeraPointQuery Command:%c, Price:%d",
          command_value,
          price_value);
    return 0;
}

}  // namespace Taiwan

// IDA_EA: 0x8146669
// IDA_NAME: _Z21CheckBillingErrorCodei
// DEMANGLED: CheckBillingErrorCode(int)
int CheckBillingErrorCode(int result_code) {
    return result_code == 0 ? 1 : 0;
}
