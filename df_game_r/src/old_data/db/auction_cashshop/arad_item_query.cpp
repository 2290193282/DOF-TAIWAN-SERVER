#include "old_data/db/auction_cashshop/arad_item_query.h"

#include "common/manager/arad_quarter_of_year.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

#include <stdint.h>


namespace {

const ENUM_DB_HANDLE_IDX kBuyItemCheckDbHandle =
    static_cast<ENUM_DB_HANDLE_IDX>(7);
const ENUM_SERVER_GROUP kDefaultServerGroup = SERVER_GROUP_DEFAULT;

}  // namespace


namespace ARAD {
namespace DATABASE {
namespace ITEM_QUERY {

// IDA_EA: 0x818CFAB
// IDA_NAME: _ZN4ARAD8DATABASE10ITEM_QUERY22arad_sp_buy_item_checkERN10ARAD_DATAS11BuyItemInfoE
// DEMANGLED: ARAD::DATABASE::ITEM_QUERY::arad_sp_buy_item_check(ARAD_DATAS::BuyItemInfo &)
int arad_sp_buy_item_check(ARAD_DATAS::BuyItemInfo& info) {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kBuyItemCheckDbHandle, kDefaultServerGroup);
    if (!db_handle) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::ITEM_QUERY::arad_sp_buy_item_check(ARAD_DATAS::BuyItemInfo&)",
            0x454,
            "%s",
            "Database null!!");
        return -1;
    }

    char quarter_of_year[5] = {0};
    ARAD::GetQuarterOfYear(quarter_of_year);
    if (info.check_type == 1) {
        db_handle->set_query(
            "select count(*) from prod_sale_entry_%s where  ipg_no = %d and  m_id = %d   and  charac_no = %d",
            quarter_of_year,
            info.ipg_no,
            info.member_id,
            info.charac_no);
    } else {
        db_handle->set_query(
            "select count(*) from prod_sale_entry_%s where  ipg_no = %d and  m_id = %d",
            quarter_of_year,
            info.ipg_no,
            info.member_id);
    }

    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::ITEM_QUERY::arad_sp_buy_item_check(ARAD_DATAS::BuyItemInfo&)",
            0x472,
            "[ERROR] DB Exec Error => MID[%d], CHARIDX[%d], ITEMIDX[%d], CHECKTYPE[%d]",
            info.member_id,
            info.charac_no,
            info.ipg_no,
            info.check_type);
        return -1;
    }

    if (db_handle->fetch() != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::ITEM_QUERY::arad_sp_buy_item_check(ARAD_DATAS::BuyItemInfo&)",
            0x47b,
            "[ERROR] DB Fetch Error => MID[%d], CHARIDX[%d], ITEMIDX[%d], CHECKTYPE[%d]",
            info.member_id,
            info.charac_no,
            info.ipg_no,
            info.check_type);
        return -1;
    }

    int result_count = 0;
    if (db_handle->get_int(0, result_count) != true) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_DB_Query.cpp",
            "int ARAD::DATABASE::ITEM_QUERY::arad_sp_buy_item_check(ARAD_DATAS::BuyItemInfo&)",
            0x485,
            "RESULT_COUNT MID[%d], CHARIDX[%d], ITEMIDX[%d], CHECKTYPE[%d]",
            info.member_id,
            info.charac_no,
            info.ipg_no,
            info.check_type);
        return -1;
    }

    info.result = 1;
    if (static_cast<int16_t>(info.maximum_count_wire) <= result_count) {
        info.result = 0;
    }
    return 0;
}

}  // namespace ITEM_QUERY
}  // namespace DATABASE
}  // namespace ARAD
