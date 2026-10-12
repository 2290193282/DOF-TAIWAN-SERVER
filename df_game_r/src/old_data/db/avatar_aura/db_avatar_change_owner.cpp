#include "old_data/db/avatar_aura/db_avatar_change_owner.h"

#include "common/config.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



namespace {

enum {
    kDbSignalAvatarChangeOwner = 53,
};

}  // namespace

// IDA_EA: 0x844D7F2
// IDA_NAME: _ZN20DB_AvatarChangeOwnerC2Ev
// DEMANGLED: DB_AvatarChangeOwner::DB_AvatarChangeOwner(void)
DB_AvatarChangeOwner::DB_AvatarChangeOwner() {}

// IDA_EA: 0x844D80E
// IDA_NAME: _ZN20DB_AvatarChangeOwnerD2Ev
// DEMANGLED: DB_AvatarChangeOwner::~DB_AvatarChangeOwner()
// IDA_EA: 0x844D83E
// IDA_NAME: _ZN20DB_AvatarChangeOwnerD0Ev
// DEMANGLED: DB_AvatarChangeOwner::~DB_AvatarChangeOwner() deleting destructor
DB_AvatarChangeOwner::~DB_AvatarChangeOwner() {}

// IDA_EA: 0x84004BC
// IDA_NAME: _ZN20DB_AvatarChangeOwner8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarChangeOwner::dispatch(int,int,Stream *)
bool DB_AvatarChangeOwner::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_AVATAR_CHANGE_OWNER* payload =
        stream->GetOutBuffer<SIG_AVATAR_CHANGE_OWNER>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate user_items set charac_no=%u,slot=%d where ui_id=%d && charac_no=%u",
        payload->new_charac_no,
        payload->slot,
        payload->ui_id,
        payload->old_charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x840055C
// IDA_NAME: _ZN20DB_AvatarChangeOwner11makeRequestEjjjh
// DEMANGLED: DB_AvatarChangeOwner::makeRequest(unsigned int,unsigned int,unsigned int,unsigned char)
void DB_AvatarChangeOwner::makeRequest(unsigned int ui_id,
                                       unsigned int cur_charac_no,
                                       unsigned int new_charac_no,
                                       unsigned char slot) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 2943);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalAvatarChangeOwner;
    *guard << -1;
    SIG_AVATAR_CHANGE_OWNER* payload =
        guard.GetInBuffer<SIG_AVATAR_CHANGE_OWNER>();
    if (!payload) {
        return;
    }
    memset(payload, 0, sizeof(*payload));
    payload->ui_id = ui_id;
    payload->old_charac_no = cur_charac_no;
    payload->new_charac_no = new_charac_no;
    payload->slot = static_cast<unsigned char>(
        slot + oldsrv::config::kAvatarInventorySlotBase);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
