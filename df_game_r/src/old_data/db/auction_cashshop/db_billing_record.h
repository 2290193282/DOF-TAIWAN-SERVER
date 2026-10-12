#ifndef OLDSRV_DB_DB_BILLING_RECORD_H_
#define OLDSRV_DB_DB_BILLING_RECORD_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


#pragma pack(push, 1)
// IDA_EA: 0x8451E9A
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_BILLING_RECORDEEPT_v
// Stream::GetOutBuffer advances exactly 270 bytes.  Every field below is
// consumed at the shown wire position by 0x8424602 or 0x84247E0.
struct SIG_BILLING_RECORD {
    char agency_no[21];             // +0
    int32_t seal_flag;              // +21
    int32_t price;                  // +25
    int32_t slot_part_type;         // +29
    int32_t slot_part_code;         // +33
    int32_t ipg_no;                 // +37
    int32_t bonus_ipg_no;           // +41
    uint32_t m_id;                  // +45
    uint32_t charac_no;             // +49
    char user_id[25];               // +53
    int32_t job;                    // +78
    int32_t sex;                    // +82
    char birthday[11];              // +86
    int32_t prize_flag;             // +97
    int32_t result_code;            // +101
    char result_message[100];       // +105
    int32_t server_id;              // +205
    int32_t grow_type;              // +209
    int32_t level;                  // +213
    int32_t gift_server_info;       // +217
    uint32_t gift_get_charac_no;    // +221
    int32_t charge_price;           // +225
    int32_t free_price;             // +229
    int32_t count_no;               // +233
    int32_t each_price;             // +237
    int32_t price_type;             // +241
    char user_ip[16];               // +245
    uint32_t tran_id;               // +261
    uint32_t unused_265;            // +265; not read by the three IDA bodies
    uint8_t buy_type;               // +269
};
#pragma pack(pop)

static_assert(sizeof(SIG_BILLING_RECORD) == 270,
              "SIG_BILLING_RECORD ABI drift");


// IDA unit: 13_database_dispatch/DB_BillingRecord; DBTask signal 176.
class DB_BillingRecord : public DBDispatcher {
public:
    DB_BillingRecord();
    virtual ~DB_BillingRecord();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    void InsertBuyUser(SIG_BILLING_RECORD* record, bool& inserted);
    void InsertSaleEntry(SIG_BILLING_RECORD* record, bool inserted);
};

#endif  // OLDSRV_DB_DB_BILLING_RECORD_H_
