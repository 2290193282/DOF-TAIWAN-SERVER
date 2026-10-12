#include "common/manager/user.h"
#include "common/manager/global_data.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "common/manager/system_time.h"
#include "infra/log/format/clog.h"

#include <map>
#include <time.h>
#include <vector>

// IDA_EA: 0x84EBE20
// IDA_NAME: _ZN8Sanicova4CPad12setCheckTimeEl
// DEMANGLED: Sanicova::CPad::setCheckTime(long)
Sanicova::CPad* Sanicova::CPad::setCheckTime(long check_time) {
    check_time_ = check_time;
    return this;
}

namespace WongWork {
// IDA_EA: 0x86AF96E
// IDA_NAME: _ZN8WongWork14CHandlePremium23handleReturn2CharacViewEP5CUser
// DEMANGLED: WongWork::CHandlePremium::handleReturn2CharacView(CUser *)
void CHandlePremium::handleReturn2CharacView(CUser* user) {
    std::vector<SUserPremiumInfo> premium_list =
        user->GetPremiumInfo()->GetPremiumInfoList(9999);
    for (std::vector<SUserPremiumInfo>::iterator it = premium_list.begin();
         it != premium_list.end(); ++it) {
        if (it->premium_type && it->state == 3) {
            handleSetUserPremium(
                user,
                static_cast<ENUM_PREMIUM_TYPE>(it->premium_type),
                0,
                0,
                0,
                true);
        }
    }
}

}  // namespace WongWork

// IDA_EA: 0x81B441A
// IDA_NAME: _ZN20UserMercenaryInfoMgr12RemoveCharacEj
// DEMANGLED: UserMercenaryInfoMgr::RemoveCharac(unsigned int)
int UserMercenaryInfoMgr::RemoveCharac(unsigned int charac_no) {
    for (std::vector<CHARAC_LOAD_MERCENARY>::iterator it = mercenaries_.begin();
         it != mercenaries_.end(); ++it) {
        if (it->charac_no == charac_no) {
            mercenaries_.erase(it);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8685BB8
// IDA_NAME: _ZN15cUserHistoryLog11GuildAddExpEjPKcji
// DEMANGLED: cUserHistoryLog::GuildAddExp(unsigned int,char const*,unsigned int,int)
int cUserHistoryLog::GuildAddExp(unsigned int guild_key,
                                 const char* guild_name,
                                 unsigned int charac_no,
                                 int guild_exp) {
    const time_t now =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    struct tm tm_value;
    const struct tm* local_time = localtime_r(&now, &tm_value);
    return owner_->LogHistory("GuildAddExp,\"%d/%d/%d\",%d,\"%s\",%d,%d",
                              local_time->tm_year + 1900,
                              local_time->tm_mon + 1,
                              local_time->tm_mday,
                              guild_key,
                              guild_name,
                              charac_no,
                              guild_exp);
}

// IDA_EA: 0x8682A92
// IDA_NAME: _ZN15cUserHistoryLog9CharacAddEi
// DEMANGLED: cUserHistoryLog::CharacAdd(int)
int cUserHistoryLog::CharacAdd(int charac_slot) {
    TimeLog_ time;
    time.setTime();
    return owner_->LogHistory(
        "Charac+,%d",
        static_cast<int>(owner_->characters_[charac_slot].fields_.job_));
}

// IDA_EA: 0x8682AE0
// IDA_NAME: _ZN15cUserHistoryLog9CharacDelEi
// DEMANGLED: cUserHistoryLog::CharacDel(int)
int cUserHistoryLog::CharacDel(int charac_slot) {
    TimeLog_ time;
    time.setTime();
    return owner_->LogHistory(
        "Charac-,%d",
        static_cast<int>(owner_->characters_[charac_slot].fields_.job_));
}

// IDA_EA: 0x8683B90
// IDA_NAME: _ZN15cUserHistoryLog7CoinAddEii14eCoinAddReason
// DEMANGLED: cUserHistoryLog::CoinAdd(int,int,eCoinAddReason)
int cUserHistoryLog::CoinAdd(int coin,
                              int add_count,
                              eCoinAddReason reason) {
    return owner_->LogHistory("Coin+,%d,%d,%d", coin, add_count, reason);
}

// IDA_EA: 0x8683CBC
// IDA_NAME: _ZN15cUserHistoryLog15ExpandInventoryEiii
// DEMANGLED: cUserHistoryLog::ExpandInventory(int,int,int)
int cUserHistoryLog::ExpandInventory(int old_capacity,
                                      int new_capacity,
                                      int reason) {
    return owner_->LogHistory("ExpandInventory+,%d,%d,%d",
                              old_capacity,
                              new_capacity,
                              reason);
}

// IDA_EA: 0x868885A
// IDA_NAME: _ZN15cUserHistoryLog5QPSubEiii
// DEMANGLED: cUserHistoryLog::QPSub(int,int,int)
int cUserHistoryLog::QPSub(int before_qp, int after_qp, int status_id) {
    return owner_->LogHistory("QP-,%d,%d,%d", before_qp, after_qp,
                              status_id);
}

// IDA_EA: 0x86888BE
// IDA_NAME: _ZN15cUserHistoryLog11ClearUsedQPEiiii
// DEMANGLED: cUserHistoryLog::ClearUsedQP(int,int,int,int)
int cUserHistoryLog::ClearUsedQP(int before_qp,
                                  int after_qp,
                                  int init_count,
                                  int init_cost) {
    return owner_->LogHistory("ClearQP,%d,%d,%d,%d", before_qp, after_qp,
                              init_count, init_cost);
}

// IDA_EA: 0x86885A6
// IDA_NAME: _ZN5CUser17GetSecretShopDataEv
// DEMANGLED: CUser::GetSecretShopData(void)
secretshop::SECRET_SHOP_DATA* CUser::GetSecretShopData() {
    CParty* const party = GetParty();
    if (party) {
        return party->GetSecretShopData();
    }
    return 0;
}

// IDA_EA: 0x86885D4
// IDA_NAME: _ZN5CUser17GetUserSecretShopEv
// DEMANGLED: CUser::GetUserSecretShop(void)
secretshop::SECRET_SHOP_INFO* CUser::GetUserSecretShop() {
    CParty* const party = GetParty();
    if (party) {
        return party->GetUserSecretShop(this);
    }
    return 0;
}

// IDA_EA: 0x868860A
// IDA_NAME: _ZN5CUser22SendSecretShopItemListEv
// DEMANGLED: CUser::SendSecretShopItemList(void)
CParty* CUser::SendSecretShopItemList() {
    CParty* const party = GetParty();
    if (party) {
        party->SendSecretShopItemList(this);
    }
    return party;
}
