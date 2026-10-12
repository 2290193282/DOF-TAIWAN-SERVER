#include "old_data/db/dungeon/db_load_dimension.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/data_manager.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>


namespace {

const unsigned char kLimitedInCountShifts[3] = {0u, 2u, 4u};
const unsigned char kLimitedInCountMasks[3] = {3u, 3u, 1u};

bool PackLimitedInCountMaximums(unsigned char* packed) {
    if (!packed) {
        return false;
    }
    std::vector<dungeon_runtime::InCountRecord> records;
    if (!dungeon_runtime::GetLimitedInCountRecords(&records) ||
        records.size() > 3u) {
        return false;
    }
    unsigned char value = 0u;
    for (std::size_t index = 0; index < records.size(); ++index) {
        const int maximum = records[index].maximum_count;
        if (maximum <= 0 ||
            maximum > static_cast<int>(kLimitedInCountMasks[index])) {
            return false;
        }
        value = static_cast<unsigned char>(
            value | static_cast<unsigned char>(
                        maximum << kLimitedInCountShifts[index]));
    }
    *packed = value;
    return true;
}

}  // namespace


#pragma pack(push, 1)
struct SIG_LOAD_DIMENSION {
    uint32_t charac_no;       // +0x00
    char dimension[6];        // +0x04
    char tournament_inout;    // +0x0A, charac_dimension_inout.dungeon7
    uint8_t limited_inout;    // +0x0B, packed 3054/3056/3057
    char blood;               // +0x0C
    char reserved_0d[3];      // +0x0D
};
#pragma pack(pop)

typedef char SIG_LOAD_DIMENSION_Size_Must_Be_0x10[
    (sizeof(SIG_LOAD_DIMENSION) == 0x10) ? 1 : -1];

// IDA_EA: 0x844EE02
// IDA_NAME: _ZN16DB_LoadDimensionC2Ev
// DEMANGLED: DB_LoadDimension::DB_LoadDimension(void)
DB_LoadDimension::DB_LoadDimension() {}

// IDA_EA: 0x845D966
// IDA_NAME: _ZN16DB_LoadDimensionD2Ev
// DEMANGLED: DB_LoadDimension::~DB_LoadDimension()
// IDA_EA: 0x845D996
// IDA_NAME: _ZN16DB_LoadDimensionD0Ev
// DEMANGLED: DB_LoadDimension::~DB_LoadDimension() deleting destructor
DB_LoadDimension::~DB_LoadDimension() {}

// IDA_EA: 0x8431D8C
// IDA_NAME: _ZN16DB_LoadDimension11makeRequestEij
// DEMANGLED: DB_LoadDimension::makeRequest(int,unsigned int)
void DB_LoadDimension::makeRequest(int uid, unsigned int charac_no) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 30998);
    CStreamGuard guard(stream, true);
    *guard << 334;
    *guard << uid;

    SIG_LOAD_DIMENSION* payload =
        guard.GetInBuffer<SIG_LOAD_DIMENSION>();
    if (!payload) {
        return;
    }
    memset(payload, 0, sizeof(*payload));
    payload->charac_no = charac_no;

    CDataManager* data_manager = G_CDataManager();
    for (int index = 0; index <= 5; ++index) {
        payload->dimension[index] =
            data_manager ? static_cast<char>(
                               data_manager->get_dimensionInout(index))
                         : 0;
    }
    CDungeon* const black_dragon =
        data_manager ? data_manager->find_dungeon(122) : 0;
    payload->tournament_inout =
        black_dragon
            ? static_cast<char>(black_dragon->get_limit_Inout_count())
            : 0;
    if (!PackLimitedInCountMaximums(&payload->limited_inout)) {
        return;
    }
    payload->blood =
        data_manager
            ? static_cast<char>(data_manager->get_limit_inout_count(2))
            : 0;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8431ECE
// IDA_NAME: _ZN16DB_LoadDimension8dispatchEiiP6Stream
// DEMANGLED: DB_LoadDimension::dispatch(int,int,Stream *)
bool DB_LoadDimension::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_LOAD_DIMENSION* payload =
        stream->GetOutBuffer<SIG_LOAD_DIMENSION>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect dungeon1, dungeon2, dungeon3, dungeon4, dungeon5, dungeon6, dungeon7, dungeon8 from charac_dimension_inout where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        db_handle->set_query(
            "inSert into charac_dimension_inout(charac_no,dungeon1,dungeon2,dungeon3,dungeon4,dungeon5,dungeon6,dungeon7,dungeon8) values(%u,%d,%d,%d,%d,%d,%d,%d,%u)",
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
            return false;
        }
    } else {
        for (int index = 0; index <= 5; ++index) {
            if (db_handle->get_byte(index, payload->dimension[index]) != true) {
                return false;
            }
        }
        if (db_handle->get_byte(6, payload->tournament_inout) != true) {
            return false;
        }
        if (db_handle->get_ubyte(7, payload->limited_inout) != true) {
            return false;
        }
    }

    db_handle->set_query(
        "seLect dungeon1 from charac_blood_inout where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        db_handle->set_query(
            "inSert into charac_blood_inout(charac_no,dungeon1) values(%u,%d)",
            payload->charac_no,
            payload->blood);
        if (db_handle->exec(true) != true) {
            return false;
        }
    } else if (db_handle->get_byte(0, payload->blood) != true) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 31061);
    CStreamGuard guard(out_stream, true);
    *guard << 334;
    *guard << user_idx;
    SIG_LOAD_DIMENSION* dest =
        guard.GetInBuffer<SIG_LOAD_DIMENSION>();
    if (!dest) {
        return false;
    }
    memcpy(dest, payload, sizeof(*dest));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
