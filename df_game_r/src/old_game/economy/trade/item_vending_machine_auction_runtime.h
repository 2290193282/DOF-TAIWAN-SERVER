#ifndef OLDSRV_OLD_GAME_NPC_ITEM_VENDING_MACHINE_AUCTION_RUNTIME_H_
#define OLDSRV_OLD_GAME_NPC_ITEM_VENDING_MACHINE_AUCTION_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/inventory/taiwan_billing_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"

// IDA unit: 99_misc_global/ItemVendingMachine::AuctionNeedInfo.  This is the
// exact native record written to the original 0x28-byte billing scratch
// stream before the two point-auction purchases are made.
struct ItemVendingMachine::AuctionNeedInfo {
    int32_t bid_cera;                        // +0x00
    uint32_t auction_key_low;                // +0x04
    uint32_t auction_key_high;               // +0x08
    int32_t auction_tax;                     // +0x0c
    Taiwan::BillTransaction primary_transaction;    // +0x10
    Taiwan::BillTransaction secondary_transaction;  // +0x1c
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ItemVendingMachine::AuctionNeedInfo) == 0x28u,
              "AuctionNeedInfo IDA ABI size must stay 0x28");
static_assert(offsetof(ItemVendingMachine::AuctionNeedInfo, auction_tax) ==
                  0x0cu,
              "AuctionNeedInfo tax offset must stay +0x0C");
static_assert(offsetof(ItemVendingMachine::AuctionNeedInfo,
                       primary_transaction) == 0x10u,
              "AuctionNeedInfo primary transaction offset must stay +0x10");
static_assert(offsetof(ItemVendingMachine::AuctionNeedInfo,
                       secondary_transaction) == 0x1cu,
              "AuctionNeedInfo secondary transaction offset must stay +0x1C");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_ITEM_VENDING_MACHINE_AUCTION_RUNTIME_H_
