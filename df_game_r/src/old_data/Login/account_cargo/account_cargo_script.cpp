#include "old_data/Login/account_cargo/account_cargo_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <utility>
#include <vector>

// IDA_DATA: 0x8E09360 dir__10
// The original object is the empty directory string immediately preceding the
// importer tag strings.  It has exactly one xref: loadRDARScriptFile below.
extern "C" const char dir__10[] = "";

// IDA_EA: 0x8372442
// IDA_NAME: _ZN18AccountCargoScript5clearEv
// DEMANGLED: AccountCargoScript::clear(void)
int AccountCargoScript::clear() {
    required_level = 0;
    required_item = 0;
    upgrade_infos.clear();
    return 0;
}

// IDA_EA: 0x8372480
// IDA_NAME: _ZN18AccountCargoScriptC2Ev
// DEMANGLED: AccountCargoScript::AccountCargoScript(void)
AccountCargoScript::AccountCargoScript()
    : upgrade_infos() {
    clear();
}

// IDA_EA: 0x837D9EC
// IDA_NAME: _ZN18AccountCargoScriptD2Ev
// DEMANGLED: AccountCargoScript::~AccountCargoScript()
AccountCargoScript::~AccountCargoScript() {}

// IDA_EA: 0x88C8056
// IDA_NAME: _ZN18AccountCargoScript13GetCreateInfoEv
// DEMANGLED: AccountCargoScript::GetCreateInfo(void)
UpgradeInfo* AccountCargoScript::GetCreateInfo() {
    std::map<int, UpgradeInfo>::iterator it = upgrade_infos.begin();
    if (it != upgrade_infos.end()) {
        return &it->second;
    }
    return 0;
}

// IDA_EA: 0x88C80BA
// IDA_NAME: _ZN18AccountCargoScript18GetCurrUpgradeInfoEi
// DEMANGLED: AccountCargoScript::GetCurrUpgradeInfo(int)
UpgradeInfo* AccountCargoScript::GetCurrUpgradeInfo(int capacity) {
    std::map<int, UpgradeInfo>::iterator it = upgrade_infos.find(capacity);
    if (it != upgrade_infos.end()) {
        return &it->second;
    }
    return 0;
}

// IDA_EA: 0x88C8124
// IDA_NAME: _ZN18AccountCargoScript18GetNextUpgradeInfoEi
// DEMANGLED: AccountCargoScript::GetNextUpgradeInfo(int)
UpgradeInfo* AccountCargoScript::GetNextUpgradeInfo(int capacity) {
    std::map<int, UpgradeInfo>::iterator it = upgrade_infos.find(capacity);
    if (it == upgrade_infos.end()) {
        return 0;
    }

    std::map<int, UpgradeInfo>::iterator next = it;
    ++next;
    if (next != upgrade_infos.end()) {
        return &next->second;
    }
    return 0;
}

// IDA_EA: 0x88C7E80
// IDA_NAME: _Z24importAccountCargoScriptP18AccountCargoScriptPKc
// DEMANGLED: importAccountCargoScript(AccountCargoScript *,char const*)
int importAccountCargoScript(AccountCargoScript* script, const char* filename) {
    if (!loadRDARScriptFile(dir__10, filename)) {
        return 0;
    }

    script->clear();

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[required level]") {
            script->required_level = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[required item]") {
            script->required_item = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[upgrade info]") {
            std::vector<int> values;
            while (1) {
                bool is_type = false;
                bool token_success = false;
                std::string token;
                const int value =
                    ScanTypeOrInt(is_type, token, token_success, true);
                if (!token_success) {
                    return 0;
                }
                if (is_type) {
                    if (token != "[/upgrade info]") {
                        return 0;
                    }
                    break;
                }
                values.push_back(value);
            }

            bool has_cash_upgrade_item =
                !values.empty() && values.size() % 5u == 0u;
            if (has_cash_upgrade_item) {
                for (std::vector<int>::size_type i = 4;
                     i < values.size(); i += 5) {
                    if (values[i] != -1 && values[i] < 10000000) {
                        has_cash_upgrade_item = false;
                        break;
                    }
                }
            }

            const std::vector<int>::size_type stride =
                has_cash_upgrade_item ? 5u : 4u;
            if (values.empty() || values.size() % stride != 0u) {
                return 0;
            }

            for (std::vector<int>::size_type i = 0;
                 i < values.size(); i += stride) {
                UpgradeInfo info;
                info.capacity = values[i];
                info.money_limit = values[i + 1];
                info.required_item_count = values[i + 2];
                info.required_gold = values[i + 3];
                info.cash_upgrade_item_id =
                    has_cash_upgrade_item ? values[i + 4] : -1;
                script->upgrade_infos.insert(
                    std::make_pair(info.capacity, info));
            }
        } else if (tag == "[account cargo end]") {
            success = true;
            break;
        }
    }

    return success;
}
