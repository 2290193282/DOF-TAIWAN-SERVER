#include "old_data/db/dungeon/db_save_pvp_mission_list.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/pvp_mission_stream.h"

#include <stdint.h>
#include <string.h>

extern int compress_zip(char* dest, unsigned int& dest_len,
                        const char* source, unsigned int source_len);


namespace {

enum {
    kCompressedPvPMissionListSize = 408,
};

}  // namespace

// IDA_EA: 0x844F716
// IDA_NAME: _ZN21DB_SavePvPMissionListC2Ev
// DEMANGLED: DB_SavePvPMissionList::DB_SavePvPMissionList(void)
DB_SavePvPMissionList::DB_SavePvPMissionList() {}

// IDA_EA: 0x845BDFA
// IDA_NAME: _ZN21DB_SavePvPMissionListD2Ev
// DEMANGLED: DB_SavePvPMissionList::~DB_SavePvPMissionList()
// IDA_EA: 0x845BE2A
// IDA_NAME: _ZN21DB_SavePvPMissionListD0Ev
// DEMANGLED: DB_SavePvPMissionList::~DB_SavePvPMissionList() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SavePvPMissionList::~DB_SavePvPMissionList() {}

// IDA_EA: 0x843F8FA
// IDA_NAME: _ZN21DB_SavePvPMissionList8dispatchEiiP6Stream
// DEMANGLED: DB_SavePvPMissionList::dispatch(int,int,Stream *)
bool DB_SavePvPMissionList::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    SIG_REQUEST_PVP_MISSION_LIST* const payload =
        stream->GetOutBuffer<SIG_REQUEST_PVP_MISSION_LIST>();

    char compressed[kCompressedPvPMissionListSize];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_len,
                     reinterpret_cast<const char*>(payload),
                     0x14Au) != 1) {
        return false;
    }

    char* mission_blob =
        db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "upDate fair_pvp_score set pvp_mission_info='%s' where charac_no=%u",
        mission_blob,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    mission_blob = db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "inSert into fair_pvp_score(charac_no, pvp_mission_info) values(%u, '%s')",
        payload->charac_no,
        mission_blob);
    return db_handle->exec(true) == true;
}
