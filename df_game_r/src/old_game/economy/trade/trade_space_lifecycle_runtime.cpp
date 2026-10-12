#include "old_game/economy/mail/mail_trade_value_types.h"

#include "common/manager/user_history_log_abi.h"
#include "old_game/player/core/cuser.h"

#include <cstring>

// This file intentionally covers only the IDA-proven lifecycle/readiness
// portion of CTradeSpace.  The escrow mutation/commit/cancel state machine is
// a separate closure: it must not be enabled until its inventory, CUser and
// database contracts are restored without raw object overlays.

// IDA_EA: 0x823438E
// IDA_NAME: _ZN11CTradeSpace9get_indexEv
// DEMANGLED: CTradeSpace::get_index(void)
int CTradeSpace::get_index() {
    return index_;
}

// IDA_EA: 0x823439A
// IDA_NAME: _ZN11CTradeSpace8is_emptyEv
// DEMANGLED: CTradeSpace::is_empty(void)
int CTradeSpace::is_empty() {
    return occupied_ ^ 1;
}

// IDA_EA: 0x82A416C
// IDA_NAME: _ZN11CTradeSpaceC2Ev
// DEMANGLED: CTradeSpace::CTradeSpace(void)
CTradeSpace::CTradeSpace()
    : mutex_(),
      occupied_(0),
      locked_(0) {}

// IDA_EA: 0x82A41E0
// IDA_NAME: _ZN11CTradeSpaceD2Ev
// DEMANGLED: CTradeSpace::~CTradeSpace()
CTradeSpace::~CTradeSpace() {}

// IDA_EA: 0x82A41F4
// IDA_NAME: _ZN11CTradeSpace6SetIDXEi
// DEMANGLED: CTradeSpace::SetIDX(int)
CTradeSpace* CTradeSpace::SetIDX(int index) {
    index_ = index;
    return this;
}

// IDA_EA: 0x85297F2
// IDA_NAME: _ZN11CTradeSpace5resetEv
// DEMANGLED: CTradeSpace::reset(void)
void CTradeSpace::reset() {
    lock();
    _SetLock(false);
    occupied_ = 0;

    for (int trader_index = 0; trader_index != 2; ++trader_index) {
        CUser* const trader = traders_[trader_index];
        if (trader) {
            trader->SetTradeSpace(-1);
            CUser_history_log(trader)->SetTrader("", "");
        }
    }

    trade_gold_[0] = 0;
    trade_gold_[1] = 0;
    trade_states_[0] = 2;
    trade_states_[1] = 2;
    std::memset(trade_slots_, 0, sizeof(trade_slots_));
    std::memset(received_creature_item_counts_,
                0,
                sizeof(received_creature_item_counts_));
    unlock();
}

// IDA_EA: 0x852993C
// IDA_NAME: _ZN11CTradeSpace23check_trade_possibilityEv
// DEMANGLED: CTradeSpace::check_trade_possibility(void)
int CTradeSpace::check_trade_possibility() {
    if (occupied_ != 1) {
        return 0;
    }

    const int first_trade_space = traders_[0]->GetTradeSpace();
    if (first_trade_space != traders_[1]->GetTradeSpace()) {
        return 0;
    }

    return traders_[0]->get_state() == traders_[1]->get_state();
}

// IDA_EA: 0x8529A72
// IDA_NAME: _ZN11CTradeSpace15GetOppositeUserEP5CUser
// DEMANGLED: CTradeSpace::GetOppositeUser(CUser *)
CUser* CTradeSpace::GetOppositeUser(CUser* user) {
    if (!occupied_) {
        return 0;
    }
    return traders_[0] == user ? traders_[1] : traders_[0];
}

// IDA_EA: 0x85320E2
// IDA_NAME: _ZN11CTradeSpace4lockEv
// DEMANGLED: CTradeSpace::lock(void)
void CTradeSpace::lock() {
    mutex_.lock();
}

// IDA_EA: 0x85320E8
// IDA_NAME: _ZN11CTradeSpace6unlockEv
// DEMANGLED: CTradeSpace::unlock(void)
void CTradeSpace::unlock() {
    mutex_.unlock();
}

// IDA_EA: 0x85320EE
// IDA_NAME: _ZN11CTradeSpace8IsLockedEv
// DEMANGLED: CTradeSpace::IsLocked(void)
int CTradeSpace::IsLocked() {
    return locked_;
}

// IDA_EA: 0x85320FA
// IDA_NAME: _ZN11CTradeSpace8_SetLockEb
// DEMANGLED: CTradeSpace::_SetLock(bool)
CTradeSpace* CTradeSpace::_SetLock(bool locked) {
    locked_ = locked;
    return this;
}
