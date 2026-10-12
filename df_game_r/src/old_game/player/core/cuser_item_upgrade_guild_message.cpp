#include "old_game/player/core/cuser.h"

#include <stdio.h>
#include <string.h>

#include <string>

#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/server_xml_runtime.h"

namespace {

const size_t kGuildBoardMessageCapacity = 120u;

bool BuildUpgradeGuildBoardMessage(char* output,
                                   const char* character_name,
                                   const char* item_name,
                                   int displayed_upgrade,
                                   int suffix_string_id) {
    std::string message(character_name);
    std::string body = g_ServerString_.GetServerString(1000, 0);
    const std::string suffix =
        g_ServerString_.GetServerString(suffix_string_id, 0);

    char upgrade_text[255];
    ::memset(upgrade_text, 0, sizeof(upgrade_text));
    ::sprintf(upgrade_text, "+%d %s", displayed_upgrade, item_name);

    body += upgrade_text;
    body += suffix;
    message += body;
    if (message.size() > 0x77u) {
        return false;
    }

    ::memset(output, 0, kGuildBoardMessageCapacity);
    ::memcpy(output, message.data(), message.size());
    return true;
}

}  // namespace

// IDA_EA: 0x8679CEE
// IDA_NAME: _ZN5CUser26makeGuildItemUpgradMessageEPKciib
// DEMANGLED: CUser::makeGuildItemUpgradMessage(char const*,int,int,bool)
void CUser::makeGuildItemUpgradMessage(const char* item_name,
                                       int old_upgrade,
                                       int new_upgrade,
                                       bool succeeded) {
    char board_message[kGuildBoardMessageCapacity];
    ::memset(board_message, 0, sizeof(board_message));

    bool ready = false;
    if (succeeded && new_upgrade > 10) {
        ready = BuildUpgradeGuildBoardMessage(
            board_message, getCurCharacName(), item_name, new_upgrade, 1004);
    } else if (!succeeded && old_upgrade > 10) {
        ready = BuildUpgradeGuildBoardMessage(
            board_message, getCurCharacName(), item_name, old_upgrade + 1,
            1005);
    }
    if (!ready) {
        return;
    }

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
    guild_proxy->SendGuildBoardWrite(
        board_message, 0u, static_cast<unsigned int>(getCurCharacNo()), -1,
        static_cast<char>(-1),
        static_cast<unsigned int>(get_charac_guildkey()));
}

// IDA_EA: 0x8679754
// IDA_NAME: _ZN5CUser23makeGuildLevelUpMessageEi
// DEMANGLED: CUser::makeGuildLevelUpMessage(int)
void CUser::makeGuildLevelUpMessage(int charac_level) {
    if (charac_level % 10) {
        return;
    }

    char board_message[kGuildBoardMessageCapacity];
    ::memset(board_message, 0, sizeof(board_message));

    std::string message(getCurCharacName());
    std::string body = g_ServerString_.GetServerString(1000, 0);
    const std::string suffix =
        g_ServerString_.GetServerString(1001, 0);

    char level_text[255];
    ::memset(level_text, 0, sizeof(level_text));
    ::sprintf(level_text, "%d", charac_level);
    body += level_text;
    body += suffix;
    message += body;

    if (message.size() > 0x77u) {
        return;
    }

    ::memcpy(board_message, message.data(), message.size());
    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
    guild_proxy->SendGuildBoardWrite(
        board_message,
        0u,
        static_cast<unsigned int>(getCurCharacNo()),
        -1,
        static_cast<char>(-1),
        static_cast<unsigned int>(get_charac_guildkey()));
}
