#include "old_game/player/core/cuser.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x85DF9B2
// IDA_NAME: _ZN5CUser11SetPvpIndexEs
// DEMANGLED: CUser::SetPvpIndex(short)
int CUser::SetPvpIndex(short pvp_index) {
    cMyTrace trace("SetPvpIndex", 9017, 0);
    trace("pvp@log %s,%s,room(%d)",
          get_acc_name(),
          getCurCharacName(),
          static_cast<int>(pvp_index));

    lock();
    pvp_index_ = pvp_index;
    return unlock();
}
