#include "old_data/db/quest/db_update_quest.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>
#include <string.h>

extern int compress_zip(char* dest, unsigned int& dest_len,
                        const char* source, unsigned int source_len);



// IDA_EA: 0x8418B74
// IDA_NAME: _ZN14DB_UpdateQuestC2Ev
// DEMANGLED: DB_UpdateQuest::DB_UpdateQuest(void)
DB_UpdateQuest::DB_UpdateQuest() {}

// IDA_EA: 0x8418B90
// IDA_NAME: _ZN14DB_UpdateQuestD2Ev
// DEMANGLED: DB_UpdateQuest::~DB_UpdateQuest()
// IDA_EA: 0x8418BC0
// IDA_NAME: _ZN14DB_UpdateQuestD0Ev
// DEMANGLED: DB_UpdateQuest::~DB_UpdateQuest() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateQuest::~DB_UpdateQuest() {}

// IDA_EA: 0x8418BDE
// IDA_NAME: _ZN14DB_UpdateQuest8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateQuest::dispatch(int,int,Stream *)
bool DB_UpdateQuest::dispatch(int signal, int user_idx, Stream* stream) {
    SIG_UPDATE_QUEST* payload = stream->GetOutBuffer<SIG_UPDATE_QUEST>();
    if (payload) {
        _updateQuests(payload);
    }
    return false;
}

// IDA_EA: 0x8418C1C
// IDA_NAME: _ZN14DB_UpdateQuest13_updateQuestsEP16SIG_UPDATE_QUEST
// DEMANGLED: DB_UpdateQuest::_updateQuests(SIG_UPDATE_QUEST *)
bool DB_UpdateQuest::_updateQuests(SIG_UPDATE_QUEST* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    char clear_zip[36012];
    memset(clear_zip, 0, sizeof(clear_zip));
    unsigned int clear_zip_len = sizeof(clear_zip);
    if (compress_zip(clear_zip,
                     clear_zip_len,
                     payload_data->clear_quest,
                     sizeof(payload_data->clear_quest)) != 1) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_UpdateQuest::_updateQuests(SIG_UPDATE_QUEST*)",
            0x4A5E,
            "3RD_EXPEND : DB_UpdateQuest::DB ERROR!! charac_no(%u)",
            payload_data->charac_no);
        return false;
    }

    char* notify_blob = db_handle->blob_to_str(
        1,
        payload_data->quest_notify,
        sizeof(payload_data->quest_notify));
    if (payload_data->update_clear_quest) {
        char* clear_blob = db_handle->blob_to_str(
            0, clear_zip, clear_zip_len);
        db_handle->set_query(
            "upDate new_charac_quest set clear_quest='%s', play_1=%d, play_1_trigger=%d, play_2=%d, play_2_trigger=%d, play_3=%d, play_3_trigger=%d, play_4=%d, play_4_trigger=%d, play_5=%d, play_5_trigger=%d,play_6=%d, play_6_trigger=%d, play_7=%d, play_7_trigger=%d, play_8=%d, play_8_trigger=%d, play_9=%d, play_9_trigger=%d, play_10=%d, play_10_trigger=%d, play_11=%d, play_11_trigger=%d, play_12=%d, play_12_trigger=%d, play_13=%d, play_13_trigger=%d, play_14=%d, play_14_trigger=%d, play_15=%d, play_15_trigger=%d,play_16=%d, play_16_trigger=%d, play_17=%d, play_17_trigger=%d, play_18=%d, play_18_trigger=%d, play_19=%d, play_19_trigger=%d, play_20=%d, play_20_trigger=%d,urgentQuestIndex=%d,quest_notify='%s' where charac_no=%u",
            clear_blob,
            payload_data->play[0], payload_data->play_trigger[0],
            payload_data->play[1], payload_data->play_trigger[1],
            payload_data->play[2], payload_data->play_trigger[2],
            payload_data->play[3], payload_data->play_trigger[3],
            payload_data->play[4], payload_data->play_trigger[4],
            payload_data->play[5], payload_data->play_trigger[5],
            payload_data->play[6], payload_data->play_trigger[6],
            payload_data->play[7], payload_data->play_trigger[7],
            payload_data->play[8], payload_data->play_trigger[8],
            payload_data->play[9], payload_data->play_trigger[9],
            payload_data->play[10], payload_data->play_trigger[10],
            payload_data->play[11], payload_data->play_trigger[11],
            payload_data->play[12], payload_data->play_trigger[12],
            payload_data->play[13], payload_data->play_trigger[13],
            payload_data->play[14], payload_data->play_trigger[14],
            payload_data->play[15], payload_data->play_trigger[15],
            payload_data->play[16], payload_data->play_trigger[16],
            payload_data->play[17], payload_data->play_trigger[17],
            payload_data->play[18], payload_data->play_trigger[18],
            payload_data->play[19], payload_data->play_trigger[19],
            payload_data->urgent_quest_index,
            notify_blob,
            payload_data->charac_no);
    } else {
        db_handle->set_query(
            "upDate new_charac_quest set play_1=%d, play_1_trigger=%d, play_2=%d, play_2_trigger=%d, play_3=%d, play_3_trigger=%d, play_4=%d, play_4_trigger=%d, play_5=%d, play_5_trigger=%d,play_6=%d, play_6_trigger=%d, play_7=%d, play_7_trigger=%d, play_8=%d, play_8_trigger=%d, play_9=%d, play_9_trigger=%d, play_10=%d, play_10_trigger=%d, play_11=%d, play_11_trigger=%d, play_12=%d, play_12_trigger=%d, play_13=%d, play_13_trigger=%d, play_14=%d, play_14_trigger=%d, play_15=%d, play_15_trigger=%d,play_16=%d, play_16_trigger=%d, play_17=%d, play_17_trigger=%d, play_18=%d, play_18_trigger=%d, play_19=%d, play_19_trigger=%d, play_20=%d, play_20_trigger=%d,urgentQuestIndex=%d,quest_notify='%s' where charac_no=%u",
            payload_data->play[0], payload_data->play_trigger[0],
            payload_data->play[1], payload_data->play_trigger[1],
            payload_data->play[2], payload_data->play_trigger[2],
            payload_data->play[3], payload_data->play_trigger[3],
            payload_data->play[4], payload_data->play_trigger[4],
            payload_data->play[5], payload_data->play_trigger[5],
            payload_data->play[6], payload_data->play_trigger[6],
            payload_data->play[7], payload_data->play_trigger[7],
            payload_data->play[8], payload_data->play_trigger[8],
            payload_data->play[9], payload_data->play_trigger[9],
            payload_data->play[10], payload_data->play_trigger[10],
            payload_data->play[11], payload_data->play_trigger[11],
            payload_data->play[12], payload_data->play_trigger[12],
            payload_data->play[13], payload_data->play_trigger[13],
            payload_data->play[14], payload_data->play_trigger[14],
            payload_data->play[15], payload_data->play_trigger[15],
            payload_data->play[16], payload_data->play_trigger[16],
            payload_data->play[17], payload_data->play_trigger[17],
            payload_data->play[18], payload_data->play_trigger[18],
            payload_data->play[19], payload_data->play_trigger[19],
            payload_data->urgent_quest_index,
            notify_blob,
            payload_data->charac_no);
    }

    if (db_handle->exec(true) != true) {
        return false;
    }
    return db_handle->getAffectedRowCount() != 0;
}
