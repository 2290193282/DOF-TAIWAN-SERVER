#include "old_data/db/inventory/db_save_achievement.h"

#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/inventory/achievement_payload.h"
#include "infra/network/packet/stream.h"

#include <cstring>

namespace {

const unsigned int kAchievementCompressedBytes = 7399u;

}  // namespace

// IDA_EA: 0x844F84A
// IDA_NAME: _ZN18DB_SaveAchievementC2Ev
// DEMANGLED: DB_SaveAchievement::DB_SaveAchievement(void)
DB_SaveAchievement::DB_SaveAchievement() : DBDispatcher() {}

// IDA_EA: 0x845FA9C
// IDA_NAME: _ZN18DB_SaveAchievementD2Ev
// DEMANGLED: DB_SaveAchievement::~DB_SaveAchievement()
// IDA_EA: 0x845FACC
// IDA_NAME: _ZN18DB_SaveAchievementD0Ev
// DEMANGLED: DB_SaveAchievement::~DB_SaveAchievement()
DB_SaveAchievement::~DB_SaveAchievement() {}

// IDA_EA: 0x840DEAA
// IDA_NAME: _ZN18DB_SaveAchievement8dispatchEiiP6Stream
// DEMANGLED: DB_SaveAchievement::dispatch(int,int,Stream *)
bool DB_SaveAchievement::dispatch(int signal,
                                  int user_idx,
                                  Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    stAchievement_DBSaveData* const payload =
        stream->GetOutBuffer<stAchievement_DBSaveData>();

    char compressed[kAchievementCompressedBytes];
    std::memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_len,
                     reinterpret_cast<const char*>(payload->achievements),
                     sizeof(payload->achievements)) != 1) {
        return false;
    }

    char* achievement_blob =
        db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "upDate charac_achievement set achievement='%s', last_update_time=now() where charac_no=%u",
        achievement_blob,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    achievement_blob =
        db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "inSert into charac_achievement(charac_no, achievement, last_update_time) values(%u, '%s', now())",
        payload->charac_no,
        achievement_blob);
    return db_handle->exec(true) == true;
}
