#ifndef OLDSRV_DB_DB_FORBID_USER_TO_PLAY_H_
#define OLDSRV_DB_DB_FORBID_USER_TO_PLAY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_ForbidUserToPlay.
// IDA_EA: dispatch @0x8421AF4; ctor/dtor @0x844E3D6/@0x845F2B0/@0x845F2E0.
// Old DBTask stores this object at slot +148, signal 147.
class DB_ForbidUserToPlay : public DBDispatcher {
public:
    DB_ForbidUserToPlay();
    virtual ~DB_ForbidUserToPlay();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_FORBID_USER_TO_PLAY_H_
