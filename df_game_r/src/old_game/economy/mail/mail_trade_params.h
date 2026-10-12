#ifndef OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_PARAMS_H_
#define OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_PARAMS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x823109E in the companion source.


#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/economy/mail/mailbox_param_runtime.h"
#include "infra/network/packet/message_base.h"


// IDA unit: 10_mail_trade_auction_cash/MSG_OPEN_CERAPACKAGE.  C2 @0x822E3F4
// writes the target slot at +0x0e and clears exactly the 35 eight-byte
// choices at +0x14; the bytes at +0x0d and +0x11..+0x13 deliberately remain
// outside that constructor write set.  The request parser consumes only the
// named fields below.
#pragma pack(push, 1)
struct MSG_OPEN_CERAPACKAGE : public MSG_BASE {
    struct PackageChoice {
        uint32_t item_index;  // +0x00 in choice, PacketBuf::get_int
        uint8_t select;       // +0x04 in choice, PacketBuf::get_byte
        uint8_t reserved_05[3];
    };

    // IDA_EA: 0x822E3F4
    // IDA_NAME: _ZN20MSG_OPEN_CERAPACKAGEC2Ev
    // DEMANGLED: MSG_OPEN_CERAPACKAGE::MSG_OPEN_CERAPACKAGE(void)
    MSG_OPEN_CERAPACKAGE();

    uint8_t uninitialized_0d;  // +0x0d, untouched by the original C2
    uint16_t target_slot;       // +0x0e
    uint8_t item_count;         // +0x10
    uint8_t reserved_11[3];     // +0x11
    PackageChoice choices[35];  // +0x14
};
#pragma pack(pop)

static_assert(sizeof(MSG_OPEN_CERAPACKAGE::PackageChoice) == 0x08u,
              "Cera package choice must retain its wire ABI");
static_assert(sizeof(MSG_OPEN_CERAPACKAGE) == 0x12cu,
              "MSG_OPEN_CERAPACKAGE IDA size must stay 0x12c");
static_assert(offsetof(MSG_OPEN_CERAPACKAGE, target_slot) == 0x0eu &&
                  offsetof(MSG_OPEN_CERAPACKAGE, item_count) == 0x10u &&
                  offsetof(MSG_OPEN_CERAPACKAGE, choices) == 0x14u,
              "MSG_OPEN_CERAPACKAGE request offsets changed");

// IDA unit: 10_mail_trade_auction_cash/MSG_MAILBOX_SEND.
// Dispatcher_MailBox_Send::read @0x81CC3DA establishes every byte of this
// client packet.  It is a MSG_BASE-derived packed payload, not an opaque
// 0x145-byte buffer.
#pragma pack(push, 1)
struct MSG_MAILBOX_SEND : public MSG_BASE {
    uint32_t receiver_name_length;  // +0x0d
    char receiver_name[29];         // +0x11
    uint32_t money;                 // +0x2e
    uint32_t unknown_32;            // +0x32
    uint8_t inventory_type;         // +0x36
    uint16_t inventory_slot;        // +0x37
    uint32_t item_id;               // +0x39
    int32_t item_add_info;          // +0x3d
    uint32_t contents_length;       // +0x41
    char contents[256];             // +0x45
};
#pragma pack(pop)

static_assert(sizeof(MSG_MAILBOX_SEND) == 0x145u,
              "MSG_MAILBOX_SEND IDA size must stay 0x145");
static_assert(offsetof(MSG_MAILBOX_SEND, receiver_name_length) == 0x0du &&
                  offsetof(MSG_MAILBOX_SEND, receiver_name) == 0x11u &&
                  offsetof(MSG_MAILBOX_SEND, money) == 0x2eu &&
                  offsetof(MSG_MAILBOX_SEND, unknown_32) == 0x32u &&
                  offsetof(MSG_MAILBOX_SEND, inventory_type) == 0x36u &&
                  offsetof(MSG_MAILBOX_SEND, inventory_slot) == 0x37u &&
                  offsetof(MSG_MAILBOX_SEND, item_id) == 0x39u &&
                  offsetof(MSG_MAILBOX_SEND, item_add_info) == 0x3du &&
                  offsetof(MSG_MAILBOX_SEND, contents_length) == 0x41u &&
                  offsetof(MSG_MAILBOX_SEND, contents) == 0x45u,
              "MSG_MAILBOX_SEND Dispatcher_MailBox_Send::read offsets changed");

// IDA unit: 07_npc_shop_dialog/Dispatcher_BuyCeraShopItem and
// 99_misc_global/ItemVendingMachine.  The dispatcher reads the fixed
// 0x1cf-byte item records at +0x37; ProcessBuyRequestEx walks exactly 35.
#pragma pack(push, 1)
struct CeraShopBuyOption {
    uint32_t item_id;  // item record +0x07 + 5 * index
    uint8_t select;    // item record +0x0b + 5 * index
};

struct CeraShopBuyAdditionalItem {
    uint32_t item_id;       // item record +0xb7 + 8 * index
    uint16_t first_option;  // item record +0xbb + 8 * index
    uint16_t second_option; // item record +0xbd + 8 * index
};

struct CeraShopBuyItem {
    uint8_t select_type;       // +0x000
    uint8_t avatar_select;     // +0x001
    uint32_t goods_no;         // +0x002
    uint8_t option_count;      // +0x006
    CeraShopBuyOption options[35];             // +0x007
    uint8_t additional_item_count;             // +0x0b6
    CeraShopBuyAdditionalItem additional_items[35];  // +0x0b7
};

struct MSG_BUY_CERASHOP_ITEM : public MSG_BASE {
    uint16_t buy_type;            // +0x00d
    uint8_t gift;                 // +0x00f
    uint32_t receiver_name_length;// +0x010
    char receiver_name[30];       // +0x014
    // ProcessBuyRequestEx @0x8177E80 transfers this complete source word to
    // stCeraShopIPGRequest_t +0x38.  Packet parsing does not establish a
    // business-level interpretation, so preserve it as the named wire word.
    uint32_t post_recipient_wire_word; // +0x032
    uint8_t item_count;           // +0x036
    CeraShopBuyItem items[35];    // +0x037
};
#pragma pack(pop)

static_assert(sizeof(CeraShopBuyOption) == 0x05u,
              "CeraShopBuyOption IDA ABI size must stay 0x05");
static_assert(sizeof(CeraShopBuyAdditionalItem) == 0x08u,
              "CeraShopBuyAdditionalItem IDA ABI size must stay 0x08");
static_assert(sizeof(CeraShopBuyItem) == 0x1cfu,
              "CeraShopBuyItem IDA ABI size must stay 0x1cf");
static_assert(sizeof(MSG_BUY_CERASHOP_ITEM) == 0x3f84u,
              "MSG_BUY_CERASHOP_ITEM IDA size must stay 0x3f84");
static_assert(offsetof(MSG_BUY_CERASHOP_ITEM, buy_type) == 0x0du &&
                  offsetof(MSG_BUY_CERASHOP_ITEM, gift) == 0x0fu &&
                  offsetof(MSG_BUY_CERASHOP_ITEM, receiver_name_length) ==
                      0x10u &&
                  offsetof(MSG_BUY_CERASHOP_ITEM, receiver_name) == 0x14u &&
                  offsetof(MSG_BUY_CERASHOP_ITEM, item_count) == 0x36u &&
                  offsetof(MSG_BUY_CERASHOP_ITEM, items) == 0x37u,
              "MSG_BUY_CERASHOP_ITEM IDA offsets changed");

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_MultiMailBoxSend.
// CMD316 bypasses the normal template reader, but its direct parser builds
// this complete packed MSG_BASE payload before handing it to the mailbox DB
// receiver check and (when required) CUser's mail-certification cache.
#pragma pack(push, 1)
struct MultiMailboxSendItem {
    uint8_t inventory_type;  // +0x00, ENUM_ITEMSPACE on the wire
    uint16_t inventory_slot; // +0x01
    uint32_t item_id;        // +0x03
    int32_t item_add_info;   // +0x07
};

struct MSG_MULTI_MAILBOX_SEND : public MSG_BASE {
    uint32_t receiver_name_length;       // +0x0d
    char receiver_name[30];              // +0x11
    uint32_t money;                      // +0x2f
    uint8_t item_count;                  // +0x33
    MultiMailboxSendItem items[10];      // +0x34
    uint32_t contents_length;            // +0xa2
    char contents[256];                  // +0xa6
};
#pragma pack(pop)

static_assert(sizeof(MultiMailboxSendItem) == 0x0bu,
              "CMD316 item entry must retain its 11-byte wire ABI");
static_assert(sizeof(MSG_MULTI_MAILBOX_SEND) == 0x1a6u,
              "MSG_MULTI_MAILBOX_SEND IDA size must stay 0x1a6");
static_assert(offsetof(MSG_MULTI_MAILBOX_SEND, receiver_name_length) ==
                  0x0du &&
                  offsetof(MSG_MULTI_MAILBOX_SEND, receiver_name) == 0x11u &&
                  offsetof(MSG_MULTI_MAILBOX_SEND, money) == 0x2fu &&
                  offsetof(MSG_MULTI_MAILBOX_SEND, item_count) == 0x33u &&
                  offsetof(MSG_MULTI_MAILBOX_SEND, items) == 0x34u &&
                  offsetof(MSG_MULTI_MAILBOX_SEND, contents_length) == 0xa2u &&
                  offsetof(MSG_MULTI_MAILBOX_SEND, contents) == 0xa6u,
              "CMD316 direct parser offsets changed");

struct MSG_CHECK_MAIL_RECEIVER {
    enum ENUM_MAILBOX_CHECKTYPE {
        kMailboxSend = 0,
        kBuyCeraShopItem = 1,
        kMultiMailboxSend = 2
    };

    uint32_t check_type;

    // IDA's three set<> bodies copy a complete, discriminated mail request
    // immediately after check_type.  Keep all three typed variants in the
    // canonical owner so consumers never recreate their field offsets from a
    // byte buffer.  The user-provided no-op constructor deliberately leaves
    // the union uninitialized, matching the original aggregate object before
    // a set<> specialization copies its active request.
    union Payload {
        MSG_MAILBOX_SEND mailbox_send;
        MSG_BUY_CERASHOP_ITEM buy_cera_shop_item;
        MSG_MULTI_MAILBOX_SEND multi_mailbox_send;

        Payload() {}
    } payload;

    template <ENUM_MAILBOX_CHECKTYPE CheckType, typename Message>
    void set(const Message& message);
};

static_assert(offsetof(MSG_CHECK_MAIL_RECEIVER, check_type) == 0x00u &&
                  offsetof(MSG_CHECK_MAIL_RECEIVER, payload) == 0x04u &&
                  sizeof(MSG_CHECK_MAIL_RECEIVER) == 0x3f88u,
              "MSG_CHECK_MAIL_RECEIVER discriminated payload ABI changed");

template <>
void MSG_CHECK_MAIL_RECEIVER::set<MSG_CHECK_MAIL_RECEIVER::kMailboxSend,
                                  MSG_MAILBOX_SEND>(
    const MSG_MAILBOX_SEND& message);

template <>
void MSG_CHECK_MAIL_RECEIVER::set<MSG_CHECK_MAIL_RECEIVER::kBuyCeraShopItem,
                                  MSG_BUY_CERASHOP_ITEM>(
    const MSG_BUY_CERASHOP_ITEM& message);

template <>
void MSG_CHECK_MAIL_RECEIVER::set<MSG_CHECK_MAIL_RECEIVER::kMultiMailboxSend,
                                  MSG_MULTI_MAILBOX_SEND>(
    const MSG_MULTI_MAILBOX_SEND& message);

#endif  // OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_PARAMS_H_
