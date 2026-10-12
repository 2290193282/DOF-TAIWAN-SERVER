#include "old_game/player/core/cuser.h"

#include <cstring>
#include <cstdio>
#include <string>

#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/server_xml_runtime.h"

// IDA_EA: 0x8679A36
// IDA_NAME: _ZN5CUser21makeGuildSkillMessageEPKci
// DEMANGLED: CUser::makeGuildSkillMessage(char const*,int)
void CUser::makeGuildSkillMessage(const char* skill_name, int skill_level) {
    if (!skill_name) {
        return;
    }

    std::string message = g_ServerString_.GetServerString(1002, 0);
    char level_text[255];
    std::memset(level_text, 0, sizeof(level_text));
    std::snprintf(level_text, sizeof(level_text), "%d ", skill_level);
    message += level_text;
    message += skill_name;
    message += g_ServerString_.GetServerString(1003, 0);
    if (message.size() > 0x77u || !GlobalData::s_guild_proxy_mgr) {
        return;
    }

    char board_message[120];
    std::memset(board_message, 0, sizeof(board_message));
    std::memcpy(board_message, message.data(), message.size());
    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
    if (guild_proxy) {
        guild_proxy->SendGuildBoardWrite(
            board_message, 0u, static_cast<unsigned int>(getCurCharacNo()),
            -1, static_cast<char>(-1),
            static_cast<unsigned int>(get_charac_guildkey()));
    }
}
