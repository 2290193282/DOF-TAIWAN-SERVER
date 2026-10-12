#include "old_game/economy/mail/mail_trade_value_types.h"

#include <cstring>

#include "common/manager/user_history_log_abi.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85295F0
// IDA_NAME: _ZN11CTradeSpace11set_tradersEP5CUserS1_
// DEMANGLED: CTradeSpace::set_traders(CUser *,CUser *)
//
// The old body establishes both directions of the session relationship before
// it assigns the pooled trade index, clears both escrow rows, and opens the
// two history-log transactions.  CUser's peer-id and trade-index are named
// physical fields; no CUser offset overlay is used here.
int CTradeSpace::set_traders(CUser* first, CUser* second) {
    lock();
    occupied_ = 1;
    _SetLock(false);

    traders_[0] = first;
    first->trade_peer_unique_id_ =
        static_cast<uint16_t>(second->get_unique_id());
    traders_[1] = second;
    second->trade_peer_unique_id_ =
        static_cast<uint16_t>(first->get_unique_id());

    if (second->getCurCharacR() && first->getCurCharacR()) {
        CUser_history_log(first)->SetTrader(second->get_acc_name(),
                                            second->getCurCharacName());
        CUser_history_log(second)->SetTrader(first->get_acc_name(),
                                             first->getCurCharacName());
    }

    first->SetTradeSpace(index_);
    second->SetTradeSpace(index_);
    trade_gold_[0] = 0;
    trade_gold_[1] = 0;
    trade_states_[0] = 0;
    trade_states_[1] = 0;
    std::memset(trade_slots_, 0, sizeof(trade_slots_));
    std::memset(received_creature_item_counts_,
                0,
                sizeof(received_creature_item_counts_));
    unlock();

    CUser_history_log(first)->TradeBegin();
    return CUser_history_log(second)->TradeBegin();
}
