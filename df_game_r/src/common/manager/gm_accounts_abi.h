#ifndef OLDSRV_GLOBAL_GM_ACCOUNTS_ABI_H_
#define OLDSRV_GLOBAL_GM_ACCOUNTS_ABI_H_

#include <stdint.h>

#include <list>

namespace WongWork {

// IDA unit: 02_login_account/WongWork/CGMAccounts.
// The old object is exactly a std::list<stGMInfo_t> at offset 0.
class CGMAccounts {
public:
    struct stGMInfo_t {
        stGMInfo_t();
        stGMInfo_t(uint32_t account_id, int gm_level);

        // IDA_EA: 0x8109610
        // IDA_NAME: _ZNK8WongWork11CGMAccounts10stGMInfo_teqERKS1_
        bool operator==(const stGMInfo_t& other) const;

        uint32_t account_id;
        int gm_level;
    };

    // IDA_EA: 0x82A73A4
    // IDA_NAME: _ZN8WongWork11CGMAccountsC2Ev
    CGMAccounts();

    // IDA_EA: 0x82A73B8
    // IDA_NAME: _ZN8WongWork11CGMAccountsD2Ev
    ~CGMAccounts();

    // IDA_EA: 0x810933C
    // IDA_NAME: _ZN8WongWork11CGMAccounts14loadGMAccountsEPKc
    int loadGMAccounts(const char* filename);

    // IDA_EA: 0x8109346
    // IDA_NAME: _ZN8WongWork11CGMAccounts4isGMEj
    bool isGM(uint32_t account_id);

    // IDA_EA: 0x81093EE
    // IDA_NAME: _ZN8WongWork11CGMAccounts8appendGMEjj
    int appendGM(uint32_t account_id, uint32_t gm_level);

    // IDA_EA: 0x81093FC
    // IDA_NAME: _ZN8WongWork11CGMAccounts8removeGMEjj
    int removeGM(uint32_t account_id, uint32_t gm_level);

    // IDA_EA: 0x810940A
    // IDA_NAME: _ZNK8WongWork11CGMAccounts9getGMInfoEj
    stGMInfo_t getGMInfo(uint32_t account_id) const;

    // IDA_EA: 0x81094EA
    // IDA_NAME: _ZN8WongWork11CGMAccounts10LoadGmListEji
    int LoadGmList(uint32_t account_id, int gm_level);

    // IDA_EA: 0x8109510
    // IDA_NAME: _ZN8WongWork11CGMAccounts11clearGmListEv
    int clearGmList();

    // IDA_EA: 0x8109524
    // IDA_NAME: _ZN8WongWork11CGMAccounts12AppendGM_SysEjc
    int AppendGM_Sys(uint32_t account_id, char gm_level);

    // IDA_EA: 0x82A7388
    // IDA_NAME: _ZNK8WongWork11CGMAccounts9GetGMListEv
    const std::list<stGMInfo_t>& GetGMList() const;

private:
    std::list<stGMInfo_t> gm_list_;

    CGMAccounts(const CGMAccounts&);
    CGMAccounts& operator=(const CGMAccounts&);
};

}  // namespace WongWork

class GlobalData;

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(WongWork::CGMAccounts::stGMInfo_t) == 0x08u,
              "CGMAccounts::stGMInfo_t IDA ABI size must stay 0x08");
static_assert(sizeof(WongWork::CGMAccounts) == 0x08u,
              "CGMAccounts IDA ABI size must stay 0x08");
#endif

#endif  // OLDSRV_GLOBAL_GM_ACCOUNTS_ABI_H_
