#include "old_data/db/character/db_update_character_day.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

namespace {

enum {
    kSigUpdateCharacterDayEventSize = 8,
};

#define DB_CHAR_DAY_U16(payload, offset) \
    (*reinterpret_cast<unsigned short*>((payload) + (offset)))
#define DB_CHAR_DAY_U32(payload, offset) \
    (*reinterpret_cast<unsigned int*>((payload) + (offset)))

}  // namespace

// IDA_EA: 0x844EF52
// IDA_NAME: _ZN21DB_UpdateCharacterDayC2Ev
// DEMANGLED: DB_UpdateCharacterDay::DB_UpdateCharacterDay(void)
DB_UpdateCharacterDay::DB_UpdateCharacterDay() {}

// IDA_EA: 0x845D4D4
// IDA_NAME: _ZN21DB_UpdateCharacterDayD2Ev
// DEMANGLED: DB_UpdateCharacterDay::~DB_UpdateCharacterDay()
// IDA_EA: 0x845D504
// IDA_NAME: _ZN21DB_UpdateCharacterDayD0Ev
// DEMANGLED: DB_UpdateCharacterDay::~DB_UpdateCharacterDay() deleting destructor
DB_UpdateCharacterDay::~DB_UpdateCharacterDay() {}

// IDA_EA: 0x8435D12
// IDA_NAME: _ZN21DB_UpdateCharacterDay8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCharacterDay::dispatch(int,int,Stream *)
bool DB_UpdateCharacterDay::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6), static_cast<ENUM_SERVER_GROUP>(0));
    if (!stream->enable_out(kSigUpdateCharacterDayEventSize)) {
        return false;
    }
    char* payload = stream->get_buffer();
    stream->out_ptr(kSigUpdateCharacterDayEventSize);
    db_handle->set_query(
        "upDate event_1111_charac_day set entries_count=entries_count+%u, update_time=now() where m_id=%s",
        DB_CHAR_DAY_U16(payload, 4),
        NumberToString(DB_CHAR_DAY_U32(payload, 0), 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into event_1111_charac_day(m_id, entries_count, update_time) values(%s, %u, now())",
            NumberToString(DB_CHAR_DAY_U32(payload, 0), 0),
            DB_CHAR_DAY_U16(payload, 4));
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

#undef DB_CHAR_DAY_U16
#undef DB_CHAR_DAY_U32
