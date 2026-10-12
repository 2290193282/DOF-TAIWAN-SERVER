#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"


namespace {

CUser* HistoryOwner(cUserHistoryLog* history) {
    return *reinterpret_cast<CUser**>(history);
}

}  // namespace

// IDA_EA: 0x8683D20
// IDA_NAME: _ZN15cUserHistoryLog12ItemDelCargoEiii14eItemDelReason
// DEMANGLED: cUserHistoryLog::ItemDelCargo(int,int,int,eItemDelReason)
int cUserHistoryLog::ItemDelCargo(int item_id,
                                  int item_count,
                                  int remaining_count,
                                  eItemDelReason reason) {
    return HistoryOwner(this)->LogHistory("Inven-,%d,%d,%d,%d",
                                          item_id,
                                          item_count,
                                          remaining_count,
                                          reason);
}

// IDA_EA: 0x86868B4
// IDA_NAME: _ZN15cUserHistoryLog19AccountCargoAddItemEiii14eItemAddReason
// DEMANGLED: cUserHistoryLog::AccountCargoAddItem(int,int,int,eItemAddReason)
int cUserHistoryLog::AccountCargoAddItem(int item_id,
                                         int money,
                                         int amount,
                                         eItemAddReason reason) {
    return HistoryOwner(this)->LogHistory("AccountCargo+,%d,%d,%d,%d",
                                          reason,
                                          item_id,
                                          money,
                                          amount);
}

// IDA_EA: 0x86868EE
// IDA_NAME: _ZN15cUserHistoryLog19AccountCargoDelItemEiii14eItemDelReason
// DEMANGLED: cUserHistoryLog::AccountCargoDelItem(int,int,int,eItemDelReason)
int cUserHistoryLog::AccountCargoDelItem(int item_id,
                                         int money,
                                         int amount,
                                         eItemDelReason reason) {
    return HistoryOwner(this)->LogHistory("AccountCargo-,%d,%d,%d,%d",
                                          reason,
                                          item_id,
                                          money,
                                          amount);
}

// IDA_EA: 0x8682C96
// IDA_NAME: _ZN15cUserHistoryLog5WPAddEii12eWPAddReason
// DEMANGLED: cUserHistoryLog::WPAdd(int,int,eWPAddReason)
int cUserHistoryLog::WPAdd(int current_point,
                           int added_point,
                           eWPAddReason reason) {
    return owner_->LogHistory("WP+,%d,%d,%d",
                              current_point,
                              added_point,
                              reason);
}

// IDA_EA: 0x8682CC8
// IDA_NAME: _ZN15cUserHistoryLog5WPSubEii12eWPSubReason
// DEMANGLED: cUserHistoryLog::WPSub(int,int,eWPSubReason)
int cUserHistoryLog::WPSub(int current_point,
                           int removed_point,
                           eWPSubReason reason) {
    return owner_->LogHistory("WP-,%d,%d,%d",
                              current_point,
                              removed_point,
                              reason);
}

// IDA_EA: 0x8684B96
// IDA_NAME: _ZN15cUserHistoryLog5FPSubEiii12eFPSubReason
// DEMANGLED: cUserHistoryLog::FPSub(int,int,int,eFPSubReason)
int cUserHistoryLog::FPSub(int point,
                           int remaining_point,
                           int total_point,
                           eFPSubReason reason) {
    return owner_->LogHistory("FP-,%d,%d,%d,%d",
                              point,
                              remaining_point,
                              total_point,
                              reason);
}

// IDA_EA: 0x8684B5C
// IDA_NAME: _ZN15cUserHistoryLog5FPAddEiii12eFPAddReason
// DEMANGLED: cUserHistoryLog::FPAdd(int,int,int,eFPAddReason)
int cUserHistoryLog::FPAdd(int point,
                           int remaining_point,
                           int total_point,
                           eFPAddReason reason) {
    return owner_->LogHistory("FP+,%d,%d,%d,%d",
                              point,
                              remaining_point,
                              total_point,
                              reason);
}

// IDA_EA: 0x8683BC2
// IDA_NAME: _ZN15cUserHistoryLog7CoinSubEii14eCoinSubReason
// DEMANGLED: cUserHistoryLog::CoinSub(int,int,eCoinSubReason)
int cUserHistoryLog::CoinSub(int coin, int sub_count, eCoinSubReason reason) {
    return owner_->LogHistory("Coin-,%d,%d,%d", coin, sub_count, reason);
}

// IDA_EA: 0x8683C26
// IDA_NAME: _ZN15cUserHistoryLog10PayCoinSubEii14eCoinSubReason
// DEMANGLED: cUserHistoryLog::PayCoinSub(int,int,eCoinSubReason)
int cUserHistoryLog::PayCoinSub(int coin,
                                int sub_count,
                                eCoinSubReason reason) {
    return owner_->LogHistory("PCoin-,%d,%d,%d", coin, sub_count, reason);
}

// IDA_EA: 0x8683C58
// IDA_NAME: _ZN15cUserHistoryLog12EventCoinAddEii14eCoinAddReason
// DEMANGLED: cUserHistoryLog::EventCoinAdd(int,int,eCoinAddReason)
int cUserHistoryLog::EventCoinAdd(int current_coin,
                                  int added_coin,
                                  eCoinAddReason reason) {
    return owner_->LogHistory("ECoin+,%d,%d,%d",
                              current_coin,
                              added_coin,
                              reason);
}

// IDA_EA: 0x8683C8A
// IDA_NAME: _ZN15cUserHistoryLog12EventCoinSubEii14eCoinSubReason
// DEMANGLED: cUserHistoryLog::EventCoinSub(int,int,eCoinSubReason)
int cUserHistoryLog::EventCoinSub(int coin,
                                  int sub_count,
                                  eCoinSubReason reason) {
    return owner_->LogHistory("ECoin-,%d,%d,%d", coin, sub_count, reason);
}
