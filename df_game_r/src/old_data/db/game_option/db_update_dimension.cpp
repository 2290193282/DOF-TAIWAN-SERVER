#include "old_data/db/game_option/db_update_dimension.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



namespace {

const unsigned char kLimitedInCountShifts[3] = {0u, 2u, 4u};
const unsigned char kLimitedInCountMasks[3] = {3u, 3u, 1u};

bool IsValidLimitedInCount(unsigned char packed) {
    std::vector<dungeon_runtime::InCountRecord> records;
    if (!dungeon_runtime::GetLimitedInCountRecords(&records) ||
        records.size() > 3u) {
        return false;
    }
    unsigned char used_mask = 0u;
    for (std::size_t index = 0; index < records.size(); ++index) {
        const int maximum = records[index].maximum_count;
        const unsigned char value_mask = kLimitedInCountMasks[index];
        if (maximum <= 0 || maximum > static_cast<int>(value_mask)) {
            return false;
        }
        used_mask = static_cast<unsigned char>(
            used_mask | static_cast<unsigned char>(
                            value_mask << kLimitedInCountShifts[index]));
        const unsigned char remaining = static_cast<unsigned char>(
            (packed >> kLimitedInCountShifts[index]) & value_mask);
        if (remaining > static_cast<unsigned char>(maximum)) {
            return false;
        }
    }
    return (packed & static_cast<unsigned char>(~used_mask)) == 0u;
}

#pragma pack(push, 1)
struct SIG_UPDATE_DIMENSION {
    uint32_t charac_no;       // +0x00
    char dimension[6];        // +0x04
    char tournament_inout;    // +0x0A, charac_dimension_inout.dungeon7
    uint8_t limited_inout;    // +0x0B, packed 3054/3056/3057
    char blood;               // +0x0C
    char save_dimension;      // +0x0D
    char save_blood;          // +0x0E
    uint8_t save_limited;     // +0x0F
};
#pragma pack(pop)

typedef char SIG_UPDATE_DIMENSION_Size_Must_Be_0x10[
    (sizeof(SIG_UPDATE_DIMENSION) == 0x10) ? 1 : -1];

}  // namespace

// IDA_EA: 0x844EE1E
// IDA_NAME: _ZN18DB_UpdateDimensionC2Ev
// DEMANGLED: DB_UpdateDimension::DB_UpdateDimension(void)
DB_UpdateDimension::DB_UpdateDimension() {}

// IDA_EA: 0x845D918
// IDA_NAME: _ZN18DB_UpdateDimensionD2Ev
// DEMANGLED: DB_UpdateDimension::~DB_UpdateDimension()
// IDA_EA: 0x845D948
// IDA_NAME: _ZN18DB_UpdateDimensionD0Ev
// DEMANGLED: DB_UpdateDimension::~DB_UpdateDimension() deleting destructor
DB_UpdateDimension::~DB_UpdateDimension() {}

// IDA_EA: 0x843230A
// IDA_NAME: _ZN18DB_UpdateDimension8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateDimension::dispatch(int,int,Stream *)
bool DB_UpdateDimension::dispatch(int signal,
                                  int user_idx,
                                  Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream->enable_out(sizeof(SIG_UPDATE_DIMENSION))) {
        return false;
    }
    SIG_UPDATE_DIMENSION* payload =
        reinterpret_cast<SIG_UPDATE_DIMENSION*>(stream->get_buffer());
    stream->out_ptr(sizeof(SIG_UPDATE_DIMENSION));

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (payload->save_dimension) {
        db_handle->set_query(
            "upDate charac_dimension_inout set dungeon1=%d, dungeon2=%d, dungeon3=%d,  dungeon4=%d, dungeon5=%d, dungeon6=%d where charac_no=%u",
            payload->dimension[0],
            payload->dimension[1],
            payload->dimension[2],
            payload->dimension[3],
            payload->dimension[4],
            payload->dimension[5],
            payload->charac_no);
        if (db_handle->exec(true) != true ||
            !db_handle->getAffectedRowCount()) {
            db_handle->set_query(
                "inSert into charac_dimension_inout(charac_no,dungeon1,dungeon2,dungeon3,dungeon4,dungeon5,dungeon6) values(%u,%d,%d,%d,%d,%d,%d)",
                payload->charac_no,
                payload->dimension[0],
                payload->dimension[1],
                payload->dimension[2],
                payload->dimension[3],
                payload->dimension[4],
                payload->dimension[5]);
            if (db_handle->exec(true) != true) {
                LogManager::logFormat(
                    1,
                    "DBThread.cpp",
                    "virtual bool DB_UpdateDimension::dispatch(int, int, Stream*)",
                    0x7972,
                    "DIMENSION_LOG : INOUT CNT INSERT DB ERROR!! values(characno:%u,dungeon1:%d,2:%d,3:%d,4:%d,5:%d,6:%d)",
                    payload->charac_no,
                    payload->dimension[0],
                    payload->dimension[1],
                    payload->dimension[2],
                    payload->dimension[3],
                    payload->dimension[4],
                    payload->dimension[5]);
            }
        }
    }

    if (payload->save_blood) {
        db_handle->set_query(
            "inSert into charac_dimension_inout(charac_no,dungeon1,dungeon2,dungeon3,dungeon4,dungeon5,dungeon6,dungeon7) values(%u,%d,%d,%d,%d,%d,%d,%d) on duplicate key upDate dungeon7=values(dungeon7)",
            payload->charac_no,
            payload->dimension[0],
            payload->dimension[1],
            payload->dimension[2],
            payload->dimension[3],
            payload->dimension[4],
            payload->dimension[5],
            payload->tournament_inout);
        if (db_handle->exec(true) != true) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "virtual bool DB_UpdateDimension::dispatch(int, int, Stream*)",
                0x797E,
                "TOURNAMENT_LOG : INOUT CNT UPSERT DB ERROR!! values(charac_no:%u,dungeon7:%d)",
                payload->charac_no,
                payload->tournament_inout);
        }

        db_handle->set_query(
            "upDate charac_blood_inout set dungeon1=%d where charac_no=%u",
            payload->blood,
            payload->charac_no);
        if (db_handle->exec(true) != true ||
            !db_handle->getAffectedRowCount()) {
            db_handle->set_query(
                "inSert into charac_blood_inout(charac_no,dungeon1) values(%u,%d)",
                payload->charac_no,
                payload->blood);
            if (db_handle->exec(true) != true) {
                LogManager::logFormat(
                    1,
                    "DBThread.cpp",
                    "virtual bool DB_UpdateDimension::dispatch(int, int, Stream*)",
                    0x797E,
                    "BLOOD_LOG : INOUT CNT INSERT DB ERROR!! values(charac_no:%u,dungeon1:%d)",
                    payload->charac_no,
                    payload->blood);
            }
        }
    }
    if (payload->save_limited) {
        if (!IsValidLimitedInCount(payload->limited_inout)) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "virtual bool DB_UpdateDimension::dispatch(int, int, Stream*)",
                0x797E,
                "LIMITED_INCOUNT_LOG : INVALID PACKED VALUE!! charac_no:%u,value:%u",
                payload->charac_no,
                static_cast<unsigned int>(payload->limited_inout));
            return false;
        }
        db_handle->set_query(
            "inSert into charac_dimension_inout(charac_no,dungeon1,dungeon2,dungeon3,dungeon4,dungeon5,dungeon6,dungeon7,dungeon8) values(%u,%d,%d,%d,%d,%d,%d,%d,%u) on duplicate key upDate dungeon8=values(dungeon8)",
            payload->charac_no,
            payload->dimension[0],
            payload->dimension[1],
            payload->dimension[2],
            payload->dimension[3],
            payload->dimension[4],
            payload->dimension[5],
            payload->tournament_inout,
            static_cast<unsigned int>(payload->limited_inout));
        if (db_handle->exec(true) != true) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "virtual bool DB_UpdateDimension::dispatch(int, int, Stream*)",
                0x797E,
                "LIMITED_INCOUNT_LOG : UPSERT DB ERROR!! charac_no:%u,dungeon8:%u",
                payload->charac_no,
                static_cast<unsigned int>(payload->limited_inout));
        }
    }
    return true;
}
