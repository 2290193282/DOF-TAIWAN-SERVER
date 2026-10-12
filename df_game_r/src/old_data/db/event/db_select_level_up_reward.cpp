#include "old_data/db/event/db_select_level_up_reward.h"

#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string>

namespace {

enum {
    kDbSignalSelectLevelUpReward = 654,
    kLevelUpRewardEventType = 148,
};

}  // namespace

// IDA_EA: 0x844F8D6
// IDA_NAME: _ZN22DB_SelectLevelUpRewardC2Ev
DB_SelectLevelUpReward::DB_SelectLevelUpReward() {}

// IDA_EA: 0x845BA04
// IDA_NAME: _ZN22DB_SelectLevelUpRewardD2Ev
// IDA_EA: 0x845BA34
// IDA_NAME: _ZN22DB_SelectLevelUpRewardD0Ev
DB_SelectLevelUpReward::~DB_SelectLevelUpReward() {}

// IDA_EA: 0x84423CC
// IDA_NAME: _ZN22DB_SelectLevelUpReward8dispatchEiiP6Stream
// DEMANGLED: DB_SelectLevelUpReward::dispatch(int, int, Stream*)
bool DB_SelectLevelUpReward::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    unsigned int charac_no = 0;
    int charac_level = 0;
    *stream >> charac_no;
    *stream >> charac_level;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const std::string table_name(
        *G_CDataManager()
             ->get_event_script_mng()
             ->get_db_table_name(kLevelUpRewardEventType));
    db_handle->set_query(
        "seLect  recv_level from %s where charac_no = %u",
        ::toTString(table_name),
        charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    unsigned char previous_receive_level = 0;
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true ||
            db_handle->get_ubyte(0, previous_receive_level) != true) {
            return false;
        }
        if (previous_receive_level == charac_level) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "virtual bool DB_SelectLevelUpReward::dispatch(int, int, Stream*)",
                41637,
                "DB_SelectLevelUpReward error char:%u lv:%u recv_lv:%u",
                charac_no,
                charac_level,
                previous_receive_level);
            return false;
        }
        db_handle->set_query(
            "upDate %s set recv_level=%u,occ_time=now() where charac_no=%u",
            ::toTString(table_name),
            charac_level,
            charac_no);
        if (db_handle->exec(true) != true) {
            return false;
        }
    } else {
        db_handle->set_query(
            "inSert into %s(charac_no,recv_level,occ_time) values (%u,%u,now() )",
            ::toTString(table_name),
            charac_no,
            charac_level);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    Stream* out = GlobalData::stream_pool()->Acquire("DBThread.cpp", 41658);
    CStreamGuard guard(out, true);
    *guard << signal;
    *guard << user_idx;
    *guard << static_cast<unsigned char>(charac_level);
    *guard << previous_receive_level;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844271A
// IDA_NAME: _ZN22DB_SelectLevelUpReward11makeRequestEiji
// DEMANGLED: DB_SelectLevelUpReward::makeRequest(int, unsigned int, int)
void DB_SelectLevelUpReward::makeRequest(int user_idx,
                                         unsigned int charac_no,
                                         int charac_level) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 41670);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSelectLevelUpReward;
    *guard << user_idx;
    *guard << charac_no;
    *guard << charac_level;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
