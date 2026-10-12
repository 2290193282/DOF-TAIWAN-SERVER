#ifndef OLDSRV_GLOBAL_SECU_GOLD_CONTROL_ABI_H_
#define OLDSRV_GLOBAL_SECU_GOLD_CONTROL_ABI_H_

#include <stddef.h>
#include <stdint.h>

class CUser;
enum eMoneyAddReason : int;
enum eMoneySubReason : int;

// IDA unit: user_gold_info.  The twelve scalar fields are the six qword
// counters and six word counters used by the gold-range rollover routines.
// This replaces the former source-local raw[0x3c] surrogate.
struct user_gold_info {
    user_gold_info();

    uint64_t add_money_amount_;          // +0x00
    uint64_t add_import_money_amount_;   // +0x08
    uint64_t sub_money_amount_;          // +0x10
    uint64_t sub_import_money_amount_;   // +0x18
    uint64_t add_auction_money_amount_;  // +0x20
    uint64_t sub_auction_money_amount_;  // +0x28
    uint16_t add_auction_money_count_;   // +0x30
    uint16_t sub_auction_money_count_;   // +0x32
    uint16_t add_money_count_;           // +0x34
    uint16_t add_import_money_count_;    // +0x36
    uint16_t sub_money_count_;           // +0x38
    uint16_t sub_import_money_count_;    // +0x3a

    user_gold_info* reset();
    user_gold_info* toCopy(user_gold_info& destination);
};

// Expanded CUser embeds Secu_GoldControl at +0x8ecd0 and constructs it before
// Secu_HackLogCheck at +0x8edb8.
struct Secu_GoldControl {
    Secu_GoldControl();
    ~Secu_GoldControl();

    // IDA_EA: 0x8287230
    // IDA_NAME: _ZN16Secu_GoldControl4InitEP5CUser
    // DEMANGLED: Secu_GoldControl::Init(CUser *)
    void* Init(CUser* user);

    // IDA_EA: 0x82872A0
    // IDA_NAME: _ZN16Secu_GoldControl7SetInfoER14user_gold_info
    // DEMANGLED: Secu_GoldControl::SetInfo(user_gold_info &)
    user_gold_info* SetInfo(user_gold_info& info);

    // IDA_EA: 0x8287552
    // IDA_NAME: _ZN16Secu_GoldControl10UseFatigueEi
    // DEMANGLED: Secu_GoldControl::UseFatigue(int)
    Secu_GoldControl* UseFatigue(int fatigue);

    // IDA_EA: 0x8287EE0
    // IDA_NAME: _ZN16Secu_GoldControl15SetMailCharNameEPKc
    // DEMANGLED: Secu_GoldControl::SetMailCharName(char const*)
    char* SetMailCharName(const char* sender_name);

    // IDA_EA: 0x8287E56 / 0x8287E68
    // IDA_NAME: _ZN16Secu_GoldControl11MailSameMidEv /
    //           _ZN16Secu_GoldControl12MailCompleteEb
    Secu_GoldControl* MailSameMid();
    Secu_GoldControl* MailComplete(bool subtract_mail_gold);

    int SavetoDB(bool save_current, bool save_previous, bool reset_range);
    int resetRangeMoney(user_gold_info& source, user_gold_info& destination);
    void AddGold(unsigned int money, eMoneyAddReason reason);
    int AddMoney(unsigned int money);
    int AddImportMoney(unsigned int money);
    void AddAuctionMoney(unsigned int money);
    int SubGold(unsigned int money, eMoneySubReason reason);
    void SubAuctionMoney(unsigned int money);
    int SubMoney(unsigned int money);
    int SubImportMoney(unsigned int money);
    int TradeComplete(int own_gold, int opposite_gold);
    Secu_GoldControl* CheckDate();
    int CheckMoneyRange();
    Secu_GoldControl* MailGold(unsigned int money, bool is_sub);

    CUser* owner_;                         // +0x00
    user_gold_info range_info_;            // +0x04
    user_gold_info range_daily_info_;      // +0x40
    user_gold_info range_saved_info_;      // +0x7c
    uint32_t used_fatigue_;                 // +0xb8
    uint32_t previous_used_fatigue_;        // +0xbc
    uint32_t date_flag_;                    // +0xc0
    uint32_t mail_gold_;                    // +0xc4
    char mail_charac_name_[30];             // +0xc8
    uint8_t pad_e6_e7_[2];
};

#if defined(__i386__)
static_assert(offsetof(user_gold_info, add_money_amount_) == 0x00u,
              "user_gold_info add-money amount must stay at IDA +0x00");
static_assert(offsetof(user_gold_info, add_import_money_amount_) == 0x08u,
              "user_gold_info add-import amount must stay at IDA +0x08");
static_assert(offsetof(user_gold_info, sub_money_amount_) == 0x10u,
              "user_gold_info sub-money amount must stay at IDA +0x10");
static_assert(offsetof(user_gold_info, sub_import_money_amount_) == 0x18u,
              "user_gold_info sub-import amount must stay at IDA +0x18");
static_assert(offsetof(user_gold_info, add_auction_money_amount_) == 0x20u,
              "user_gold_info add-auction amount must stay at IDA +0x20");
static_assert(offsetof(user_gold_info, sub_auction_money_amount_) == 0x28u,
              "user_gold_info sub-auction amount must stay at IDA +0x28");
static_assert(offsetof(user_gold_info, add_auction_money_count_) == 0x30u,
              "user_gold_info add-auction count must stay at IDA +0x30");
static_assert(offsetof(user_gold_info, sub_auction_money_count_) == 0x32u,
              "user_gold_info sub-auction count must stay at IDA +0x32");
static_assert(offsetof(user_gold_info, add_money_count_) == 0x34u,
              "user_gold_info add-money count must stay at IDA +0x34");
static_assert(offsetof(user_gold_info, add_import_money_count_) == 0x36u,
              "user_gold_info add-import count must stay at IDA +0x36");
static_assert(offsetof(user_gold_info, sub_money_count_) == 0x38u,
              "user_gold_info sub-money count must stay at IDA +0x38");
static_assert(offsetof(user_gold_info, sub_import_money_count_) == 0x3au,
              "user_gold_info sub-import count must stay at IDA +0x3a");
static_assert(sizeof(user_gold_info) == 0x3cu,
              "user_gold_info must preserve the old i386 extent");
static_assert(offsetof(Secu_GoldControl, owner_) == 0x00u,
              "Secu_GoldControl owner must stay at IDA +0x00");
static_assert(offsetof(Secu_GoldControl, range_info_) == 0x04u,
              "Secu_GoldControl current range must stay at IDA +0x04");
static_assert(offsetof(Secu_GoldControl, range_daily_info_) == 0x40u,
              "Secu_GoldControl daily range must stay at IDA +0x40");
static_assert(offsetof(Secu_GoldControl, range_saved_info_) == 0x7cu,
              "Secu_GoldControl saved range must stay at IDA +0x7c");
static_assert(offsetof(Secu_GoldControl, used_fatigue_) == 0xb8u,
              "Secu_GoldControl fatigue must stay at IDA +0xb8");
static_assert(offsetof(Secu_GoldControl, previous_used_fatigue_) == 0xbcu,
              "Secu_GoldControl previous fatigue must stay at IDA +0xbc");
static_assert(offsetof(Secu_GoldControl, date_flag_) == 0xc0u,
              "Secu_GoldControl date flag must stay at IDA +0xc0");
static_assert(offsetof(Secu_GoldControl, mail_gold_) == 0xc4u,
              "Secu_GoldControl mail gold must stay at IDA +0xc4");
static_assert(offsetof(Secu_GoldControl, mail_charac_name_) == 0xc8u,
              "Secu_GoldControl mail name must stay at IDA +0xc8");
static_assert(sizeof(Secu_GoldControl) == 0xe8u,
              "Secu_GoldControl must preserve the old i386 extent");
#endif


#endif  // OLDSRV_GLOBAL_SECU_GOLD_CONTROL_ABI_H_
