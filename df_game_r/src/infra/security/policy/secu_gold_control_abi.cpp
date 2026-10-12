#include "infra/security/policy/secu_gold_control_abi.h"

#include "old_data/db/character/db_check_charac_name.h"
#include "infra/security/policy/hades_public_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/transport/network_endpoint.h"

#include <stdint.h>
#include <string.h>

namespace {

const uint64_t kMoneyRangeLimit = 0xFFFFFFFFull;

void rollover_amount(uint64_t& source, uint64_t& destination) {
    if (source < kMoneyRangeLimit) {
        source = 0u;
    } else {
        destination = kMoneyRangeLimit;
        source -= kMoneyRangeLimit;
    }
}

}  // namespace

// IDA_EA: 0x8287230
// IDA_NAME: _ZN16Secu_GoldControl4InitEP5CUser
// DEMANGLED: Secu_GoldControl::Init(CUser *)
void* Secu_GoldControl::Init(CUser* user) {
    owner_ = user;
    date_flag_ = static_cast<uint32_t>(GlobalData::g_GoldControlDateFlag);
    used_fatigue_ = 0u;
    previous_used_fatigue_ = 0u;
    mail_gold_ = 0u;
    mail_charac_name_[0] = '\0';
    return memset(mail_charac_name_, 0, sizeof(mail_charac_name_));
}

// IDA_EA: 0x82872A0
// IDA_NAME: _ZN16Secu_GoldControl7SetInfoER14user_gold_info
// DEMANGLED: Secu_GoldControl::SetInfo(user_gold_info &)
user_gold_info* Secu_GoldControl::SetInfo(user_gold_info& info) {
    return info.toCopy(range_info_);
}

// This narrow named helper is an exact friendship boundary declared by CUser;
// it replaces the old anonymous CUser+0xe0 cast without creating a public
// network accessor.
void Secu_GoldControl_capture_hades_endpoint(Secu_GoldControl* gold_control,
                                             char* ip,
                                             char* mac) {
    CUser* const user = gold_control->owner_;
    user->getHades()->GetIP(ip);
    memset(mac, 0, 65u);
    strncpy(mac, user->network_.get_mac_addr(), 65u);
}

// IDA_EA: 0x828800A
// IDA_NAME: _ZN14user_gold_info5resetEv
// DEMANGLED: user_gold_info::reset(void)
user_gold_info* user_gold_info::reset() {
    memset(this, 0, sizeof(*this));
    return this;
}

// IDA_EA: 0x828802E
// IDA_NAME: _ZN14user_gold_info6toCopyERS_
// DEMANGLED: user_gold_info::toCopy(user_gold_info&)
user_gold_info* user_gold_info::toCopy(user_gold_info& destination) {
    memcpy(&destination, this, sizeof(*this));
    return &destination;
}

// IDA_EA: 0x8287BDC
// IDA_NAME: _ZN16Secu_GoldControl15resetRangeMoneyER14user_gold_infoS1_
// DEMANGLED: Secu_GoldControl::resetRangeMoney(user_gold_info &,user_gold_info &)
int Secu_GoldControl::resetRangeMoney(user_gold_info& source,
                                      user_gold_info& destination) {
    user_gold_info* const src_info = &source;
    user_gold_info* const dest_info = &destination;
    memcpy(dest_info, src_info, sizeof(*src_info));
    const bool last_under_limit =
        src_info->sub_auction_money_amount_ < kMoneyRangeLimit;
    rollover_amount(src_info->add_money_amount_,
                    dest_info->add_money_amount_);
    rollover_amount(src_info->add_import_money_amount_,
                    dest_info->add_import_money_amount_);
    rollover_amount(src_info->sub_money_amount_,
                    dest_info->sub_money_amount_);
    rollover_amount(src_info->sub_import_money_amount_,
                    dest_info->sub_import_money_amount_);
    rollover_amount(src_info->add_auction_money_amount_,
                    dest_info->add_auction_money_amount_);
    rollover_amount(src_info->sub_auction_money_amount_,
                    dest_info->sub_auction_money_amount_);
    if (last_under_limit) {
        return reinterpret_cast<int>(src_info);
    }
    return static_cast<int>(src_info->sub_auction_money_amount_);
}

// IDA_EA: 0x82872CA
// IDA_NAME: _ZN16Secu_GoldControl8SavetoDBEbbb
// DEMANGLED: Secu_GoldControl::SavetoDB(bool,bool,bool)
int Secu_GoldControl::SavetoDB(bool save_current,
                               bool save_previous,
                               bool reset_range) {
    Secu_GoldControl* const gold_control = this;
    int result = static_cast<int>(reset_range);
    if (save_current &&
        (gold_control->range_daily_info_.add_money_count_ ||
         gold_control->range_daily_info_.sub_money_count_ ||
         gold_control->used_fatigue_)) {
        char ip[16] = {};
        char mac[65];
        Secu_GoldControl_capture_hades_endpoint(gold_control, ip, mac);
        if (reset_range) {
            user_gold_info saved;
            gold_control->resetRangeMoney(gold_control->range_daily_info_, saved);
        } else {
            gold_control->range_daily_info_.reset();
        }
        gold_control->used_fatigue_ = 0u;
        result = reinterpret_cast<int>(gold_control);
    }
    if (save_previous &&
        (gold_control->range_saved_info_.add_money_count_ ||
         gold_control->range_saved_info_.sub_money_count_ ||
         gold_control->previous_used_fatigue_)) {
        char ip[16] = {};
        char mac[65];
        Secu_GoldControl_capture_hades_endpoint(gold_control, ip, mac);
        gold_control->range_saved_info_.reset();
        gold_control->previous_used_fatigue_ = 0u;
        result = reinterpret_cast<int>(gold_control);
    }
    return result;
}

// IDA_EA: 0x828795A
// IDA_NAME: _ZN16Secu_GoldControl15SubAuctionMoneyEj
// DEMANGLED: Secu_GoldControl::SubAuctionMoney(unsigned int)
void Secu_GoldControl::SubAuctionMoney(unsigned int money) {
    Secu_GoldControl* const gold_control = this;
    if (money > 0x1387u) {
        gold_control->range_daily_info_.sub_auction_money_amount_ += money;
        gold_control->range_info_.sub_auction_money_amount_ += money;
        ++gold_control->range_info_.sub_auction_money_count_;
        gold_control->owner_->getHades()->GoldMinus(
            money, XNuclear::ENUM_GOLD_CONTROL_REASON_GOLD);
    }
}

// IDA_EA: 0x82879E4
// IDA_NAME: _ZN16Secu_GoldControl8SubMoneyEj
// DEMANGLED: Secu_GoldControl::SubMoney(unsigned int)
int Secu_GoldControl::SubMoney(unsigned int money) {
    Secu_GoldControl* const gold_control = this;
    gold_control->range_daily_info_.sub_money_amount_ += money;
    ++gold_control->range_daily_info_.sub_money_count_;
    gold_control->range_info_.sub_money_amount_ += money;
    ++gold_control->range_info_.sub_money_count_;
    return gold_control->owner_->getHades()->GoldMinus(
        money, XNuclear::ENUM_GOLD_CONTROL_REASON_AUCTION);
}

// IDA_EA: 0x8287A74
// IDA_NAME: _ZN16Secu_GoldControl14SubImportMoneyEj
// DEMANGLED: Secu_GoldControl::SubImportMoney(unsigned int)
int Secu_GoldControl::SubImportMoney(unsigned int money) {
    Secu_GoldControl* const gold_control = this;
    gold_control->range_daily_info_.sub_import_money_amount_ += money;
    ++gold_control->range_daily_info_.sub_import_money_count_;
    gold_control->range_info_.sub_import_money_amount_ += money;
    ++gold_control->range_info_.sub_import_money_count_;
    return gold_control->owner_->getHades()->GoldMinus(
        money, XNuclear::ENUM_GOLD_CONTROL_REASON_USER);
}

// IDA_EA: 0x8287DF2
// IDA_NAME: _ZN16Secu_GoldControl13TradeCompleteEii
// DEMANGLED: Secu_GoldControl::TradeComplete(int,int)
int Secu_GoldControl::TradeComplete(int own_gold, int opposite_gold) {
    Secu_GoldControl* const gold_control = this;
    int result = 0;
    if (own_gold > 0) {
        gold_control->SubImportMoney(
                                        static_cast<unsigned int>(own_gold));
        result = gold_control->SubMoney(
                                           static_cast<unsigned int>(own_gold));
    }
    if (opposite_gold > 0) {
        return gold_control->AddMoney(static_cast<unsigned int>(opposite_gold));
    }
    return result;
}

// IDA_EA: 0x8287B04
// IDA_NAME: _ZN16Secu_GoldControl9CheckDateEv
// DEMANGLED: Secu_GoldControl::CheckDate(void)
Secu_GoldControl* Secu_GoldControl::CheckDate() {
    Secu_GoldControl* const gold_control = this;
    const uint32_t date_flag =
        static_cast<uint32_t>(GlobalData::g_GoldControlDateFlag);
    if (gold_control->date_flag_ != date_flag) {
        gold_control->date_flag_ = date_flag;
        memcpy(&gold_control->range_saved_info_,
               &gold_control->range_daily_info_,
               sizeof(gold_control->range_saved_info_));
        gold_control->range_info_.reset();
        gold_control->range_daily_info_.reset();
        gold_control->previous_used_fatigue_ = gold_control->used_fatigue_;
        gold_control->used_fatigue_ = 0u;
        return gold_control;
    }
    return reinterpret_cast<Secu_GoldControl*>(
        static_cast<uintptr_t>(date_flag));
}

// IDA_EA: 0x8287B80
// IDA_NAME: _ZN16Secu_GoldControl15CheckMoneyRangeEv
// DEMANGLED: Secu_GoldControl::CheckMoneyRange(void)
int Secu_GoldControl::CheckMoneyRange() {
    Secu_GoldControl* const gold_control = this;
    if (gold_control->range_daily_info_.add_money_amount_ >=
            kMoneyRangeLimit ||
        gold_control->range_daily_info_.sub_money_amount_ >=
            kMoneyRangeLimit) {
        return reinterpret_cast<Secu_GoldControl*>(gold_control)->SavetoDB(true, false, true);
    }
    return static_cast<int>(gold_control->range_daily_info_.sub_money_amount_);
}

// IDA_EA: 0x8287E3C
// IDA_NAME: _ZN16Secu_GoldControl8MailGoldEjb
// DEMANGLED: Secu_GoldControl::MailGold(unsigned int,bool)
Secu_GoldControl* Secu_GoldControl::MailGold(unsigned int money,
                                             bool /*is_sub*/) {
    Secu_GoldControl* const gold_control = this;
    gold_control->mail_gold_ = money;
    return gold_control;
}

// IDA_EA: 0x8287E56
// IDA_NAME: _ZN16Secu_GoldControl11MailSameMidEv
// DEMANGLED: Secu_GoldControl::MailSameMid(void)
Secu_GoldControl* Secu_GoldControl::MailSameMid() {
    mail_gold_ = 0u;
    return this;
}

// IDA_EA: 0x8287E68
// IDA_NAME: _ZN16Secu_GoldControl12MailCompleteEb
// DEMANGLED: Secu_GoldControl::MailComplete(bool)
Secu_GoldControl* Secu_GoldControl::MailComplete(bool subtract_mail_gold) {
    if (subtract_mail_gold) {
        this->SubImportMoney(mail_gold_);
        this->SubMoney(mail_gold_);
    } else {
        this->AddMoney(mail_gold_);
    }
    mail_gold_ = 0u;
    return this;
}

// IDA_EA: 0x8287EE0
// IDA_NAME: _ZN16Secu_GoldControl15SetMailCharNameEPKc
// DEMANGLED: Secu_GoldControl::SetMailCharName(char const*)
char* Secu_GoldControl::SetMailCharName(const char* sender_name) {
    if (!sender_name) {
        return 0;
    }
    return strncpy(mail_charac_name_, sender_name,
                   sizeof(mail_charac_name_));
}

// IDA_EA: 0x828772A
// IDA_NAME: _ZN16Secu_GoldControl8AddMoneyEj
// DEMANGLED: Secu_GoldControl::AddMoney(unsigned int)
int Secu_GoldControl::AddMoney(unsigned int money) {
    Secu_GoldControl* const gold_control = this;
    gold_control->range_daily_info_.add_money_amount_ += money;
    ++gold_control->range_daily_info_.add_money_count_;
    gold_control->range_info_.add_money_amount_ += money;
    ++gold_control->range_info_.add_money_count_;
    return gold_control->owner_->getHades()->GoldPlus(
        money, XNuclear::ENUM_GOLD_CONTROL_REASON_AUCTION);
}

// IDA_EA: 0x82877BA
// IDA_NAME: _ZN16Secu_GoldControl14AddImportMoneyEj
// DEMANGLED: Secu_GoldControl::AddImportMoney(unsigned int)
int Secu_GoldControl::AddImportMoney(unsigned int money) {
    Secu_GoldControl* const gold_control = this;
    gold_control->range_daily_info_.add_import_money_amount_ += money;
    ++gold_control->range_daily_info_.add_import_money_count_;
    gold_control->range_info_.add_import_money_amount_ += money;
    ++gold_control->range_info_.add_import_money_count_;
    return gold_control->owner_->getHades()->GoldPlus(
        money, XNuclear::ENUM_GOLD_CONTROL_REASON_USER);
}

// IDA_EA: 0x828784A
// IDA_NAME: _ZN16Secu_GoldControl15AddAuctionMoneyEj
// DEMANGLED: Secu_GoldControl::AddAuctionMoney(unsigned int)
void Secu_GoldControl::AddAuctionMoney(unsigned int money) {
    Secu_GoldControl* const gold_control = this;
    if (money <= 0x1387u) {
        return;
    }

    gold_control->range_daily_info_.add_auction_money_amount_ += money;
    ++gold_control->range_daily_info_.add_auction_money_count_;
    gold_control->range_info_.add_auction_money_amount_ += money;
    ++gold_control->range_info_.add_auction_money_count_;

    CUser* const user = gold_control->owner_;
    XNuclear::CHades* const hades = user->getHades();
    hades->GoldPlus(money, XNuclear::ENUM_GOLD_CONTROL_REASON_GOLD);

    char ip[24] = {};
    hades->GetIP(ip);
    hades->GoldTracking(
        4u,
        static_cast<unsigned int>(user->get_acc_id()),
        ip,
        money,
        static_cast<unsigned int>(
            static_cast<CUserCharacInfo*>(user)->getCurCharacUsedFatigue()));
}

// IDA_EA: 0x828757C
// IDA_NAME: _ZN16Secu_GoldControl7AddGoldEj15eMoneyAddReason
// DEMANGLED: Secu_GoldControl::AddGold(unsigned int,eMoneyAddReason)
void Secu_GoldControl::AddGold(unsigned int money,
                               eMoneyAddReason reason) {
    Secu_GoldControl* const gold_control = this;
    gold_control->CheckDate();
    switch (reason) {
    case 0:
    case 3:
    case 4:
    case 5:
    case 10:
    case 15:
    case 26:
        gold_control->AddImportMoney(money);
        break;
    case 1:
    case 2:
        return;
    case 14: {
        gold_control->MailGold(money, false);
        CUser* const user = gold_control->owner_;
        DB_CheckCharacName::makeRequest(
            user->GetUID(),
            static_cast<unsigned int>(user->get_acc_id()),
            gold_control->mail_charac_name_,
            false);
        return;
    }
    case 18:
        gold_control->AddAuctionMoney(money);
        break;
    default:
        break;
    }
    gold_control->AddMoney(money);
    gold_control->CheckMoneyRange();
}

// IDA_EA: 0x828764A
// IDA_NAME: _ZN16Secu_GoldControl7SubGoldEj15eMoneySubReason
// DEMANGLED: Secu_GoldControl::SubGold(unsigned int,eMoneySubReason)
int Secu_GoldControl::SubGold(unsigned int money,
                              eMoneySubReason reason) {
    Secu_GoldControl* const gold_control = this;
    gold_control->CheckDate();
    if (reason == 14) {
        gold_control->SubImportMoney(money);
    } else if (reason > 14) {
        if (reason == 17) {
            gold_control->MailGold(money, true);
            CUser* user = gold_control->owner_;
            DB_CheckCharacName::makeRequest(
                user->GetUID(),
                static_cast<unsigned int>(user->get_acc_id()),
                gold_control->mail_charac_name_,
                true);
            return 0;
        }
        if (reason == 26) {
            gold_control->SubAuctionMoney(money);
        }
    } else if (reason == 2) {
        return reason;
    }
    gold_control->SubMoney(money);
    return gold_control->CheckMoneyRange();
}
