#ifndef OLDSRV_GLOBAL_USER_HISTORY_LOG_ABI_H_
#define OLDSRV_GLOBAL_USER_HISTORY_LOG_ABI_H_

#include "old_game/player/core/cuser.h"

inline cUserHistoryLog* CUser_history_log(CUser* user) {
    return user ? &user->history_log() : 0;
}


#endif  // OLDSRV_GLOBAL_USER_HISTORY_LOG_ABI_H_
