#ifndef OLDSRV_OLD_DATA_PVF_ETC_GUILD_PARAMETER_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_GUILD_PARAMETER_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <utility>
#include <vector>

#include "old_data/PVF/etc/guild_parameter_types.h"


// IDA_EA: 0x83704AE, 0x837058A, 0x837D542, 0x89795DE, 0x8979648, 0x8979672, 0x89796DC, 0x8979726, 0x89797A4, 0x8979864, 0x8978D48.
struct STGuildLevelUpParam {
    int32_t value_00;
    int32_t value_04;
};

// IDA unit: 12_pvf_datamanager_scripts/GuildParameterScript.
class GuildParameterScript {
public:
    GuildParameterScript();
    ~GuildParameterScript();

    GuildParameterScript* clear();
    int32_t getGuildExpWeight(int32_t level);
    STGuildLevelUpParam* getGuildLevelUpParam(int32_t level);
    int32_t getGuildExpBook(int32_t value);
    STAgitUpgradeInfo* getGuildAgitUpgradeInfo(int32_t value);
    STGuildStorageUpgradeInfo* getGuildCargoUpradeInfo(int32_t value);
    int32_t getLevelupPrice(int32_t index, int32_t value);
    long double getExpWeight(int32_t value);

    int32_t guild_create_cost;
    int32_t guild_point_acquirement_ratio;
    int8_t guild_exp_prob_count;
    int32_t guild_level_demand_for_agit;
    std::map<int, int> guild_exp_book;
    int32_t guild_level_demand_for_buying_exp_book;
    std::vector<STAgitUpgradeInfo> agit_upgrade_info;
    std::vector<STGuildStorageUpgradeInfo> guild_storage_upgrade_info;
    std::map<int, int> guild_exp_weight;
    STGuildLevelUpParam guild_levelup_params[17];
    std::vector<std::pair<int, double> > guild_exp_member_weight;
    std::vector<std::pair<int, std::vector<int> > > guild_levelup_price;
    double today_guild_member_weight;
    int32_t guild_fund_condition[3];
    double guild_fund_condition_weight;
    int32_t guild_mail;
};

int importGuildParameterScript(GuildParameterScript* script,
                               const char* filename);

static_assert(sizeof(STGuildLevelUpParam) == 0x08u,
              "STGuildLevelUpParam IDA ABI size must stay 0x08");
static_assert(sizeof(GuildParameterScript) == 0x11Cu,
              "GuildParameterScript IDA ABI size must stay 0x11C");
static_assert(offsetof(GuildParameterScript, guild_exp_book) == 0x10u &&
                  offsetof(GuildParameterScript, agit_upgrade_info) == 0x2cu &&
                  offsetof(GuildParameterScript, guild_storage_upgrade_info) == 0x38u &&
                  offsetof(GuildParameterScript, guild_exp_weight) == 0x44u &&
                  offsetof(GuildParameterScript, guild_levelup_params) == 0x5cu &&
                  offsetof(GuildParameterScript, guild_exp_member_weight) == 0xe4u &&
                  offsetof(GuildParameterScript, guild_levelup_price) == 0xf0u,
              "GuildParameterScript IDA container offsets must stay exact");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_GUILD_PARAMETER_SCRIPT_H_
