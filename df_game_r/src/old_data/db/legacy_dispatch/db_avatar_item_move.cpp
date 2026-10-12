#include "old_data/db/avatar_aura/db_avatar_item_move.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x840031E
// IDA_NAME: _ZN17DB_AvatarItemMoveC2Ev
// DEMANGLED: DB_AvatarItemMove::DB_AvatarItemMove(void)
DB_AvatarItemMove::DB_AvatarItemMove() {}

// IDA_EA: 0x840033A
// IDA_NAME: _ZN17DB_AvatarItemMoveD2Ev
// DEMANGLED: DB_AvatarItemMove::~DB_AvatarItemMove()
// IDA_EA: 0x840036A
// IDA_NAME: _ZN17DB_AvatarItemMoveD0Ev
// DEMANGLED: DB_AvatarItemMove::~DB_AvatarItemMove() deleting destructor
DB_AvatarItemMove::~DB_AvatarItemMove() {}

// IDA_EA: 0x8400388
// IDA_NAME: _ZN17DB_AvatarItemMove8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarItemMove::dispatch(int,int,Stream *)
bool DB_AvatarItemMove::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_AVATAR_MOVE* payload = stream->GetOutBuffer<SIG_AVATAR_MOVE>();
    if (!payload) {
        return false;
    }
    return MoveAvatarItem(payload) == true;
}

// IDA_EA: 0x84003CA
// IDA_NAME: _ZN17DB_AvatarItemMove14MoveAvatarItemEP15SIG_AVATAR_MOVE
// DEMANGLED: DB_AvatarItemMove::MoveAvatarItem(SIG_AVATAR_MOVE *)
bool DB_AvatarItemMove::MoveAvatarItem(SIG_AVATAR_MOVE* payload) {
    ChangeSlot(payload->first_ui_id,
               payload->first_slot,
               payload->first_clear_avatar_id);
    ChangeSlot(payload->second_ui_id,
               payload->second_slot,
               payload->second_clear_avatar_id);
    return true;
}

// IDA_EA: 0x8400432
// IDA_NAME: _ZN17DB_AvatarItemMove10ChangeSlotEiii
// DEMANGLED: DB_AvatarItemMove::ChangeSlot(int,int,int)
bool DB_AvatarItemMove::ChangeSlot(int ui_id,
                                   int slot,
                                   int clear_avatar_id) {
    if (ui_id < 0) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate user_items set slot=%d,clear_avatar_id=%d where ui_id=%d",
        slot,
        clear_avatar_id,
        ui_id);
    return db_handle->exec(true) == true;
}
