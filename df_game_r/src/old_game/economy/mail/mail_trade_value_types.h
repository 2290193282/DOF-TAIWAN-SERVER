#ifndef OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_VALUE_TYPES_H_
#define OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_VALUE_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/economy/mall/cash_billing_runtime.h"
#include "old_game/economy/mall/cash_service_runtime.h"
#include "old_game/economy/mall/cash_transaction_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/trade/trade_slot.h"
#include "old_data/PVF/event/action_send_mail.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/economy/mail/mailbox_public_abi.h"
#include "infra/concurrent/thread/mutex.h"

class CUser;
struct MSG_CHECK_MAIL_RECEIVER;
struct CTradeSpaceLayoutVerifier;

namespace ARAD_DATAS {

// IDA unit: 10_mail_trade_auction_cash/ARAD_DATAS/AradMailItemInfo.
struct AradMailItemInfo {
    AradMailItemInfo();

    int GetItemAttr() const;

    uint8_t seal_flag;            // +0x00
    uint8_t reserved_01[3];       // +0x01
    uint32_t item_count;          // +0x04
    uint8_t item_category;        // +0x08
    uint8_t reserved_09;          // +0x09
    uint16_t endurance;           // +0x0a
    uint32_t item_index;          // +0x0c
    uint32_t extend_info;         // +0x10
    uint8_t item_attr;            // +0x14
    uint8_t reserved_15[3];       // +0x15
};

static_assert(sizeof(AradMailItemInfo) == 0x18u,
              "AradMailItemInfo must retain the 0627 0x18 ABI");
static_assert(offsetof(AradMailItemInfo, item_count) == 0x04u &&
                  offsetof(AradMailItemInfo, item_category) == 0x08u &&
                  offsetof(AradMailItemInfo, endurance) == 0x0au &&
                  offsetof(AradMailItemInfo, item_index) == 0x0cu &&
                  offsetof(AradMailItemInfo, extend_info) == 0x10u &&
                  offsetof(AradMailItemInfo, item_attr) == 0x14u,
              "AradMailItemInfo physical members moved");

// IDA unit: 10_mail_trade_auction_cash/ARAD_DATAS/AradMailInfo.
struct AradMailInfo {
    AradMailInfo();

    uint8_t server_group;          // +0x00
    uint8_t reserved_01[3];       // +0x01
    int32_t gold;                 // +0x04
    int32_t occ_time;             // +0x08
    int32_t receiver_charac_no;   // +0x0c
    int32_t sender_charac_no;     // +0x10
    int32_t letter_text_length;   // +0x14
    uint32_t content_address;     // +0x18, old i386 char* value
    uint32_t title_address;       // +0x1c, old i386 char* value
    AradMailItemInfo item_info;
};

static_assert(sizeof(AradMailInfo) == 0x38u,
              "AradMailInfo must retain the 0627 0x38 ABI");
static_assert(offsetof(AradMailInfo, receiver_charac_no) == 0x0cu &&
                  offsetof(AradMailInfo, letter_text_length) == 0x14u &&
                  offsetof(AradMailInfo, content_address) == 0x18u &&
                  offsetof(AradMailInfo, title_address) == 0x1cu &&
                  offsetof(AradMailInfo, item_info) == 0x20u,
              "AradMailInfo physical members moved");

}  // namespace ARAD_DATAS

// C batch 025 migration marker: WongWork::CMailBox declaration is canonical in
// old_game/mail_trade_cash/mail_trade_runtime.h; legacy 10/ provides no second class.

// IDA unit: 10_mail_trade_auction_cash/CTradeSpace.
class CTradeSpace {
public:
    // Old IDA address: 0x82A416C.
    CTradeSpace();
    // Old IDA address: 0x82A41E0.
    ~CTradeSpace();

    // Old IDA address: 0x823438E.
    int get_index();
    // Old IDA address: 0x823439A.
    int is_empty();
    // Old IDA address: 0x82A41F4.
    CTradeSpace* SetIDX(int index);
    // IDA_EA: 0x85295F0.
    // IDA_NAME: _ZN11CTradeSpace11set_tradersEP5CUserS1_
    // DEMANGLED: CTradeSpace::set_traders(CUser *,CUser *)
    int set_traders(CUser* first, CUser* second);
    // IDA_EA: 0x85297F2.
    // IDA_NAME: _ZN11CTradeSpace5resetEv
    // DEMANGLED: CTradeSpace::reset(void)
    void reset();
    // IDA_EA: 0x852993C.
    // IDA_NAME: _ZN11CTradeSpace23check_trade_possibilityEv
    // DEMANGLED: CTradeSpace::check_trade_possibility(void)
    int check_trade_possibility();
    // IDA_EA: 0x852DBB8.
    // IDA_NAME: _ZN11CTradeSpace16checkCancelTradeEv
    // DEMANGLED: CTradeSpace::checkCancelTrade(void)
    int checkCancelTrade();
    // IDA_EA: 0x8530210.
    // IDA_NAME: _ZN11CTradeSpace10checkTradeEv
    // DEMANGLED: CTradeSpace::checkTrade(void)
    int checkTrade();
    // IDA_EA: 0x852E544.
    // IDA_NAME: _ZN11CTradeSpace22cancel_trade_for_chinaEP5CUser
    // DEMANGLED: CTradeSpace::cancel_trade_for_china(CUser *)
    int cancel_trade_for_china(CUser* disconnected_user);
    // IDA_EA: 0x852E3A8.
    // IDA_NAME: _ZN11CTradeSpace24send_lose_item_for_chinaEP5CUserR10Inven_Item
    // DEMANGLED: CTradeSpace::send_lose_item_for_china(CUser *,Inven_Item &)
    int send_lose_item_for_china(CUser* user, Inven_Item& item);
    // IDA_EA: 0x852EFFA.
    // IDA_NAME: _ZN11CTradeSpace12cancel_tradeEv
    // DEMANGLED: CTradeSpace::cancel_trade(void)
    int cancel_trade();
    // IDA_EA: 0x852F086.
    // IDA_NAME: _ZN11CTradeSpace19cancel_trade_by_disEP5CUser
    // DEMANGLED: CTradeSpace::cancel_trade_by_dis(CUser *)
    int cancel_trade_by_dis(CUser* disconnected_user);
    // Old IDA address: 0x85299C0.
    int get_empty_itemslot(int trader_index);
    // Old IDA address: 0x8529A18.
    int check_item_exist(int trader_index, int item_key);
    // IDA_EA: 0x8529AA2.
    // IDA_NAME: _ZN11CTradeSpace8add_itemEi9TradeSloti
    // DEMANGLED: CTradeSpace::add_item(int,TradeSlot,int)
    int add_item(int trader_index, TradeSlot slot, int add_info);
    // IDA_EA: 0x8529DCE.
    // IDA_NAME: _ZN11CTradeSpace11_IsTradableERK10Inven_Item
    // DEMANGLED: CTradeSpace::_IsTradable(Inven_Item const&)
    int _IsTradable(const Inven_Item& item);
    // IDA_EA: 0x8529E7E.
    // IDA_NAME: _ZN11CTradeSpace11regist_itemEP5CUseriii
    // DEMANGLED: CTradeSpace::regist_item(CUser *,int,int,int)
    int regist_item(CUser* owner,
                    int item_space,
                    int slot,
                    int item_count);
    // IDA_EA: 0x852BFCC.
    // IDA_NAME: _ZN11CTradeSpace11remove_itemEP5CUseriiii
    // DEMANGLED: CTradeSpace::remove_item(CUser *,int,int,int,int)
    int remove_item(CUser* owner,
                    int item_space,
                    int slot,
                    int trade_slot,
                    int item_count);
    // IDA_EA: 0x852F134.
    // IDA_NAME: _ZN11CTradeSpace18change_trade_stateEP5CUseri
    // DEMANGLED: CTradeSpace::change_trade_state(CUser *,int)
    int change_trade_state(CUser* owner, int state);
    // IDA_EA: 0x853087A.
    // IDA_NAME: _ZN11CTradeSpace13proceed_tradeEv
    // DEMANGLED: CTradeSpace::proceed_trade(void)
    int proceed_trade();
    // IDA_EA: 0x853084A.
    // IDA_NAME: _ZN11CTradeSpace26_SaveAvatarItemOwnerChangeEjjjh
    // DEMANGLED: CTradeSpace::_SaveAvatarItemOwnerChange(unsigned int,unsigned int,unsigned int,unsigned char)
    void _SaveAvatarItemOwnerChange(unsigned int avatar_ui_id,
                                    unsigned int old_charac_no,
                                    unsigned int new_charac_no,
                                    unsigned char slot);
    // Old IDA address: 0x8529A72.
    CUser* GetOppositeUser(CUser* user);
    // Old IDA address: 0x8529D48.
    int _CheckTrade4SameAvatar(const TradeSlot* slots,
                               const Inven_Item& item);
    // Old IDA address: 0x85320E2.
    void lock();
    // Old IDA address: 0x85320E8.
    void unlock();
    // Old IDA address: 0x85320EE.
    int IsLocked();
    // Old IDA address: 0x85320FA.
    CTradeSpace* _SetLock(bool locked);

private:
    friend struct CTradeSpaceLayoutVerifier;

    Mutex mutex_;
    int index_;
    uint8_t occupied_;
    uint8_t locked_;
    uint8_t pad_1e[2];
    CUser* traders_[2];
    int trade_states_[2];
    TradeSlot trade_slots_[2][27];
    int trade_gold_[2];
    // During the two-pass creature transfer, each entry counts creatures
    // already received by that trader.  IDA proceed_trade @0x853087A adds
    // this count to the recipient's pre-trade creature count before issuing
    // the ChangeOwner DB request.
    int received_creature_item_counts_[2];
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct CTradeSpaceLayoutVerifier {
    static_assert(offsetof(CTradeSpace, index_) == 0x18u,
                  "CTradeSpace pool index must remain at +0x18");
    static_assert(offsetof(CTradeSpace, occupied_) == 0x1cu,
                  "CTradeSpace occupied flag must remain at +0x1c");
    static_assert(offsetof(CTradeSpace, traders_) == 0x20u,
                  "CTradeSpace trader pointers must remain at +0x20");
    static_assert(offsetof(CTradeSpace, trade_states_) == 0x28u,
                  "CTradeSpace state pair must remain at +0x28");
    static_assert(offsetof(CTradeSpace, trade_slots_) == 0x30u,
                  "CTradeSpace escrow rows must remain at +0x30");
    static_assert(offsetof(CTradeSpace, trade_gold_) == 0xf60u,
                  "CTradeSpace escrow gold must remain at +0xf60");
    static_assert(
        offsetof(CTradeSpace, received_creature_item_counts_) == 0xf68u,
        "CTradeSpace received-creature counts must remain at +0xf68");
    static_assert(sizeof(CTradeSpace) == 0xf70u,
                  "CTradeSpace old i386 ABI must remain 0xf70 bytes");
};
#endif

// SR08 Commerce migration: CAuctionAveragePrice and Timer_CheckAuctionReady
// each have one canonical provider in old_game/mail_trade_cash/auction_runtime
// and commerce_timer_gm_runtime.cpp.

#endif  // OLDSRV_10_MAIL_TRADE_AUCTION_CASH_MAIL_TRADE_VALUE_TYPES_H_
