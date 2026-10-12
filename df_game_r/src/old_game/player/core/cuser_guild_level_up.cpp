#include "old_game/player/core/cuser.h"

#include <stdint.h>

#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/guild_parameter_script.h"

// IDA_EA: 0x866C27E
// IDA_NAME: _ZN5CUser17CheckGuildLevelUpERj
// DEMANGLED: CUser::CheckGuildLevelUp(unsigned int&)
int CUser::CheckGuildLevelUp(unsigned int& levelup_price) {
    if (!get_charac_guildkey()) {
        return 34;
    }
    if (!IsGuildMaster() &&
        static_cast<unsigned char>(IsSubGuildMaster()) != 1u) {
        return 83;
    }

    const int next_guild_level = guild_info_.guild_level + 1;
    if (next_guild_level > 16) {
        return 37;
    }

    GuildParameterScript* const parameters =
        G_CDataManager()->GetGuildParameterScriptForBuild();
    STGuildLevelUpParam* const level_parameter =
        parameters->getGuildLevelUpParam(next_guild_level);
    if (!level_parameter) {
        LogManager::logFormat(
            5,
            "unsigned int CUser::CheckGuildLevelUp(unsigned int&)",
            "CUser::CheckGuildLevelUp",
            21442,
            "CUser::CheckGuildLevelUp Guild Param Err(g lev : %d)",
            guild_info_.guild_level);
        return 34;
    }

    const int price =
        parameters->getLevelupPrice(next_guild_level,
                                    guild_info_.guild_member_count);
    if (price > static_cast<int>(guild_info_.guild_fund)) {
        return 22;
    }

    levelup_price = static_cast<unsigned int>(price);
    return static_cast<uint32_t>(level_parameter->value_04) <=
                   guild_info_.guild_experience
               ? 0
               : 84;
}
