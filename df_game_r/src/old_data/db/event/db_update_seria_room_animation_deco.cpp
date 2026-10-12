#include "old_data/db/event/db_update_seria_room_animation_deco.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

enum {
    kDbSignalUpdateSeriaRoomAnimationDeco = 798,
};

}  // namespace

// IDA_EA: 0x844FEBE
// IDA_NAME: _ZN31DB_UpdateSeriaRoomAnimationDecoC2Ev
DB_UpdateSeriaRoomAnimationDeco::DB_UpdateSeriaRoomAnimationDeco() {}

// IDA_EA: 0x845ABB2
// IDA_NAME: _ZN31DB_UpdateSeriaRoomAnimationDecoD2Ev
// IDA_EA: 0x845ABE2
// IDA_NAME: _ZN31DB_UpdateSeriaRoomAnimationDecoD0Ev
DB_UpdateSeriaRoomAnimationDeco::~DB_UpdateSeriaRoomAnimationDeco() {}

// IDA_EA: 0x844ADC0
// IDA_NAME: _ZN31DB_UpdateSeriaRoomAnimationDeco8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateSeriaRoomAnimationDeco::dispatch(int, int, Stream*)
bool DB_UpdateSeriaRoomAnimationDeco::dispatch(int signal,
                                               int user_idx,
                                               Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    short server_id = 0;
    int charac_no = 0;
    short receive_flow = 0;
    short deco_flow = 0;
    *stream >> account_id;
    *stream >> server_id;
    *stream >> charac_no;
    *stream >> receive_flow;
    *stream >> deco_flow;

    db_handle->set_query(
        "upDate event_1208_seriaroom_anideco set receive_flow = %d, deco_flow = %d where m_id = %s and server_id = %d and charac_no = %d",
        receive_flow,
        deco_flow,
        NumberToString(account_id, 0),
        server_id,
        charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into event_1208_seriaroom_anideco(m_id ,server_id, charac_no, receive_flow, deco_flow, occ_date) values(%s, %d, %d, %d, %d, now())",
        NumberToString(account_id, 0),
        server_id,
        charac_no,
        receive_flow,
        deco_flow);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x844AF74
// IDA_NAME: _ZN31DB_UpdateSeriaRoomAnimationDeco11makeRequestEjsiss
// DEMANGLED: DB_UpdateSeriaRoomAnimationDeco::makeRequest(unsigned int, short, int, short, short)
void DB_UpdateSeriaRoomAnimationDeco::makeRequest(unsigned int account_id,
                                                   short server_id,
                                                   int charac_no,
                                                   short receive_flow,
                                                   short deco_flow) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 48024);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateSeriaRoomAnimationDeco;
    *guard << -1;
    *guard << account_id;
    *guard << server_id;
    *guard << charac_no;
    *guard << receive_flow;
    *guard << deco_flow;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
