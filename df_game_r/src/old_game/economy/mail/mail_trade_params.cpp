#include "old_game/economy/mail/mail_trade_params.h"

#include <cstring>

typedef char MSG_MAILBOX_SEND_Size_Must_Be_0x145[
    (sizeof(MSG_MAILBOX_SEND) == 0x145) ? 1 : -1];
typedef char MSG_BUY_CERASHOP_ITEM_Size_Must_Be_0x3f84[
    (sizeof(MSG_BUY_CERASHOP_ITEM) == 0x3f84) ? 1 : -1];
typedef char MSG_MULTI_MAILBOX_SEND_Size_Must_Be_0x1a6[
    (sizeof(MSG_MULTI_MAILBOX_SEND) == 0x1a6) ? 1 : -1];
typedef char MSG_CHECK_MAIL_RECEIVER_Size_Must_Be_0x3f88[
    (sizeof(MSG_CHECK_MAIL_RECEIVER) == 0x3f88) ? 1 : -1];
typedef char stAuctionLimitItemParam_t_Size_Must_Be_0x0c[
    (sizeof(stAuctionLimitItemParam_t) == 0x0c) ? 1 : -1];

// IDA_EA: 0x822E3F4
// IDA_NAME: _ZN20MSG_OPEN_CERAPACKAGEC2Ev
// DEMANGLED: MSG_OPEN_CERAPACKAGE::MSG_OPEN_CERAPACKAGE(void)
MSG_OPEN_CERAPACKAGE::MSG_OPEN_CERAPACKAGE()
    : MSG_BASE() {
    // The old C2 intentionally does not initialize the header-adjacent pad or
    // the parser-only item count.  It writes only the target slot and the
    // complete 35-entry choice range.
    target_slot = 0;
    std::memset(choices, 0, sizeof(choices));
}

// IDA_EA: 0x8236E06
// IDA_NAME: _ZN23MSG_CHECK_MAIL_RECEIVER3setILNS_22ENUM_MAILBOX_CHECKTYPEE0E16MSG_MAILBOX_SENDEEvRKT0_
// DEMANGLED: void MSG_CHECK_MAIL_RECEIVER::set<(MSG_CHECK_MAIL_RECEIVER::ENUM_MAILBOX_CHECKTYPE)0,MSG_MAILBOX_SEND>(MSG_MAILBOX_SEND const&)
template <>
void MSG_CHECK_MAIL_RECEIVER::set<MSG_CHECK_MAIL_RECEIVER::kMailboxSend,
                                  MSG_MAILBOX_SEND>(
    const MSG_MAILBOX_SEND& message) {
    check_type = kMailboxSend;
    std::memcpy(&payload.mailbox_send, &message, sizeof(message));
}

// IDA_EA: 0x82378EE
// IDA_NAME: _ZN23MSG_CHECK_MAIL_RECEIVER3setILNS_22ENUM_MAILBOX_CHECKTYPEE1E21MSG_BUY_CERASHOP_ITEMEEvRKT0_
// DEMANGLED: void MSG_CHECK_MAIL_RECEIVER::set<(MSG_CHECK_MAIL_RECEIVER::ENUM_MAILBOX_CHECKTYPE)1,MSG_BUY_CERASHOP_ITEM>(MSG_BUY_CERASHOP_ITEM const&)
template <>
void MSG_CHECK_MAIL_RECEIVER::set<MSG_CHECK_MAIL_RECEIVER::kBuyCeraShopItem,
                                  MSG_BUY_CERASHOP_ITEM>(
    const MSG_BUY_CERASHOP_ITEM& message) {
    check_type = kBuyCeraShopItem;
    std::memcpy(&payload.buy_cera_shop_item, &message, sizeof(message));
}

// IDA_EA: 0x8238086
// IDA_NAME: _ZN23MSG_CHECK_MAIL_RECEIVER3setILNS_22ENUM_MAILBOX_CHECKTYPEE2E22MSG_MULTI_MAILBOX_SENDEEvRKT0_
// DEMANGLED: void MSG_CHECK_MAIL_RECEIVER::set<(MSG_CHECK_MAIL_RECEIVER::ENUM_MAILBOX_CHECKTYPE)2,MSG_MULTI_MAILBOX_SEND>(MSG_MULTI_MAILBOX_SEND const&)
template <>
void MSG_CHECK_MAIL_RECEIVER::set<MSG_CHECK_MAIL_RECEIVER::kMultiMailboxSend,
                                  MSG_MULTI_MAILBOX_SEND>(
    const MSG_MULTI_MAILBOX_SEND& message) {
    check_type = kMultiMailboxSend;
    std::memcpy(&payload.multi_mailbox_send, &message, sizeof(message));
}
