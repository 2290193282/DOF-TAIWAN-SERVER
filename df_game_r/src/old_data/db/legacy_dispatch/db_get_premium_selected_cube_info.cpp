#include "old_data/db/auction_cashshop/db_get_premium_selected_cube_info.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>

// IDA_EA: 0x844FF2E
// IDA_NAME: _ZN29DB_GetPremiumSelectedCubeInfoC2Ev
// DEMANGLED: DB_GetPremiumSelectedCubeInfo::DB_GetPremiumSelectedCubeInfo(void)
DB_GetPremiumSelectedCubeInfo::DB_GetPremiumSelectedCubeInfo() {}

// IDA_EA: 0x845A8C2
// IDA_NAME: _ZN29DB_GetPremiumSelectedCubeInfoD2Ev
// DEMANGLED: DB_GetPremiumSelectedCubeInfo::~DB_GetPremiumSelectedCubeInfo()
// IDA_EA: 0x845A8F2
// IDA_NAME: _ZN29DB_GetPremiumSelectedCubeInfoD0Ev
// DEMANGLED: DB_GetPremiumSelectedCubeInfo::~DB_GetPremiumSelectedCubeInfo() deleting destructor
DB_GetPremiumSelectedCubeInfo::~DB_GetPremiumSelectedCubeInfo() {}

// IDA_EA: 0x844C4E2
// IDA_NAME: _ZN29DB_GetPremiumSelectedCubeInfo8dispatchEiiP6Stream
// DEMANGLED: DB_GetPremiumSelectedCubeInfo::dispatch(int,int,Stream *)
bool DB_GetPremiumSelectedCubeInfo::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    SIG_GET_PREMIUM_SELECTED_CUBE_INFO request;
    if (stream->get_binary(&request, sizeof(request)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }

    db_handle->set_query(
        "seLect selected, cube_type from cube_premium where charac_no=%u",
        request.charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES response;
    memset(&response, 0, sizeof(response));
    response.charac_no = request.charac_no;

    if (db_handle->fetch()) {
        if (db_handle->get_ubyte(0, response.selected) != true) {
            return false;
        }
        if (db_handle->get_ubyte(1, response.cube_type) != true) {
            return false;
        }
    }

    Stream* out_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                            49009);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(&response, sizeof(response));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
