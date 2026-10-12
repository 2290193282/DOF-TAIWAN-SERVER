#include "old_data/db/quest/db_load_quest.h"

#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>

extern int uncompress_zip(char* dest, unsigned int& dest_len,
                          const char* source, unsigned int source_len);



namespace {

const char kLoadQuestQuery[] =
    "seLect charac_no, clear_quest,play_1, play_1_trigger, play_2, play_2_trigger, play_3, play_3_trigger, play_4, play_4_trigger, play_5, play_5_trigger,play_6, play_6_trigger, play_7, play_7_trigger, play_8, play_8_trigger, play_9, play_9_trigger, play_10, play_10_trigger,play_11, play_11_trigger, play_12, play_12_trigger, play_13, play_13_trigger, play_14, play_14_trigger, play_15, play_15_trigger,play_16, play_16_trigger, play_17, play_17_trigger, play_18, play_18_trigger, play_19, play_19_trigger, play_20, play_20_trigger,urgentQuestIndex,quest_notify from new_charac_quest  where charac_no=%u";

}  // namespace

// IDA_EA: 0x844D930
// IDA_NAME: _ZN12DB_LoadQuestC2Ev
// DEMANGLED: DB_LoadQuest::DB_LoadQuest(void)
DB_LoadQuest::DB_LoadQuest() {}

// IDA_EA: 0x844D94C
// IDA_NAME: _ZN12DB_LoadQuestD2Ev
// DEMANGLED: DB_LoadQuest::~DB_LoadQuest()
// IDA_EA: 0x844D97C
// IDA_NAME: _ZN12DB_LoadQuestD0Ev
// DEMANGLED: DB_LoadQuest::~DB_LoadQuest() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadQuest::~DB_LoadQuest() {}

// IDA_EA: 0x840E236
// IDA_NAME: _ZN12DB_LoadQuest8dispatchEiiP6Stream
// DEMANGLED: DB_LoadQuest::dispatch(int,int,Stream *)
bool DB_LoadQuest::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    Stream* out_stream = GlobalData::big_stream_pool()->Acquire(
        "DBThread.cpp", 10745);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_LOAD_QUEST* payload = guard.GetInBuffer<SIG_LOAD_QUEST>();
    if (!payload) {
        return false;
    }
    memset(payload, 0, sizeof(*payload));
    payload->error_code = 0;

    SIG_LOAD_QUEST* request = stream->GetOutBuffer<SIG_LOAD_QUEST>();
    if (!request) {
        return false;
    }
    payload->charac_no = request->charac_no;

    if (GetUserQuest(payload) != true) {
        payload->error_code = 1;
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x840E39A
// IDA_NAME: _ZN12DB_LoadQuest12GetUserQuestEP14SIG_LOAD_QUEST
// DEMANGLED: DB_LoadQuest::GetUserQuest(SIG_LOAD_QUEST *)
bool DB_LoadQuest::GetUserQuest(SIG_LOAD_QUEST* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(kLoadQuestQuery, payload_data->charac_no);

    char clear_zip[36012];
    memset(clear_zip, 0, sizeof(clear_zip));
    unsigned int clear_raw_len = 36012;
    int column = 0;
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        cMyTrace trace("bool DB_LoadQuest::GetUserQuest(SIG_LOAD_QUEST*)",
                       10803,
                       5);
        trace("QUEST_EXPEND_LOG : NO QUEST DATA EXIST!! : CHARAC_NO(%u)",
              payload_data->charac_no);
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }

    unsigned int row_charac_no = 0;
    if (db_handle->get_int(column++, row_charac_no) != true) {
        return false;
    }

    const unsigned int compressed_len =
        static_cast<unsigned int>(db_handle->get_binary_length(column));
    if (db_handle->get_binary(column++, clear_zip, sizeof(clear_zip)) != true) {
        return false;
    }
    char clear_quest[sizeof(payload_data->clear_quest)];
    memset(clear_quest, 0, sizeof(clear_quest));
    if (uncompress_zip(clear_quest,
                       clear_raw_len,
                       clear_zip,
                       compressed_len) != 1) {
        return false;
    }
    memcpy(payload_data->clear_quest, clear_quest,
           sizeof(payload_data->clear_quest));

    for (int index = 0; index < 20; ++index) {
        int play = 0;
        int trigger = 0;
        if (db_handle->get_int(column++, play) != true) {
            return false;
        }
        if (db_handle->get_int(column++, trigger) != true) {
            return false;
        }
        payload_data->play_quest[index] = static_cast<uint32_t>(play);
        payload_data->play_quest_trigger[index] =
            static_cast<uint32_t>(trigger);
    }

    int urgent_index = 0;
    if (db_handle->get_int(column++, urgent_index) != true) {
        return false;
    }
    payload_data->urgent_quest_index = static_cast<uint32_t>(urgent_index);
    if (db_handle->get_binary(column++, payload_data->quest_notify,
                              sizeof(payload_data->quest_notify)) != true) {
        return false;
    }
    return true;
}
