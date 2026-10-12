#include "common/manager/gm_accounts_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/log/format/trace.h"

#include <algorithm>

WongWork::CGMAccounts* GlobalData::s_pGMAccounts_ = 0;

namespace WongWork {

CGMAccounts::stGMInfo_t::stGMInfo_t()
    : account_id(0),
      gm_level(3) {}

CGMAccounts::stGMInfo_t::stGMInfo_t(uint32_t account_id_value,
                                    int gm_level_value)
    : account_id(account_id_value),
      gm_level(gm_level_value) {}

// IDA_EA: 0x8109610
// IDA_NAME: _ZNK8WongWork11CGMAccounts10stGMInfo_teqERKS1_
// DEMANGLED: WongWork::CGMAccounts::stGMInfo_t::operator==(stGMInfo_t const&)const
bool CGMAccounts::stGMInfo_t::operator==(const stGMInfo_t& other) const {
    return account_id == other.account_id;
}

// IDA_EA: 0x82A73A4
// IDA_NAME: _ZN8WongWork11CGMAccountsC2Ev
// DEMANGLED: WongWork::CGMAccounts::CGMAccounts(void)
CGMAccounts::CGMAccounts() : gm_list_() {}

// IDA_EA: 0x82A73B8
// IDA_NAME: _ZN8WongWork11CGMAccountsD2Ev
// DEMANGLED: WongWork::CGMAccounts::~CGMAccounts(void)
CGMAccounts::~CGMAccounts() {}

// IDA_EA: 0x810933C
// IDA_NAME: _ZN8WongWork11CGMAccounts14loadGMAccountsEPKc
// DEMANGLED: WongWork::CGMAccounts::loadGMAccounts(char const*)
int CGMAccounts::loadGMAccounts(const char* filename) {
    (void)filename;
    return 1;
}

// IDA_EA: 0x8109346
// IDA_NAME: _ZN8WongWork11CGMAccounts4isGMEj
// DEMANGLED: WongWork::CGMAccounts::isGM(unsigned int)
bool CGMAccounts::isGM(uint32_t account_id) {
    const stGMInfo_t gm_info(account_id, 3);
    return std::find(gm_list_.begin(), gm_list_.end(), gm_info) !=
           gm_list_.end();
}

// IDA_EA: 0x81093EE
// IDA_NAME: _ZN8WongWork11CGMAccounts8appendGMEjj
// DEMANGLED: WongWork::CGMAccounts::appendGM(unsigned int,unsigned int)
int CGMAccounts::appendGM(uint32_t account_id, uint32_t gm_level) {
    (void)account_id;
    (void)gm_level;
    return 0;
}

// IDA_EA: 0x81093FC
// IDA_NAME: _ZN8WongWork11CGMAccounts8removeGMEjj
// DEMANGLED: WongWork::CGMAccounts::removeGM(unsigned int,unsigned int)
int CGMAccounts::removeGM(uint32_t account_id, uint32_t gm_level) {
    (void)account_id;
    (void)gm_level;
    return 0;
}

// IDA_EA: 0x810940A
// IDA_NAME: _ZNK8WongWork11CGMAccounts9getGMInfoEj
// DEMANGLED: WongWork::CGMAccounts::getGMInfo(unsigned int)const
CGMAccounts::stGMInfo_t CGMAccounts::getGMInfo(uint32_t account_id) const {
    const stGMInfo_t gm_info(account_id, 3);
    std::list<stGMInfo_t>::const_iterator it =
        std::find(gm_list_.begin(), gm_list_.end(), gm_info);
    if (it != gm_list_.end()) {
        return *it;
    }
    return stGMInfo_t(0, 3);
}

// IDA_EA: 0x81094EA
// IDA_NAME: _ZN8WongWork11CGMAccounts10LoadGmListEji
// DEMANGLED: WongWork::CGMAccounts::LoadGmList(unsigned int,int)
int CGMAccounts::LoadGmList(uint32_t account_id, int gm_level) {
    const stGMInfo_t gm_info(account_id, gm_level);
    gm_list_.push_back(gm_info);
    return 0;
}

// IDA_EA: 0x8109510
// IDA_NAME: _ZN8WongWork11CGMAccounts11clearGmListEv
// DEMANGLED: WongWork::CGMAccounts::clearGmList(void)
int CGMAccounts::clearGmList() {
    gm_list_.clear();
    return 0;
}

// IDA_EA: 0x8109524
// IDA_NAME: _ZN8WongWork11CGMAccounts12AppendGM_SysEjc
// DEMANGLED: WongWork::CGMAccounts::AppendGM_Sys(unsigned int,char)
int CGMAccounts::AppendGM_Sys(uint32_t account_id, char gm_level) {
    const stGMInfo_t gm_info(account_id, gm_level);
    gm_list_.push_back(gm_info);
    const char* account_id_text = NumberToString(account_id, 0);
    CMyFileLog log("AppendGM_Sys", 205);
    return log("./log/Init", "GM List Add mid:%s", account_id_text);
}

// IDA_EA: 0x82A7388
// IDA_NAME: _ZNK8WongWork11CGMAccounts9GetGMListEv
// DEMANGLED: WongWork::CGMAccounts::GetGMList(void)const
const std::list<CGMAccounts::stGMInfo_t>& CGMAccounts::GetGMList() const {
    return gm_list_;
}

}  // namespace WongWork
