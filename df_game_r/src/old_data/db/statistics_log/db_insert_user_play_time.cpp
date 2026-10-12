#include "old_data/db/statistics_log/db_insert_user_play_time.h"

#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>
#include <string.h>

// IDA_EA: 0x844E596
// IDA_NAME: _ZN21DB_InsertUserPlayTimeC2Ev
// DEMANGLED: DB_InsertUserPlayTime::DB_InsertUserPlayTime(void)
DB_InsertUserPlayTime::DB_InsertUserPlayTime() {}

// IDA_EA: 0x845ED34
// IDA_NAME: _ZN21DB_InsertUserPlayTimeD2Ev
// DEMANGLED: DB_InsertUserPlayTime::~DB_InsertUserPlayTime()
// IDA_EA: 0x845ED64
// IDA_NAME: _ZN21DB_InsertUserPlayTimeD0Ev
// DEMANGLED: DB_InsertUserPlayTime::~DB_InsertUserPlayTime() deleting destructor
DB_InsertUserPlayTime::~DB_InsertUserPlayTime() {}

// IDA_EA: 0x84256B4
// IDA_NAME: _ZN21DB_InsertUserPlayTime8dispatchEiiP6Stream
// DEMANGLED: DB_InsertUserPlayTime::dispatch(int,int,Stream *)
bool DB_InsertUserPlayTime::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(DB_HANDLE_LOG_REF),
        static_cast<ENUM_SERVER_GROUP>(SERVER_GROUP_DEFAULT));

    uint32_t m_id = 0;
    int play_time = 0;
    int trade_cnt = 0;
    uint32_t exp = 0;
    uint16_t used_fatigue = 0;
    uint8_t ting = 0;
    *stream >> m_id;
    *stream >> play_time;
    *stream >> trade_cnt;
    *stream >> exp;
    *stream >> used_fatigue;
    *stream >> ting;

    char table_name[20];
    memset(table_name, 0, sizeof(table_name));
    memcpy(table_name, "member_play_info", 17);

    db_handle->set_query(
        "upDate %s set play_time=play_time+%d,play_count=play_count+1,trade_cnt=trade_cnt+%d, exp=exp+%u, used_fatigue=used_fatigue+%u, ting_count=ting_count+%d where occ_date=cast(now() as date) and m_id=%s",
        table_name,
        play_time,
        trade_cnt,
        exp,
        static_cast<unsigned int>(used_fatigue),
        ting == 0,
        NumberToString(m_id, 0));

    bool should_insert = true;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount()) {
            should_insert = false;
        }
    }

    if (should_insert) {
        db_handle->set_query(
            "inSert into %s(occ_date,m_id,play_time,play_count,trade_cnt,exp,used_fatigue,ting_count) values(cast(now() as date),%s,%d,1,%d,%u,%u,%d)",
            table_name,
            NumberToString(m_id, 0),
            play_time,
            trade_cnt,
            exp,
            static_cast<unsigned int>(used_fatigue),
            ting == 0);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
