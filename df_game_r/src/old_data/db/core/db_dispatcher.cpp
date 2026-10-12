#include "old_data/db/core/db_dispatcher.h"

#include "common/manager/global_lifecycle.h"


class CUser;

// IDA_EA: 0x844D7A6
// IDA_NAME: _ZN12DBDispatcherC2Ev
// DEMANGLED: DBDispatcher::DBDispatcher(void)
DBDispatcher::DBDispatcher() {}

// IDA_EA: 0x80E2378
// IDA_NAME: _ZN12DBDispatcherD2Ev
// DEMANGLED: DBDispatcher::~DBDispatcher()
// IDA_EA: 0x80E239E
// IDA_NAME: _ZN12DBDispatcherD0Ev
// DEMANGLED: DBDispatcher::~DBDispatcher() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DBDispatcher::~DBDispatcher() {}

// IDA_EA: 0x83FBF7C
// IDA_NAME: _ZN12DBDispatcher8dispatchEiiP6Stream
// DEMANGLED: DBDispatcher::dispatch(int,int,Stream *)
bool DBDispatcher::dispatch(int signal, int user_idx, Stream* stream) {
    (void)stream;
    if (user_idx != -1) {
        CGameManager* manager = G_CGameManager();
        // IDA 0x83FBF97..0x83FBFA5: getUser(user_idx, signal + 0x7D0).
        // The first argument is the packed CUser UID; reversing the pair
        // makes the signal-derived value look like a slot/increment ID.
        if (manager == 0 ||
            manager->getUser(user_idx, signal + 0x7D0) == 0) {
            return false;
        }
    }
    return true;
}
