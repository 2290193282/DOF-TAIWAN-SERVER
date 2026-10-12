#include "old_data/PVF/etc/guild_parameter_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string.h>

#include <string>
#include <vector>


typedef char GuildParameterScript_Size_Must_Be_0x11C[
    (sizeof(GuildParameterScript) == 0x11C) ? 1 : -1];

// IDA_EA: 0x83704AE
// IDA_NAME: _ZN20GuildParameterScript5clearEv
// DEMANGLED: GuildParameterScript::clear(void)
GuildParameterScript* GuildParameterScript::clear() {
    guild_create_cost = 0;
    guild_point_acquirement_ratio = 0;
    guild_exp_prob_count = 0;
    guild_level_demand_for_agit = 0;
    guild_exp_book.clear();
    guild_level_demand_for_buying_exp_book = 0;
    agit_upgrade_info.clear();
    guild_storage_upgrade_info.clear();
    guild_exp_member_weight.clear();
    guild_levelup_price.clear();
    today_guild_member_weight = 0.0;
    guild_fund_condition[0] = 0;
    guild_fund_condition[1] = 0;
    guild_fund_condition[2] = 0;
    guild_fund_condition_weight = 0.0;
    memset(guild_levelup_params, 0, 0x88u);
    return this;
}

// IDA_EA: 0x837058A
// IDA_NAME: _ZN20GuildParameterScriptC2Ev
// DEMANGLED: GuildParameterScript::GuildParameterScript(void)
GuildParameterScript::GuildParameterScript()
    : guild_exp_book(),
      agit_upgrade_info(),
      guild_storage_upgrade_info(),
      guild_exp_weight(),
      guild_exp_member_weight(),
      guild_levelup_price() {
    clear();
}

// IDA_EA: 0x837D542
// IDA_NAME: _ZN20GuildParameterScriptD2Ev
// DEMANGLED: GuildParameterScript::~GuildParameterScript()
GuildParameterScript::~GuildParameterScript() {
}

// IDA_EA: 0x89795DE
// IDA_NAME: _ZN20GuildParameterScript17getGuildExpWeightEi
// DEMANGLED: GuildParameterScript::getGuildExpWeight(int)
int32_t GuildParameterScript::getGuildExpWeight(int32_t level) {
    std::map<int, int>::iterator it = guild_exp_weight.find(level);
    if (it != guild_exp_weight.end()) {
        return it->second;
    }
    return 0;
}

// IDA_EA: 0x8979648
// IDA_NAME: _ZN20GuildParameterScript20getGuildLevelUpParamEi
// DEMANGLED: GuildParameterScript::getGuildLevelUpParam(int)
STGuildLevelUpParam* GuildParameterScript::getGuildLevelUpParam(
    int32_t level) {
    if (level <= 0 || level > 16) {
        return 0;
    }
    return &guild_levelup_params[level - 1];
}

// IDA_EA: 0x8979672
// IDA_NAME: _ZN20GuildParameterScript15getGuildExpBookEi
// DEMANGLED: GuildParameterScript::getGuildExpBook(int)
int32_t GuildParameterScript::getGuildExpBook(int32_t value) {
    std::map<int, int>::iterator it = guild_exp_book.find(value);
    if (it != guild_exp_book.end()) {
        return it->second;
    }
    return 0;
}

// IDA_EA: 0x89796DC
// IDA_NAME: _ZN20GuildParameterScript23getGuildAgitUpgradeInfoEi
// DEMANGLED: GuildParameterScript::getGuildAgitUpgradeInfo(int)
STAgitUpgradeInfo* GuildParameterScript::getGuildAgitUpgradeInfo(
    int32_t value) {
    if (value - 1 >= static_cast<int32_t>(agit_upgrade_info.size())) {
        return 0;
    }
    return &agit_upgrade_info.at(value - 1);
}

// IDA_EA: 0x8979726
// IDA_NAME: _ZN20GuildParameterScript23getGuildCargoUpradeInfoEi
// DEMANGLED: GuildParameterScript::getGuildCargoUpradeInfo(int)
STGuildStorageUpgradeInfo* GuildParameterScript::getGuildCargoUpradeInfo(
    int32_t value) {
    for (std::vector<STGuildStorageUpgradeInfo>::iterator it =
             guild_storage_upgrade_info.begin();
         it != guild_storage_upgrade_info.end();
         ++it) {
        if (it->values[0] == value) {
            return &*it;
        }
    }
    return 0;
}

// IDA_EA: 0x89797A4
// IDA_NAME: _ZN20GuildParameterScript15getLevelupPriceEii
// DEMANGLED: GuildParameterScript::getLevelupPrice(int,int)
int32_t GuildParameterScript::getLevelupPrice(int32_t index, int32_t value) {
    for (int32_t i = static_cast<int32_t>(guild_levelup_price.size()) - 1;
         i >= 0;
         --i) {
        if (guild_levelup_price[i].first <= value) {
            return 10000 * guild_levelup_price[i].second[index];
        }
    }
    return 10000 * guild_levelup_price[0].second[index];
}

// IDA_EA: 0x8979864
// IDA_NAME: _ZN20GuildParameterScript12getExpWeightEi
// DEMANGLED: GuildParameterScript::getExpWeight(int)
long double GuildParameterScript::getExpWeight(int32_t value) {
    for (int32_t i = static_cast<int32_t>(guild_exp_member_weight.size()) - 1;
         i >= 0;
         --i) {
        if (guild_exp_member_weight[i].first <= value) {
            return guild_exp_member_weight[i].second;
        }
    }
    return guild_exp_member_weight[0].second;
}

// IDA_EA: 0x8978D48
// IDA_NAME: _Z26importGuildParameterScriptP20GuildParameterScriptPKc
// DEMANGLED: importGuildParameterScript(GuildParameterScript *,char const*)
int importGuildParameterScript(GuildParameterScript* script,
                               const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    GuildParameterScript* target = script;
    target->clear();

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[guild create cost]") {
            target->guild_create_cost = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[guild levelup cost]") {
            for (int index = 0; index <= 16; ++index) {
                const int level = ScanInt(static_cast<bool*>(0));
                if (level > 17) {
                    break;
                }
                target->guild_levelup_params[level - 1].value_04 =
                    ScanInt(static_cast<bool*>(0));
                target->guild_levelup_params[level - 1].value_00 =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[guild point acquirement ratio]") {
            target->guild_point_acquirement_ratio =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[guild exp prob]") {
            target->guild_exp_prob_count =
                static_cast<int8_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[guild exp weight]") {
            if (!target->guild_exp_prob_count) {
                break;
            }
            for (int index = 0; target->guild_exp_prob_count > index;
                 ++index) {
                const int start = ScanInt(static_cast<bool*>(0));
                const int end = ScanInt(static_cast<bool*>(0));
                const int weight = ScanInt(static_cast<bool*>(0));
                for (int level = start; level <= end; ++level) {
                    std::pair<std::map<int, int>::iterator, bool> result =
                        target->guild_exp_weight.insert(
                            std::make_pair(level, weight));
                    if (result.second != true) {
                        break;
                    }
                }
            }
        } else if (tag == "[guild mail]") {
            target->guild_mail = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[guild level demand for agit]") {
            target->guild_level_demand_for_agit =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[guild exp book]") {
            for (int index = 0; index <= 2; ++index) {
                const int key = ScanInt(static_cast<bool*>(0));
                const int value = ScanInt(static_cast<bool*>(0));
                target->guild_exp_book.insert(std::make_pair(key, value));
            }
        } else if (tag ==
                   "[guild level demand for buying guild exp book]") {
            target->guild_level_demand_for_buying_exp_book =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[agit upgrade info]") {
            STAgitUpgradeInfo info;
            for (int index = 0; index <= 4; ++index) {
                info.values[0] = ScanInt(static_cast<bool*>(0));
                info.values[1] = ScanInt(static_cast<bool*>(0));
                info.values[2] = ScanInt(static_cast<bool*>(0));
                info.values[3] = ScanInt(static_cast<bool*>(0));
                target->agit_upgrade_info.push_back(info);
            }
        } else if (tag == "[guild storage upgrade info]") {
            STGuildStorageUpgradeInfo info;
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                info.values[0] = value;
                info.values[1] = ScanInt(static_cast<bool*>(0));
                info.values[2] = ScanInt(static_cast<bool*>(0));
                info.values[3] = ScanInt(static_cast<bool*>(0));
                info.values[4] = ScanInt(static_cast<bool*>(0));
                target->guild_storage_upgrade_info.push_back(info);
            }
        } else if (tag == "[guild exp member weight]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                const double weight = ScanFloat(static_cast<bool*>(0));
                target->guild_exp_member_weight.push_back(
                    std::make_pair(value, weight));
            }
        } else if (tag == "[guild levelup price]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                std::vector<int> prices;
                prices.reserve(0x11u);
                for (int index = 0; index <= 16; ++index) {
                    prices.push_back(ScanInt(static_cast<bool*>(0)));
                }
                target->guild_levelup_price.push_back(
                    std::make_pair(value, prices));
            }
        } else if (tag == "[today guild member weight]") {
            target->today_guild_member_weight =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[guild fund condition]") {
            target->guild_fund_condition[0] = ScanInt(static_cast<bool*>(0));
            target->guild_fund_condition[1] = ScanInt(static_cast<bool*>(0));
            target->guild_fund_condition[2] = ScanInt(static_cast<bool*>(0));
            target->guild_fund_condition_weight =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[guild parameter end]") {
            success = true;
            break;
        }
    }

    return success ? 1 : 0;
}
