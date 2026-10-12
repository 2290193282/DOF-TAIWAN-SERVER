#ifndef OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_PACKETS_H_
#define OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_PACKETS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x847164C in the companion source.


#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mall/auction_public_values.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "infra/network/packet/PacketHeader.h"


#pragma pack(push, 1)

// IDA unit: 10_mail_trade_auction_cash/Packet_Monitor_Notify_New_Mail.
struct Packet_Monitor_Notify_New_Mail : public PacketHeader {
    Packet_Monitor_Notify_New_Mail();

    // IDA 0x8471072 writes the recipient at +0x0a on the outbound request.
    // The monitor resolves that character to a live game-user UID and writes
    // it at +0x0e before relaying the same frame back to the game server;
    // official CDispatchServerPacket @0x8473edb consumes that routing UID.
    uint32_t receiver_charac_no;             // +0x0a
    int32_t routing_user_uid;                 // +0x0e, inbound relay field
};

static_assert(sizeof(Packet_Monitor_Notify_New_Mail) == 0x12u,
              "new-mail monitor packet must retain its 0x12-byte ABI");
static_assert(offsetof(Packet_Monitor_Notify_New_Mail, receiver_charac_no) ==
                   0x0au &&
                  offsetof(Packet_Monitor_Notify_New_Mail,
                           routing_user_uid) == 0x0eu,
              "new-mail monitor recipient/routing offsets must stay +0x0a/+0x0e");

// Auction::SendMessageToMonitor in the official auction service constructs
// this PacketHeader(3100, 0x26) frame and sends all 0x26 bytes to the monitor.
// The monitor fills routing_user_uid before relaying the same frame to the
// game server.  Official CDispatchServerPacket then queues that complete
// frame as Inter_NotifyAuctionMail (signal 239).
//
// Sender evidence (df_auction_r, i386):
//   Packet_Monitor_Notify_Auction_Mail::C2 @ 0x0805C55E
//   Auction::SendMessageToMonitor          @ 0x0805BAC2
struct Packet_Monitor_Notify_Auction_Mail : public PacketHeader {
    uint32_t receiver_charac_no;  // +0x0a
    int32_t routing_user_uid;     // +0x0e, monitor-owned inbound route
    uint8_t mail_item_type;       // +0x12
    uint32_t mail_item_id;        // +0x13, persisted postal row/item id
    uint8_t notification_type;    // +0x17, auction payment/notice type
    RandomOption random_option;   // +0x18
};

static_assert(sizeof(Packet_Monitor_Notify_Auction_Mail) == 0x26u,
              "auction-mail monitor packet must retain its 0x26-byte ABI");
static_assert(
    offsetof(Packet_Monitor_Notify_Auction_Mail, receiver_charac_no) ==
            0x0au &&
        offsetof(Packet_Monitor_Notify_Auction_Mail, routing_user_uid) ==
            0x0eu &&
        offsetof(Packet_Monitor_Notify_Auction_Mail, mail_item_type) ==
            0x12u &&
        offsetof(Packet_Monitor_Notify_Auction_Mail, mail_item_id) == 0x13u &&
        offsetof(Packet_Monitor_Notify_Auction_Mail, notification_type) ==
            0x17u &&
        offsetof(Packet_Monitor_Notify_Auction_Mail, random_option) == 0x18u,
    "auction-mail monitor fields moved");

// IDA unit: 10_mail_trade_auction_cash/Packet_Monitor_Send_Guild_Mail.
struct Packet_Monitor_Send_Guild_Mail : public PacketHeader {
    Packet_Monitor_Send_Guild_Mail();

    uint8_t raw_0a[0x08];
    uint8_t body_12[0x100];
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_CHECK_AUCTION_READY_GA.
struct PCK_AUCTION_CHECK_AUCTION_READY_GA : public tagPacketHeader {
    PCK_AUCTION_CHECK_AUCTION_READY_GA();

    uint8_t body_06[0x0c];
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_CHECK_AUCTION_READY_GP.
struct PCK_AUCTION_CHECK_AUCTION_READY_GP : public tagPacketHeader {
    PCK_AUCTION_CHECK_AUCTION_READY_GP();

    uint8_t body_06[0x0c];
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_REGIST_GA.
struct PCK_AUCTION_REGIST_GA : public tagPacketHeader {
    PCK_AUCTION_REGIST_GA();

    uint8_t body_06[0x10];
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_REGIST_GP.
struct PCK_AUCTION_REGIST_GP : public tagPacketHeader {
    PCK_AUCTION_REGIST_GP();

    uint8_t body_06[0x10];
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_BIDDING_GA.
// Dispatcher_AuctionBidding proves the 0x33-byte GA request has the native
// nsl header followed by identity, character name, bid money, and an opaque
// eight-byte auction key.
struct AuctionBiddingKey {
    char bytes[0x08];
};

static_assert(sizeof(AuctionBiddingKey) == 0x08u,
              "auction bidding key must stay eight bytes");

struct PCK_AUCTION_BIDDING_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_BIDDING_GA();

    uint8_t reserved_06[0x0c];  // +0x06
    int32_t user_uid;           // +0x12
    int32_t charac_no;          // +0x16
    char charac_name[13];       // +0x1a
    int32_t bid_money;          // +0x27
    AuctionBiddingKey key;      // +0x2b
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_BIDDING_GP.
// The point-auction acknowledgement mirrors the identity/key portion of the
// GA packet.  Inter_OwnerisVIP @ 0x84E7C90 fills every named field below
// before CCeraAuctionServerProxy forwards it to the point-auction service.
struct PCK_AUCTION_BIDDING_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_BIDDING_GP();

    uint8_t reserved_06[0x0c];  // +0x06
    int32_t user_uid;           // +0x12
    int32_t charac_no;          // +0x16
    char charac_name[13];       // +0x1a
    int32_t bid_cera;           // +0x27
    AuctionBiddingKey key;      // +0x2b
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_REGIST_CANCEL_GA.
// The protocol carries this opaque, fixed-width auction registration key;
// neither the dispatcher nor the old proxy interprets its eight bytes.
struct AuctionRegistrationKey {
    char bytes[0x08];
};

static_assert(sizeof(AuctionRegistrationKey) == 0x08u,
              "auction registration key must stay eight bytes");

struct PCK_AUCTION_REGIST_CANCEL_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_REGIST_CANCEL_GA();

    // +0x06..+0x11 are zeroed by C2 and are not read by the sender.
    uint8_t reserved_06[0x0c];
    int32_t user_uid;                       // +0x12, CUser::GetUID
    int32_t charac_no;                      // +0x16, getCurCharacNo
    AuctionRegistrationKey registration_key;  // +0x1a
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_REGIST_CANCEL_GP.
struct PCK_AUCTION_REGIST_CANCEL_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_REGIST_CANCEL_GP();

    // +0x06..+0x11 are zeroed by C2 and are not read by the sender.
    uint8_t reserved_06[0x0c];
    int32_t user_uid;                       // +0x12, CUser::GetUID
    int32_t charac_no;                      // +0x16, getCurCharacNo
    AuctionRegistrationKey registration_key;  // +0x1a
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_ASK_AVERAGE_PRICE_GP.
struct PCK_AUCTION_ASK_AVERAGE_PRICE_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_ASK_AVERAGE_PRICE_GP();

    uint8_t reserved_06[0x0c];
    int32_t user_uid;
    int32_t charac_no;
    uint32_t item_index;
    uint8_t field_1e;
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_ASK_AVERAGE_PRICE_GA.
struct PCK_AUCTION_ASK_AVERAGE_PRICE_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_ASK_AVERAGE_PRICE_GA();

    uint8_t reserved_06[0x0c];
    int32_t user_uid;
    int32_t charac_no;
    uint32_t item_index;
    uint8_t field_1e;
    uint8_t field_1f;
    uint8_t field_20;
    uint8_t field_21;
    ROI_Category category;
    uint8_t field_2e;
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GP.
struct PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GP();

    uint8_t reserved_06[0x0c];  // +0x06
    int32_t user_uid;           // +0x12
    int32_t charac_no;          // +0x16
    uint8_t item_space;         // +0x1a
    uint16_t item_slot;         // +0x1b
    uint32_t field_1d;          // +0x1d
    int32_t field_21;           // +0x21
    int32_t field_25;           // +0x25
    int32_t field_29;           // +0x29
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GA.
struct PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GA();

    uint8_t reserved_06[0x0c];  // +0x06
    int32_t user_uid;           // +0x12
    int32_t charac_no;          // +0x16
    uint8_t item_space;         // +0x1a
    uint16_t item_slot;         // +0x1b
    uint32_t item_index;        // +0x1d
    int32_t field_21;           // +0x21
    int32_t field_25;           // +0x25
    int32_t field_29;           // +0x29
    int32_t field_2d;           // +0x2d
    ROI_Category category;      // +0x31
};

static_assert(sizeof(PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GP) == 0x2du,
              "PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GP IDA ABI size must stay 0x2d");
static_assert(sizeof(PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GA) == 0x3du,
              "PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GA IDA ABI size must stay 0x3d");

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_REGIST_ITEM_GA.
struct PCK_AUCTION_REGIST_ITEM_GA : public tagPacketHeader {
    PCK_AUCTION_REGIST_ITEM_GA();

    uint8_t reserved_06[0x0c];              // +0x06
    int32_t user_uid;                       // +0x12
    int32_t charac_no;                      // +0x16
    char charac_name[13];                   // +0x1a
    uint8_t is_affected_premium;            // +0x27
    int32_t field_28;                       // +0x28
    int32_t field_2c;                       // +0x2c
    DnfItemInfo item;                       // +0x30
    stAvatarEmblemInfo_t emblem;            // +0x65
    stAvatarExpansionInfo_t expansion;      // +0x83
    int32_t curr_charac_black_count;        // +0x87
    uint8_t reserved_8b[0x0a];              // +0x8b
    int32_t field_95;                       // +0x95
    ROI_Category category;                  // +0x99
    char account_name[32];                  // +0xa5
};

static_assert(offsetof(PCK_AUCTION_REGIST_ITEM_GA, item) == 0x30u,
              "auction registration item info must stay at +0x30");
static_assert(offsetof(PCK_AUCTION_REGIST_ITEM_GA, category) == 0x99u,
              "auction registration category must stay at +0x99");
static_assert(sizeof(PCK_AUCTION_REGIST_ITEM_GA) == 0xc5u,
              "auction registration packet must stay 0xc5 bytes");

// IDA unit: 10_mail_trade_auction_cash/TSearchByItemId_.
// Dispatcher_AuctionSearchByItemKey proves the body starts at packet +0x1a,
// embeds ROI_Category at body +0x09, and writes at most twenty 32-bit keys
// starting at body +0x17.
typedef TSearchByItemId_ AuctionSearchByItemKeyBody;

static_assert(sizeof(AuctionSearchByItemKeyBody) == 0x67u,
              "auction item-key search body must stay 0x67 bytes");

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_SEARCH_BY_ITEMKEY_GA.
struct PCK_AUCTION_SEARCH_BY_ITEMKEY_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_SEARCH_BY_ITEMKEY_GA();

    uint8_t reserved_06[0x0c];
    int32_t user_uid;
    int32_t charac_no;
    AuctionSearchByItemKeyBody search;
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_SEARCH_BY_ITEMKEY_GP.
struct PCK_AUCTION_SEARCH_BY_ITEMKEY_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_SEARCH_BY_ITEMKEY_GP();

    uint8_t reserved_06[0x0c];
    int32_t user_uid;
    int32_t charac_no;
    AuctionSearchByItemKeyBody search;
};

// IDA unit: 10_mail_trade_auction_cash/TSearchByCategory_.
// The offsets are proven by Dispatcher_AuctionSearchByNoItmeKey's GA stack
// locals: +0x1a is this body, and ROI_Category starts at body +0x0b.
typedef TSearchByCategory_ AuctionSearchByNoItemKeyBody;

static_assert(sizeof(AuctionSearchByNoItemKeyBody) == 0x19u,
              "auction no-item-key search body must stay 0x19 bytes");

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_SEARCH_BY_NOITEMKEY_GA.
struct PCK_AUCTION_SEARCH_BY_NOITEMKEY_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_SEARCH_BY_NOITEMKEY_GA();

    uint8_t reserved_06[0x0c];
    int32_t user_uid;
    int32_t charac_no;
    AuctionSearchByNoItemKeyBody search;
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_SEARCH_BY_NOITEMKEY_GP.
struct PCK_AUCTION_SEARCH_BY_NOITEMKEY_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_SEARCH_BY_NOITEMKEY_GP();

    uint8_t reserved_06[0x0c];
    int32_t user_uid;
    int32_t charac_no;
    AuctionSearchByNoItemKeyBody search;
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_MY_REGISTED_ITEM_INFO_GA.
struct PCK_AUCTION_MY_REGISTED_ITEM_INFO_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_MY_REGISTED_ITEM_INFO_GA();

    // +0x06..+0x11 are zeroed by C2 and are not read by the sender.
    uint8_t reserved_06[0x0c];
    int32_t user_uid;    // +0x12, CUser::GetUID
    int32_t charac_no;   // +0x16, CUserCharacInfo::getCurCharacNo
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_MY_REGISTED_ITEM_INFO_GP.
struct PCK_AUCTION_MY_REGISTED_ITEM_INFO_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_MY_REGISTED_ITEM_INFO_GP();

    // +0x06..+0x11 are zeroed by C2 and are not read by the sender.
    uint8_t reserved_06[0x0c];
    int32_t user_uid;    // +0x12, CUser::GetUID
    int32_t charac_no;   // +0x16, CUserCharacInfo::getCurCharacNo
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_MY_BIDDING_INFO_GA.
struct PCK_AUCTION_MY_BIDDING_INFO_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_MY_BIDDING_INFO_GA();

    // +0x06..+0x11 are zeroed by C2 and are not read by the sender.
    uint8_t reserved_06[0x0c];
    int32_t user_uid;    // +0x12, CUser::GetUID
    int32_t charac_no;   // +0x16, CUserCharacInfo::getCurCharacNo
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_MY_AUCTION_HISTORY_GA.
struct PCK_AUCTION_MY_AUCTION_HISTORY_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_MY_AUCTION_HISTORY_GA();

    // +0x06..+0x11 are zeroed by C2 and are not read by the sender.
    uint8_t reserved_06[0x0c];
    int32_t user_uid;    // +0x12, CUser::GetUID
    int32_t charac_no;   // +0x16, CUserCharacInfo::getCurCharacNo
    int32_t account_id;  // +0x1a, CUser::get_acc_id
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_ASK_OWNER_IS_VIP_GP.
// CMD188's Cera branch sends this 0x37-byte packet rather than the otherwise
// similarly sized GP bidding packet.
struct PCK_AUCTION_ASK_OWNER_IS_VIP_GP : public nsl::PACKET_HEADER {
    PCK_AUCTION_ASK_OWNER_IS_VIP_GP();

    uint8_t reserved_06[0x0c];  // +0x06
    int32_t user_uid;           // +0x12
    AuctionBiddingKey key;      // +0x16
    int32_t charac_no;          // +0x1e
    char charac_name[13];       // +0x22
    uint32_t bid_cera;          // +0x2f
    int32_t field_33;           // +0x33
};

static_assert(sizeof(PCK_AUCTION_BIDDING_GA) == 0x33u,
              "PCK_AUCTION_BIDDING_GA IDA ABI size must stay 0x33");
static_assert(sizeof(PCK_AUCTION_ASK_OWNER_IS_VIP_GP) == 0x37u,
              "PCK_AUCTION_ASK_OWNER_IS_VIP_GP IDA ABI size must stay 0x37");

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_OPEN_PRIVATE_STORE_GA.
struct PCK_AUCTION_OPEN_PRIVATE_STORE_GA : public tagPacketHeader {
    PCK_AUCTION_OPEN_PRIVATE_STORE_GA();

    uint8_t reserved_06[0x0c];  // +0x06
    int32_t account_id;         // +0x12
    uint8_t reserved_16[0x04];  // +0x16
    int32_t charac_no[36];      // +0x1a
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_CLOSE_PRIVATE_STORE_GA.
struct PCK_AUCTION_CLOSE_PRIVATE_STORE_GA : public tagPacketHeader {
    PCK_AUCTION_CLOSE_PRIVATE_STORE_GA();

    int32_t account_id;       // +0x06
    uint8_t reserved_0a[0x0c];
};

// IDA unit: 10_mail_trade_auction_cash/PCK_AUCTION_BUY_ITEM_APIECE_GA.
// The native sender initializes an nsl header and then fills the money, opaque
// eight-byte auction key, and trailing scalar in their proven wire positions.
struct AuctionBuyItemApieceKey {
    char bytes[0x08];
};

static_assert(sizeof(AuctionBuyItemApieceKey) == 0x08u,
              "auction buy-apiece key must stay eight bytes");

struct PCK_AUCTION_BUY_ITEM_APIECE_GA : public nsl::PACKET_HEADER {
    PCK_AUCTION_BUY_ITEM_APIECE_GA();

    uint8_t reserved_06[0x0c];  // +0x06
    int32_t user_uid;           // +0x12
    int32_t charac_no;          // +0x16
    int32_t purchase_money;     // +0x1a
    AuctionBuyItemApieceKey key;  // +0x1e
    int32_t field_26;           // +0x26
};

static_assert(sizeof(PCK_AUCTION_BUY_ITEM_APIECE_GA) == 0x2au,
              "PCK_AUCTION_BUY_ITEM_APIECE_GA IDA ABI size must stay 0x2a");

#pragma pack(pop)


#endif  // OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_PACKETS_H_
