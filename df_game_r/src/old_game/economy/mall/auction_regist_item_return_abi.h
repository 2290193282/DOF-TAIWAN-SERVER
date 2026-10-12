#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_REGIST_ITEM_RETURN_ABI_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_REGIST_ITEM_RETURN_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mall/auction_public_values.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/mail/mail_trade_packets.h"

// IDA unit: Inter_AuctionResultAskRegistedItemNum @ 0x84D5930.
// The DB-return payload is received without a packet header.  Its tail is
// copied verbatim into the ROI_Category part of PCK_AUCTION_REGIST_ITEM_GA.
#pragma pack(push, 1)
struct AuctionResultAskRegistedItemNumPayload {
    uint32_t payment_type;          // +0x00
    int32_t charac_no;              // +0x04
    int16_t registed_item_count;    // +0x08
    uint8_t item_space;             // +0x0a
    uint16_t item_slot;             // +0x0b
    uint32_t item_index;            // +0x0d
    int32_t item_count;             // +0x11
    int32_t field_15;               // +0x15
    int32_t field_19;               // +0x19
    int32_t field_1d;               // +0x1d
    ROI_Category category;          // +0x21
};

// The packet embeds the canonical packed DnfItemInfo physical object.  Keep
// the historical local name as a type alias so callers do not create a
// second byte-projection owner for the same 0x35-byte ABI.
typedef DnfItemInfo AuctionRegistItemWire;

// The canonical packet now owns every physical member directly.  The alias
// retains the handler's descriptive local type without introducing a second
// projection or any reinterpretation through a body offset.
typedef PCK_AUCTION_REGIST_ITEM_GA AuctionRegistItemGaBody;
#pragma pack(pop)

static_assert(sizeof(AuctionResultAskRegistedItemNumPayload) == 0x2du,
              "auction registered-item result payload must stay 0x2d");
static_assert(offsetof(AuctionResultAskRegistedItemNumPayload, category) ==
                  0x21u,
              "auction registered-item category must stay at +0x21");
static_assert(sizeof(AuctionRegistItemWire) == 0x35u,
              "auction registration item wire must stay 0x35");
static_assert(offsetof(AuctionRegistItemWire, item_id_) == 0x01u,
              "auction registration item id must stay at +0x01");
static_assert(offsetof(AuctionRegistItemWire, random_option_) == 0x1du,
              "auction registration random option must stay at +0x1d");
static_assert(offsetof(AuctionRegistItemWire, upgrade_separate_) == 0x2bu,
              "auction registration upgrade separate must stay at +0x2b");
static_assert(sizeof(AuctionRegistItemGaBody) == 0xc5u,
              "auction registration GA packet must stay 0xc5");
static_assert(offsetof(AuctionRegistItemGaBody, user_uid) == 0x12u,
              "auction registration user uid must stay at packet +0x12");
static_assert(offsetof(AuctionRegistItemGaBody, item) == 0x30u,
              "auction registration item must stay at packet +0x30");
static_assert(offsetof(AuctionRegistItemGaBody, emblem) == 0x65u,
              "auction registration emblem must stay at packet +0x65");
static_assert(offsetof(AuctionRegistItemGaBody, category) == 0x99u,
              "auction registration category must stay at packet +0x99");
static_assert(offsetof(AuctionRegistItemGaBody, account_name) == 0xa5u,
              "auction registration account must stay at packet +0xa5");
static_assert(sizeof(PCK_AUCTION_REGIST_ITEM_GA) == 0xc5u,
              "auction registration GA packet must stay 0xc5");

inline AuctionRegistItemGaBody& AuctionRegistItemGaBodyOf(
    PCK_AUCTION_REGIST_ITEM_GA& packet) {
    return packet;
}

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_REGIST_ITEM_RETURN_ABI_H_
