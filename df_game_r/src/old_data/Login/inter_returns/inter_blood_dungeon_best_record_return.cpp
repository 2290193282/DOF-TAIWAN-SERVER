#include "old_data/Login/inter_returns/inter_blood_dungeon_best_record_return.h"

#include <stdint.h>

#include "old_data/db/dungeon/blood_best_record_payload.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8687FD8
// IDA_NAME: _ZN15CUserCharacInfo18setBloodBestRecordEjiib
int CUserCharacInfo::setBloodBestRecord(unsigned int play_time,
                                        int max_round,
                                        int dungeon_index,
                                        bool from_database) {
    _Charac_info* const current =
        const_cast<_Charac_info*>(getCurCharacR());
    if (!current) {
        return 0;
    }
    CUserBloodBestRecordMap& records = current->fields_.blood_best_records_;
    int32_t current_round = current->fields_.blood_best_round_record_;
    int32_t current_time = current->fields_.blood_best_time_record_;

    if (max_round > 15) {
        max_round = 15;
    }

    CUserBloodBestRecordMap::iterator found = records.find(dungeon_index);
    if (found == records.end()) {
        if (!from_database) {
            setBloodBestRecordUpdate(true);
        }
        stBloodBestRecord record;
        record.round_record = max_round;
        record.time_record = static_cast<int>(play_time);
        records.insert(std::make_pair(dungeon_index, record));
        current_round = max_round;
        current_time = static_cast<int>(play_time);
        current->fields_.blood_best_round_record_ = current_round;
        current->fields_.blood_best_time_record_ = current_time;
        return current_round;
    }

    current_round = found->second.round_record;
    current_time = found->second.time_record;
    if (current_round <= max_round &&
        (current_time != static_cast<int>(play_time) ||
         current_round != max_round)) {
        setBloodBestRecordUpdate(true);
        if (current_round == max_round) {
            if (static_cast<int>(play_time) < current_time) {
                current_time = static_cast<int>(play_time);
            }
        } else {
            current_time = static_cast<int>(play_time);
            current_round = max_round;
        }
        found->second.round_record = current_round;
        found->second.time_record = current_time;
    }
    current->fields_.blood_best_round_record_ = current_round;
    current->fields_.blood_best_time_record_ = current_time;
    return current_round;
}

// IDA_EA: 0x84D82F6
// IDA_NAME: _ZN32Inter_LoadBloodDungeonBestRecord12dispatch_sigEP5CUserPci
int Inter_LoadBloodDungeonBestRecord::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int) {
    if (!user->get_state()) {
        return 16093;
    }
    const SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD& record =
        *reinterpret_cast<const SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD*>(payload);
    const uint32_t charac_no = record.charac_no;
    const int charac_index = user->get_charac_idx(charac_no);
    if (charac_index < 0) {
        return 16099;
    }
    if (user->get_charac_no(charac_index) != static_cast<int>(charac_no)) {
        return 16101;
    }

    // The original stores the same confirmed number back into the selected
    // vector element before applying the DB record.  The named lookup above
    // proves that identity without recreating a raw vector write.
    user->setBloodBestRecord(record.best_time,
                             record.best_round,
                             record.dungeon_index,
                             true);
    return 0;
}

// IDA_EA: 0x84EDEFC / 0x84F84F6 / 0x84F8526
// IDA_NAME: _ZN32Inter_LoadBloodDungeonBestRecordC2Ev / D2Ev / D0Ev
Inter_LoadBloodDungeonBestRecord::Inter_LoadBloodDungeonBestRecord()
    : ISigDispatcher() {}
Inter_LoadBloodDungeonBestRecord::~Inter_LoadBloodDungeonBestRecord() {}
