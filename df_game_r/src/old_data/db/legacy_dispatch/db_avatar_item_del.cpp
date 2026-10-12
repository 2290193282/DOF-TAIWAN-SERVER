#include "old_data/db/avatar_aura/db_avatar_item_del.h"

#include "infra/network/packet/stream.h"

#include <string.h>


// IDA_EA: 0x83FFC42
// IDA_NAME: _ZN16DB_AvatarItemDelC2Ev
// DEMANGLED: DB_AvatarItemDel::DB_AvatarItemDel(void)
DB_AvatarItemDel::DB_AvatarItemDel() {}

// IDA_EA: 0x83FFC5E
// IDA_NAME: _ZN16DB_AvatarItemDelD2Ev
// DEMANGLED: DB_AvatarItemDel::~DB_AvatarItemDel()
// IDA_EA: 0x83FFC8E
// IDA_NAME: _ZN16DB_AvatarItemDelD0Ev
// DEMANGLED: DB_AvatarItemDel::~DB_AvatarItemDel() deleting destructor
DB_AvatarItemDel::~DB_AvatarItemDel() {}

// IDA_EA: 0x83FFCAC
// IDA_NAME: _ZN16DB_AvatarItemDel8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarItemDel::dispatch(int,int,Stream *)
bool DB_AvatarItemDel::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    SIG_AVATAR_DELETE payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    return DelAvatarItem(payload) == true;
}

// IDA_EA: 0x83FFD46
// IDA_NAME: _ZN16DB_AvatarItemDel13DelAvatarItemER17SIG_AVATAR_DELETE
// DEMANGLED: DB_AvatarItemDel::DelAvatarItem(SIG_AVATAR_DELETE &)
bool DB_AvatarItemDel::DelAvatarItem(SIG_AVATAR_DELETE& payload) {
    (void)payload;
    return true;
}
