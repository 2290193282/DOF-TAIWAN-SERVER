#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8683BF4
// IDA_NAME: _ZN15cUserHistoryLog10PayCoinAddEii14eCoinAddReason
// DEMANGLED: cUserHistoryLog::PayCoinAdd(int,int,eCoinAddReason)
int cUserHistoryLog::PayCoinAdd(int coin,
                                int add_count,
                                eCoinAddReason reason) {
    return owner_->LogHistory(
        "PCoin+,%d,%d,%d", coin, add_count, static_cast<int>(reason));
}
