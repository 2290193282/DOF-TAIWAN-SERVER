#include "old_data/db/avatar_aura/db_update_avatar_jewel_slot.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


namespace {

enum {
    kDbSignalUpdateAvatarJewelSlot = 313,
    kAvatarJewelDbHandle = 3,
};

}  // namespace

// IDA_EA: 0x844EC96
// IDA_NAME: _ZN24DB_UpdateAvatarJewelSlotC2Ev
// DEMANGLED: DB_UpdateAvatarJewelSlot::DB_UpdateAvatarJewelSlot(void)
DB_UpdateAvatarJewelSlot::DB_UpdateAvatarJewelSlot() {}

// IDA_EA: 0x845DAEC
// IDA_NAME: _ZN24DB_UpdateAvatarJewelSlotD2Ev
// DEMANGLED: DB_UpdateAvatarJewelSlot::~DB_UpdateAvatarJewelSlot()
// IDA_EA: 0x845DB1C
// IDA_NAME: _ZN24DB_UpdateAvatarJewelSlotD0Ev
// DEMANGLED: DB_UpdateAvatarJewelSlot::~DB_UpdateAvatarJewelSlot() deleting destructor
DB_UpdateAvatarJewelSlot::~DB_UpdateAvatarJewelSlot() {}

// IDA_EA: 0x843077C
// IDA_NAME: _ZN24DB_UpdateAvatarJewelSlot8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateAvatarJewelSlot::dispatch(int,int,Stream *)
bool DB_UpdateAvatarJewelSlot::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_AVATAR_JEWEL_SOCKET* payload =
        stream->GetOutBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>();
    if (payload == 0) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAvatarJewelDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    char* jewel_socket = db_handle->blob_to_str(
        0, &payload->jewel_socket, sizeof(payload->jewel_socket));
    db_handle->set_query(
        "upDate user_items set jewel_socket='%s' where ui_id=%d and charac_no=%d",
        jewel_socket,
        payload->avatar_ui_id,
        payload->charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x843081C
// IDA_NAME: _ZN24DB_UpdateAvatarJewelSlot11makeRequestEjjPK20stAvatarEmblemInfo_t
// DEMANGLED: DB_UpdateAvatarJewelSlot::makeRequest(unsigned int,unsigned int,stAvatarEmblemInfo_t const*)
void DB_UpdateAvatarJewelSlot::makeRequest(
    unsigned int charac_no,
    unsigned int avatar_ui_id,
    const stAvatarEmblemInfo_t* jewel_socket) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 30513);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateAvatarJewelSlot;
    *guard << -1;

    SIG_UPDATE_AVATAR_JEWEL_SOCKET* payload =
        guard->GetInBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>();
    payload->charac_no = charac_no;
    payload->avatar_ui_id = avatar_ui_id;
    std::memcpy(&payload->jewel_socket,
                jewel_socket,
                sizeof(payload->jewel_socket));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
