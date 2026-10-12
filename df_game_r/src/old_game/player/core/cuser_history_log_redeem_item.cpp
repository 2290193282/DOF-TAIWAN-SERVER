#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8686990
// IDA_NAME: _ZN15cUserHistoryLog13RedeemItemAddEii
// DEMANGLED: cUserHistoryLog::RedeemItemAdd(int,int)
int cUserHistoryLog::RedeemItemAdd(int item_id, int item_count) {
    return owner_->LogHistory("RedeemItem+,%d,%d", item_id, item_count);
}

// IDA_EA: 0x86869BC
// IDA_NAME: _ZN15cUserHistoryLog13RedeemItemDelEii
// DEMANGLED: cUserHistoryLog::RedeemItemDel(int,int)
int cUserHistoryLog::RedeemItemDel(int item_id, int item_count) {
    return owner_->LogHistory("RedeemItem-,%d,%d", item_id, item_count);
}
