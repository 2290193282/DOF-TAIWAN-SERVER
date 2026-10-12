#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/ipg_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x86928C2
// IDA_NAME: _ZN5CUser13DoGetGoldItemEiRSt6vectorISt4pairIiiESaIS2_EEbR10ENUM_ERROR
// DEMANGLED: CUser::DoGetGoldItem(int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,ENUM_ERROR &)
int CUser::DoGetGoldItem(int gold_item_id,
                         std::vector<std::pair<int, int> >& inserted_slots,
                         bool is_received,
                         ENUM_ERROR& error) {
    int money = 0;
    if (gold_item_id > 2675345) {
        money = 10000000 * gold_item_id - 88713216;
    } else {
        money = 1000000 * (gold_item_id - 2675335);
    }

    if (is_received) {
        const int money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
            get_charac_level(), get_acc_name());
        if (money_limit - getCurCharacMoney() < money) {
            error = static_cast<ENUM_ERROR>(10);
            return 0;
        }
    } else {
        getCurCharacInvenW()->gain_money(
            money, static_cast<eMoneyAddReason>(14), true, 0);
        inserted_slots.push_back(std::make_pair(0, 0));
    }
    return 1;
}

// IDA_EA: 0x8692A34
// IDA_NAME: _ZN5CUser18DoGetCeraPointItemEjbR10ENUM_ERRORm
// DEMANGLED: CUser::DoGetCeraPointItem(unsigned int,bool,ENUM_ERROR &,unsigned long)
int CUser::DoGetCeraPointItem(unsigned int package_money,
                              bool is_received,
                              ENUM_ERROR&,
                              unsigned long cera_item_id) {
    if (!is_received) {
        const char charge_kind =
            cera_item_id == 2681917ul ? static_cast<char>(3)
                                      : static_cast<char>(1);
        GlobalData::s_pIPGHelper->IPGInputPoint(
            this, package_money, charge_kind, 0, 0);
        GlobalData::s_pIPGHelper->IPGQuery(this);
    }
    return 1;
}

// IDA_EA: 0x86848A8
// IDA_NAME: _ZN15cUserHistoryLog8RecvMailEjPKcS1_
// DEMANGLED: cUserHistoryLog::RecvMail(unsigned int,char const*,char const*)
int cUserHistoryLog::RecvMail(unsigned int mail_idx,
                              const char* sender_name,
                              const char* account_name) {
    return owner_->LogHistory("MailR,%d,\"%s\",\"%s\"", mail_idx,
                              sender_name, account_name);
}
