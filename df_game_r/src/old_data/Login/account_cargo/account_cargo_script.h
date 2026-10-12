#ifndef OLDSRV_OLD_DATA_LOGIN_ACCOUNT_CARGO_SCRIPT_H_
#define OLDSRV_OLD_DATA_LOGIN_ACCOUNT_CARGO_SCRIPT_H_

#include <stdint.h>

#include <map>

class CDataManager;

// IDA unit: 02_login_account/AccountCargoScript.
// This is B-owned RDAR data consumed by the account-cargo runtime.
struct UpgradeInfo {
    int32_t capacity;
    int32_t money_limit;
    int32_t required_item_count;
    int32_t required_gold;
    int32_t cash_upgrade_item_id;
};

class AccountCargoScript {
public:
    // IDA_EA: 0x8372480
    // IDA_NAME: _ZN18AccountCargoScriptC2Ev
    AccountCargoScript();
    // IDA_EA: 0x837D9EC
    // IDA_NAME: _ZN18AccountCargoScriptD2Ev
    ~AccountCargoScript();

    // IDA_EA: 0x8372442
    // IDA_NAME: _ZN18AccountCargoScript5clearEv
    int clear();
    // IDA_EA: 0x88C8056
    // IDA_NAME: _ZN18AccountCargoScript13GetCreateInfoEv
    UpgradeInfo* GetCreateInfo();
    // IDA_EA: 0x88C80BA
    // IDA_NAME: _ZN18AccountCargoScript18GetCurrUpgradeInfoEi
    UpgradeInfo* GetCurrUpgradeInfo(int capacity);
    // IDA_EA: 0x88C8124
    // IDA_NAME: _ZN18AccountCargoScript18GetNextUpgradeInfoEi
    UpgradeInfo* GetNextUpgradeInfo(int capacity);

    int32_t required_level;
    int32_t required_item;
    std::map<int, UpgradeInfo> upgrade_infos;
};

// The PVF importer owns this already-existing typed boundary for the embedded
// CDataManager +0xA7E0 AccountCargoScript.  Keep the offset implementation in
// that data-manager unit; account-cargo business code only consumes the type.
AccountCargoScript* AccountCargoAt(CDataManager* data_manager);

// IDA_EA: 0x88C7E80
// IDA_NAME: _Z24importAccountCargoScriptP18AccountCargoScriptPKc
// DEMANGLED: importAccountCargoScript(AccountCargoScript *,char const*)
int importAccountCargoScript(AccountCargoScript* script, const char* filename);

static_assert(sizeof(UpgradeInfo) == 0x14u,
              "UpgradeInfo PVF value size must stay 0x14");
static_assert(sizeof(AccountCargoScript) == 0x20u,
              "AccountCargoScript IDA ABI size must stay 0x20");

#endif  // OLDSRV_OLD_DATA_LOGIN_ACCOUNT_CARGO_SCRIPT_H_
