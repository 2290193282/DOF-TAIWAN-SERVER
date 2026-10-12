#include "old_game/player/core/cuser.h"

#include <cstring>
#include <ctime>

#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x865CD0A
// IDA_NAME: _ZN5CUser17SetGuildInfoProxyER12STGuildProxy
// DEMANGLED: CUser::SetGuildInfoProxy(STGuildProxy &)
CUser* CUser::SetGuildInfoProxy(STGuildProxy& info) {
    guild_info_.guild_level = info.guild_level;
    std::memcpy(guild_info_.guild_name,
                info.guild_name,
                sizeof(info.guild_name) - 1u);
    guild_info_.power_side = info.power_side;
    return this;
}

// IDA_EA: 0x8685978
// IDA_NAME: _ZN15cUserHistoryLog11GuildCreateEjPKcj
// DEMANGLED: cUserHistoryLog::GuildCreate(unsigned int,char const*,unsigned int)
int cUserHistoryLog::GuildCreate(unsigned int guild_key,
                                 const char* guild_name,
                                 unsigned int charac_no) {
    std::tm local_time;
    const std::time_t current_time =
        static_cast<std::time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm* const time_info = localtime_r(&current_time, &local_time);
    return owner_->LogHistory("GuildCreate,\"%d/%d/%d\",%d,\"%s\",%d",
                              time_info->tm_year + 1900,
                              time_info->tm_mon + 1,
                              time_info->tm_mday,
                              guild_key,
                              guild_name,
                              charac_no);
}

// IDA_EA: 0x8685B00
// IDA_NAME: _ZN15cUserHistoryLog8GuildOutEjPKcjct
// DEMANGLED: cUserHistoryLog::GuildOut(unsigned int,char const*,unsigned int,char,unsigned short)
int cUserHistoryLog::GuildOut(unsigned int guild_key,
                              const char* guild_name,
                              unsigned int charac_no,
                              char guild_member_grade,
                              unsigned short guild_member_index) {
    std::tm local_time;
    const std::time_t current_time =
        static_cast<std::time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm* const time_info = localtime_r(&current_time, &local_time);
    return owner_->LogHistory("GuildOut,\"%d/%d/%d\",%d,\"%s\",%d,%d,%d",
                              time_info->tm_year + 1900,
                              time_info->tm_mon + 1,
                              time_info->tm_mday,
                              guild_key,
                              guild_name,
                              charac_no,
                              guild_member_grade,
                              guild_member_index);
}
