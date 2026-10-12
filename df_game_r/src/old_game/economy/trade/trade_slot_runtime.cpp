#include "old_game/economy/trade/trade_slot.h"

// IDA_EA: 0x82A411A
// IDA_NAME: _ZN9TradeSlotC2Ev
// DEMANGLED: TradeSlot::TradeSlot(void)
TradeSlot::TradeSlot()
    : item_() {
    Reset();
}

// IDA_EA: 0x82A413C
// IDA_NAME: _ZN9TradeSlot5ResetEv
// DEMANGLED: TradeSlot::Reset(void)
TradeSlot* TradeSlot::Reset() {
    item_.reset();
    original_slot_ = 0;
    owner_character_no_ = 0;
    item_space_ = 0;
    return this;
}
