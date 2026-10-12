#include "old_data/db/auction_cashshop/db_billing_record.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <stdio.h>
#include <time.h>


// IDA_EA: 0x844E526
// IDA_NAME: _ZN16DB_BillingRecordC2Ev
// DEMANGLED: DB_BillingRecord::DB_BillingRecord(void)
DB_BillingRecord::DB_BillingRecord() {}

// IDA_EA: 0x845EE6C
// IDA_NAME: _ZN16DB_BillingRecordD2Ev
// DEMANGLED: DB_BillingRecord::~DB_BillingRecord()
// IDA_EA: 0x845EE9C
// IDA_NAME: _ZN16DB_BillingRecordD0Ev
// DEMANGLED: DB_BillingRecord::~DB_BillingRecord() deleting destructor
DB_BillingRecord::~DB_BillingRecord() {}

// IDA_EA: 0x8424574
// IDA_NAME: _ZN16DB_BillingRecord8dispatchEiiP6Stream
// DEMANGLED: DB_BillingRecord::dispatch(int,int,Stream *)
bool DB_BillingRecord::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_BILLING_RECORD* record =
        stream->GetOutBuffer<SIG_BILLING_RECORD>();
    if (!record) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_BillingRecord::dispatch(int, int, Stream*)",
            0x606E,
            "SIG_BILLING_RECORD is NULL");
        return false;
    }

    bool inserted = false;
    InsertBuyUser(record, inserted);
    InsertSaleEntry(record, inserted);
    return true;
}

// IDA_EA: 0x8424602
// IDA_NAME: _ZN16DB_BillingRecord13InsertBuyUserEP18SIG_BILLING_RECORDRb
// DEMANGLED: DB_BillingRecord::InsertBuyUser(SIG_BILLING_RECORD *,bool &)
void DB_BillingRecord::InsertBuyUser(SIG_BILLING_RECORD* record,
                                     bool& inserted) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(7),static_cast<ENUM_SERVER_GROUP>( 0));
    inserted = false;
    db_handle->set_query(
        " upDate prod_buy_user  set user_id = '%s', sex = %d, birthday = '%s', last_buy_time = now()  where m_id = %d ",
        record->user_id,
        record->sex,
        record->birthday,
        record->m_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "void DB_BillingRecord::InsertBuyUser(SIG_BILLING_RECORD*, bool&)",
            0x6117,
            "[QUERY_ERROR] (upDate prod_buy_user m_id:%d)",
            record->m_id);
        return;
    }

    db_handle->set_query(
        "seLect m_id, user_id from prod_buy_user where m_id = %d",
        record->m_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "void DB_BillingRecord::InsertBuyUser(SIG_BILLING_RECORD*, bool&)",
            0x6120,
            "[QUERY_ERROR] (seLect prod_buy_user m_id:%d)",
            record->m_id);
        return;
    }
    if (db_handle->get_n_rows()) {
        return;
    }

    db_handle->set_query(
        " inSert into prod_buy_user ( m_id, user_id, sex, birthday, first_buy_time, last_buy_time )  values(%d, '%s', %d, '%s', now(), now() ) ",
        record->m_id,
        record->user_id,
        record->sex,
        record->birthday);
    inserted = true;
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "void DB_BillingRecord::InsertBuyUser(SIG_BILLING_RECORD*, bool&)",
            0x6134,
            "[QUERY_ERROR] (inSert into prod_buy_user m_id:%d)",
            record->m_id);
    }
}

// IDA_EA: 0x84247E0
// IDA_NAME: _ZN16DB_BillingRecord15InsertSaleEntryEP18SIG_BILLING_RECORDb
// DEMANGLED: DB_BillingRecord::InsertSaleEntry(SIG_BILLING_RECORD *,bool)
void DB_BillingRecord::InsertSaleEntry(SIG_BILLING_RECORD* record,
                                       bool inserted) {
    time_t timer;
    time(&timer);
    struct tm time_parts;
    localtime_r(&timer, &time_parts);
    char quarter[5] = {0};
    sprintf(quarter,
            "%02d%d",
            (time_parts.tm_year + 1900) % 2000,
            time_parts.tm_mon / 3 + 1);

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(7),static_cast<ENUM_SERVER_GROUP>( 0));
    const int new_flag = inserted ? 1 : 2;
    db_handle->set_query(
        " insert into prod_sale_entry_%s  ( agency_no, seal_flag, sale_time, price, slot_part_type, slot_part_code, ipg_no, bonus_ipg_no, m_id, user_id  , server_id, charac_no, job, grow_type, level, prize_flag, new_flag, result_code, result_message, gift_server_info, gift_get_charac_no  , charge_price, free_price, count_no, each_price, price_type, user_ip, buy_type, tran_id )  values( '%s', %d, now(), %d, %d, %d, %d, %d, %d, '%s', %d, %d, %d, %d, %d, %d, %d, %d, '%s', %d, %d, %d, %d, %d, %d, %d, '%s', %u, %u ) ",
        quarter,
        record->agency_no,
        record->seal_flag,
        record->price,
        record->slot_part_type,
        record->slot_part_code,
        record->ipg_no,
        record->bonus_ipg_no,
        record->m_id,
        record->user_id,
        record->server_id,
        record->charac_no,
        record->job,
        record->grow_type,
        record->level,
        record->prize_flag,
        new_flag,
        record->result_code,
        record->result_message,
        record->gift_server_info,
        record->gift_get_charac_no,
        record->charge_price,
        record->free_price,
        record->count_no,
        record->each_price,
        record->price_type,
        record->user_ip,
        static_cast<unsigned int>(record->buy_type),
        record->tran_id);
    if (db_handle->exec(true) == true) {
        return;
    }

    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "void DB_BillingRecord::InsertSaleEntry(SIG_BILLING_RECORD*, bool)",
        0x6191,
        "db insert error prod_sale_entry_%s m_id: %d",
        quarter,
        record->m_id);

    db_handle->set_query(
        " insert into prod_sale_entry_073 ( agency_no, seal_flag, sale_time, price, slot_part_type, slot_part_code, ipg_no, bonus_ipg_no, m_id, user_id, server_id, charac_no, job, prize_flag, new_flag, result_code, result_message )  values( '%s', %d, now(), %d, %d, %d, %d, %d, %d, '%s', %d, %d, %d, %d, %d, %d, '%s' ) ",
        record->agency_no,
        record->seal_flag,
        record->price,
        record->slot_part_type,
        record->slot_part_code,
        record->ipg_no,
        record->bonus_ipg_no,
        record->m_id,
        record->user_id,
        record->server_id,
        record->charac_no,
        record->job,
        record->prize_flag,
        new_flag,
        record->result_code,
        record->result_message);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "void DB_BillingRecord::InsertSaleEntry(SIG_BILLING_RECORD*, bool)",
            0x61AB,
            "db insert error prod_sale_entry_073 m_id: %d",
            record->m_id);
    }
}
