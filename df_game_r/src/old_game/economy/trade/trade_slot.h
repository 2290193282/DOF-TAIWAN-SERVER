#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_SLOT_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_SLOT_H_

#include "old_game/economy/inventory/inventory.h"

#include <stddef.h>
#include <stdint.h>


// IDA unit: TradeSlot. This is the one C-owned trade escrow record shared by
// character construction and CTradeSpace. Its 0x48-byte stride is verified by
// CTradeSpace @0x82A416C and every trade slot access at +0x09/+0x0b.
#pragma pack(push, 1)
struct TradeSlot {
    // IDA_EA: 0x82A411A
    // IDA_NAME: _ZN9TradeSlotC2Ev
    // DEMANGLED: TradeSlot::TradeSlot(void)
    TradeSlot();

    // IDA_EA: 0x82A413C
    // IDA_NAME: _ZN9TradeSlot5ResetEv
    // DEMANGLED: TradeSlot::Reset(void)
    TradeSlot* Reset();

    uint32_t original_slot_;       // IDA +0x00
    uint32_t owner_character_no_;  // IDA +0x04
    uint8_t item_space_;           // IDA +0x08
    Inven_Item item_;              // IDA +0x09
    uint8_t alignment_padding_[2]; // IDA +0x46
};
#pragma pack(pop)

static_assert(offsetof(TradeSlot, original_slot_) == 0x00u,
              "TradeSlot original inventory slot must remain at +0x00");
static_assert(offsetof(TradeSlot, owner_character_no_) == 0x04u,
              "TradeSlot owner character number must remain at +0x04");
static_assert(offsetof(TradeSlot, item_space_) == 0x08u,
              "TradeSlot item-space discriminator must remain at +0x08");
static_assert(offsetof(TradeSlot, item_) == 0x09u,
              "TradeSlot escrow item must remain at +0x09");
static_assert(sizeof(TradeSlot) == 0x48u,
              "TradeSlot old i386/wire ABI must remain 0x48 bytes");

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_SLOT_H_
