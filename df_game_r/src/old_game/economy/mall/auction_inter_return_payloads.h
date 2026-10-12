#ifndef OLDSRV_OLD_GAME_ECONOMY_MALL_AUCTION_INTER_RETURN_PAYLOADS_H_
#define OLDSRV_OLD_GAME_ECONOMY_MALL_AUCTION_INTER_RETURN_PAYLOADS_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mail/mail_trade_packets.h"

// Fixed InterDispatcher response bodies consumed by the 0627 auction
// handlers.  These are packed DB/auction-service wire values.  Naming their
// physical fields keeps the handlers from rebuilding the contracts with
// payload[index] and payload+offset arithmetic.
#pragma pack(push, 1)

// Inter239 consumes the complete monitor frame rather than a body stripped
// from the PacketHeader.  Keep the historical response name as an alias so
// the dispatcher describes its role while sharing one canonical wire type.
typedef Packet_Monitor_Notify_Auction_Mail AuctionNotifyMailPayload;

struct AuctionResultAskPricePayload {
    uint32_t payment_type;            // +0x00
    int32_t charac_no;                // +0x04
    int32_t representative_price;     // +0x08
    int32_t first_price_values[3];    // +0x0c
    int32_t second_price_values[3];   // +0x18
};

struct AuctionSimpleResultPayload {
    uint32_t payment_type;            // +0x00
    int32_t charac_no;                // +0x04
    uint8_t result;                   // +0x08
    uint8_t error_code;               // +0x09
};

struct AuctionResultBiddingPayload {
    uint32_t payment_type;            // +0x00
    int32_t charac_no;                // +0x04
    int32_t requested_money;          // +0x08
    uint8_t result;                   // +0x0c
    uint8_t error_code;               // +0x0d
};

struct AuctionVariableListPrefix {
    int32_t charac_no;                // +0x00
};

#pragma pack(pop)

static_assert(offsetof(AuctionNotifyMailPayload, receiver_charac_no) == 0x0au &&
                  offsetof(AuctionNotifyMailPayload, mail_item_type) == 0x12u &&
                  offsetof(AuctionNotifyMailPayload, mail_item_id) == 0x13u &&
                  offsetof(AuctionNotifyMailPayload, notification_type) ==
                      0x17u &&
                  offsetof(AuctionNotifyMailPayload, random_option) == 0x18u,
              "auction new-mail response fields moved");
static_assert(sizeof(AuctionResultAskPricePayload) == 0x24u,
              "auction ask-price response must stay 0x24 bytes");
static_assert(sizeof(AuctionSimpleResultPayload) == 0x0au,
              "auction simple response must stay 0x0a bytes");
static_assert(sizeof(AuctionResultBiddingPayload) == 0x0eu,
              "auction bidding response must stay 0x0e bytes");
static_assert(sizeof(AuctionVariableListPrefix) == 0x04u,
              "auction list response prefix must stay four bytes");

#endif  // OLDSRV_OLD_GAME_ECONOMY_MALL_AUCTION_INTER_RETURN_PAYLOADS_H_
