#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAILBOX_SYSTEM_MULTI_MAIL_PAYLOAD_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAILBOX_SYSTEM_MULTI_MAIL_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/inventory/inventory.h"

// IDA_EA: CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>
// @0x8558B3C and Stream::GetOutBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>
// @0x8453F08.  C owns this one DB-wire record; both the mail producer and the
// DB dispatcher use its physical Inven_Item values directly.  Keeping these
// objects intact is what preserves the GUID, amplify, random-option and
// separate-upgrade persistence path without a scalar compatibility facade.
#pragma pack(push, 1)
struct SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL {
    char title[30];                           // +0x000
    char letter_text[256];                    // +0x01E
    int32_t charac_no;                        // +0x11E
    int32_t sender_charac_no;                 // +0x122
    Inven_Item items[10];                     // +0x126
    uint32_t item_count;                      // +0x388
    uint32_t sender;                          // +0x38C
    uint32_t occurrence_days;                 // +0x390
    uint8_t unlimit_flag;                     // +0x394
    int32_t server_group;                     // +0x395
    int32_t text_length;                      // +0x399
    int32_t letter_id;                        // +0x39D
    int32_t expiry_time;                      // +0x3A1
    int32_t letter_stat;                      // +0x3A5
};
#pragma pack(pop)

static_assert(sizeof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL) == 0x3A9u,
              "system multi-mail DB payload must stay 0x3A9");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, title) == 0x000u,
              "system multi-mail title must stay at +0x000");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, letter_text) ==
                  0x01Eu,
              "system multi-mail letter text must stay at +0x01E");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, charac_no) ==
                  0x11Eu,
              "system multi-mail character must stay at +0x11E");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, sender_charac_no) ==
                  0x122u,
              "system multi-mail sender character must stay at +0x122");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, items) == 0x126u,
              "system multi-mail items must stay at +0x126");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, item_count) ==
                  0x388u,
              "system multi-mail count must stay at +0x388");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, sender) == 0x38Cu,
              "system multi-mail sender must stay at +0x38C");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, occurrence_days) ==
                  0x390u,
              "system multi-mail occurrence days must stay at +0x390");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, unlimit_flag) ==
                  0x394u,
              "system multi-mail unlimit flag must stay at +0x394");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, server_group) ==
                  0x395u,
              "system multi-mail group must stay at +0x395");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, text_length) ==
                  0x399u,
              "system multi-mail text length must stay at +0x399");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, letter_id) ==
                  0x39Du,
              "system multi-mail letter id must stay at +0x39D");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, expiry_time) ==
                  0x3A1u,
              "system multi-mail expiry must stay at +0x3A1");
static_assert(offsetof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL, letter_stat) ==
                  0x3A5u,
              "system multi-mail letter state must stay at +0x3A5");

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAILBOX_SYSTEM_MULTI_MAIL_PAYLOAD_H_
