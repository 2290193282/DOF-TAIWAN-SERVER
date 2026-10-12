#include "old_data/db/event/db_event_item_and_levelup_dispatchers.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_game/activity/schedule/event_login_item_runtime.h"
#include "old_game/activity/schedule/event_stage_log_stream.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>

namespace {

const unsigned int kEventLevelupReplyBytes = 44;

void WriteU16(unsigned char* bytes, unsigned int offset, uint16_t value) {
    memcpy(bytes + offset, &value, sizeof(value));
}

}  // namespace

// IDA_EA: 0x844DAD8 / 0x844DAF4
DB_SaveEventItem::DB_SaveEventItem() {}
DB_SaveEventItem::~DB_SaveEventItem() {}

// IDA_EA: 0x841A51A
// IDA_NAME: _ZN16DB_SaveEventItem25_CheckAlreadyGetYegamItemEii
// DEMANGLED: DB_SaveEventItem::_CheckAlreadyGetYegamItem(int,int)
bool DB_SaveEventItem::_CheckAlreadyGetYegamItem(int entry_id, int item_id) {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_1, SERVER_GROUP_DEFAULT);
    if (item_id == 1) {
        db_handle->set_query(
            "seLect coin_check from event_yegam_info where id=%d", entry_id);
    } else {
        db_handle->set_query(
            "seLect item_check from event_yegam_info where id=%d", entry_id);
    }
    if (!db_handle->exec(true)) {
        return false;
    }
    int check_time = 0;
    return db_handle->get_int(0, check_time) && check_time != 0;
}

// IDA_EA: 0x841A5EA
// IDA_NAME: _ZN16DB_SaveEventItem21_UpdateEventYegamInfoEii
// DEMANGLED: DB_SaveEventItem::_UpdateEventYegamInfo(int,int)
int DB_SaveEventItem::_UpdateEventYegamInfo(int entry_id, int item_id) {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_1, SERVER_GROUP_DEFAULT);
    if (item_id == 1) {
        db_handle->set_query(
            "upDate event_yegam_info set coin_check=unix_timestamp(now()) "
            "where id=%d",
            entry_id);
    } else {
        db_handle->set_query(
            "upDate event_yegam_info set item_check=unix_timestamp(now()) "
            "where id=%d and item!=0",
            entry_id);
    }
    return db_handle->exec(true);
}

// IDA_EA: 0x841A662
// IDA_NAME: _ZN16DB_SaveEventItem19_HardCodeEventItemsEP18SIG_SAVE_EVENTITEM
// DEMANGLED: DB_SaveEventItem::_HardCodeEventItems(SIG_SAVE_EVENTITEM *)
int DB_SaveEventItem::_HardCodeEventItems(SIG_SAVE_EVENTITEM* request) {
    if (request->item_id != 26030u) {
        return 0;
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    db_handle->set_query(
        "seLect id from charac_event_items where charac_no=%d and it_id=%d",
        static_cast<int>(request->charac_no),
        static_cast<int>(request->item_id));
    int event_item_id = 0;
    if (!db_handle->exec(true) || db_handle->get_n_rows() == 0 ||
        !db_handle->fetch() || !db_handle->get_int(0, event_item_id) ||
        event_item_id == 0) {
        db_handle->set_query(
            "inSert into charac_event_items(charac_no,it_id,stack_count,"
            "event_code,reg_time,delete_time,delete_flag) values(%d,%d,%d,%d,"
            "now(),now(),1)",
            static_cast<int>(request->charac_no),
            static_cast<int>(request->item_id),
            static_cast<int>(request->stack_count),
            static_cast<int>(request->event_code));
        if (!db_handle->exec(true)) {
            return 1;
        }
    } else {
        db_handle->set_query(
            "upDate charac_event_items set stack_count=stack_count+1 where "
            "id=%d",
            event_item_id);
        if (!db_handle->exec(true)) {
            return 1;
        }
    }
    return 1;
}

// IDA_EA: 0x841A7E4
bool DB_SaveEventItem::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    // Raw 0x841A7E4 acquires the character DB before decoding the stream.
    // Keep that acquisition even when a later per-event branch uses DB1.
    (void)GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_3,
                                             SERVER_GROUP_DEFAULT);
    SIG_SAVE_EVENTITEM request;
    if (!stream || stream->get_binary(&request, sizeof(request)) != 1) {
        return false;
    }

    if (request.event_source != 0) {
        int updated = 0;
        switch (request.event_code) {
        case 1:
            if (_CheckAlreadyGetYegamItem(
                    static_cast<int>(request.event_source),
                    static_cast<int>(request.item_id))) {
                return false;
            }
            updated = _UpdateEventYegamInfo(
                static_cast<int>(request.event_source),
                static_cast<int>(request.item_id));
            break;
        case 2:
            if (WongWork::EventATI::CheckAlreadyGetATIItem(
                    request.event_source, static_cast<int>(request.item_id))) {
                return false;
            }
            updated = WongWork::EventATI::UpdateEventATIInfo(
                request.event_source, static_cast<int>(request.item_id));
            break;
        case 3:
            if (WongWork::EventSigong::CheckAlreadyGetSigongItem(
                    request.event_source, static_cast<int>(request.item_id))) {
                return false;
            }
            updated = WongWork::EventSigong::UpdateEventSigongInfo(
                request.event_source, static_cast<int>(request.item_id));
            break;
        case 4:
            if (WongWork::EventEverland::CheckAlreadyGetEverlandItem(
                    request.event_source, static_cast<int>(request.item_id))) {
                return false;
            }
            updated = WongWork::EventEverland::UpdateEventEverlandInfo(
                request.event_source, static_cast<int>(request.item_id));
            break;
        case 5:
            if (WongWork::EventMuz::CheckAlreadyGetMuzItem(
                    request.event_source, static_cast<int>(request.item_id))) {
                return false;
            }
            updated = WongWork::EventMuz::UpdateEventMuzInfo(
                request.event_source, static_cast<int>(request.item_id));
            break;
        case 6:
            if (WongWork::EventAlttle::CheckAlreadyGetAlttleItem(
                    request.event_context, static_cast<int>(request.item_id))) {
                return false;
            }
            updated = WongWork::EventAlttle::UpdateEventAlttleInfo(
                request.event_context, static_cast<int>(request.item_id));
            break;
        case 7:
            if (WongWork::EventFCCombo::CheckAlreadyFCComboEventItem(
                    request.event_source, static_cast<int>(request.item_id))) {
                return false;
            }
            updated = WongWork::EventFCCombo::UpdateFCComboEventInfo(
                request.event_source, static_cast<int>(request.item_id));
            break;
        }
        if (updated != 1) {
            return false;
        }
    }

    if (_HardCodeEventItems(&request) != 0) {
        return true;
    }
    if (request.no_persist != 0) {
        return true;
    }

    const unsigned int inserted = WongWork::EventCommon::InsertEventItem2EventTable(
        request.charac_no,
        request.item_id,
        request.stack_count,
        request.event_code);
    if (inserted != 0) {
        WongWork::EventCommon::LoadCharacEventItem(request.event_context,
                                                    request.charac_no,
                                                    inserted);
    }
    return true;
}

// IDA_EA: 0x844DB42 / 0x844DB5E
DB_RemoveEventItem::DB_RemoveEventItem() {}
DB_RemoveEventItem::~DB_RemoveEventItem() {}

// IDA_EA: 0x841AB30
bool DB_RemoveEventItem::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    uint32_t count = 0;
    if (!stream || stream->get_binary(&count, sizeof(count)) != 1 || count > 64) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    for (uint32_t index = 0; index < count; ++index) {
        uint32_t item_row_id = 0;
        if (stream->get_binary(&item_row_id, sizeof(item_row_id)) != 1) {
            return false;
        }
        db_handle->set_query(
            "upDate charac_event_items set delete_flag=1,delete_time=now() "
            "where id=%u", item_row_id);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x844FF4A / 0x845AEBE
DB_InsertEventStageLog::DB_InsertEventStageLog() {}
DB_InsertEventStageLog::~DB_InsertEventStageLog() {}

// IDA_EA: 0x8449144
bool DB_InsertEventStageLog::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    SIG_EVENT_STAGE_LOG payload;
    if (!stream || stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    char result[sizeof(payload.stage_result) + 1];
    memset(result, 0, sizeof(result));
    memcpy(result, payload.stage_result, sizeof(payload.stage_result));
    db_handle->set_query(
        "inSert into dnf_event_accept_list(event_id,m_id,charac_no,server_info,"
        "occ_date,stage,stage_result) values(%u,%u,%u,%u,now(),%u,'%s')",
        static_cast<unsigned int>(payload.event_id),
        payload.account_id,
        static_cast<unsigned int>(payload.charac_no),
        static_cast<unsigned int>(payload.server_info),
        static_cast<unsigned int>(payload.stage),
        result);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x844920E
// IDA_NAME: _ZN22DB_InsertEventStageLog11makeRequestER19SIG_EVENT_STAGE_LOG
// DEMANGLED: DB_InsertEventStageLog::makeRequest(SIG_EVENT_STAGE_LOG &)
void DB_InsertEventStageLog::makeRequest(SIG_EVENT_STAGE_LOG& payload) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 47071);
    CStreamGuard guard(stream, true);
    guard << 819;
    guard << -1;
    SIG_EVENT_STAGE_LOG* const destination =
        guard->GetInBuffer<SIG_EVENT_STAGE_LOG>();
    memcpy(destination, &payload, sizeof(payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844FFD6 / 0x845A758
DB_LoadEventLevelupSupport::DB_LoadEventLevelupSupport() {}
DB_LoadEventLevelupSupport::~DB_LoadEventLevelupSupport() {}

// IDA_EA: 0x844CD38
bool DB_LoadEventLevelupSupport::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    int charac_no = 0;
    if (!stream || stream->get_binary(&charac_no, sizeof(charac_no)) != 1) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "seLect level, state from event_levelup_support where charac_no=%d",
        charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    unsigned char response[kEventLevelupReplyBytes];
    memset(response, 0, sizeof(response));
    const unsigned int rows = db_handle->get_n_rows() > 10
                                  ? 10u
                                  : static_cast<unsigned int>(db_handle->get_n_rows());
    memcpy(response, &rows, sizeof(rows));
    for (unsigned int row = 0; row < rows; ++row) {
        unsigned short level = 0;
        unsigned short state = 0;
        if (db_handle->fetch() != true || db_handle->get_ushort(0, level) != true ||
            db_handle->get_ushort(1, state) != true) {
            return false;
        }
        WriteU16(response, 4 + row * 4, level);
        WriteU16(response, 6 + row * 4, state);
    }

    Stream* out = GlobalData::stream_pool()->Acquire("DBThread.cpp", 49785);
    CStreamGuard guard(out, true);
    guard << 848;
    guard << user_idx;
    guard.put_binary(response, sizeof(response));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844D0A6
void DB_LoadEventLevelupSupport::makeRequest(int user_idx, int charac_no) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 49824);
    CStreamGuard guard(stream, true);
    guard << 848;
    guard << user_idx;
    guard << charac_no;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
