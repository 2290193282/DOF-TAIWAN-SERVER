#include "old_game/player/core/cuser.h"

#include <stdio.h>
#include <string.h>

#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_data/PVF/common/rdar_script.h"

namespace {

const size_t kGuildBoardMessageCapacity = 120u;

bool BuildGuildBoardMessage(char* output,
                            const char* character_name,
                            const char* prefix,
                            const char* notice,
                            const char* item_name,
                            int separate_upgrade,
                            int normal_upgrade) {
    char upgrade_text[255];
    ::memset(upgrade_text, 0, sizeof(upgrade_text));
    if (normal_upgrade != 0) {
        ::snprintf(upgrade_text, sizeof(upgrade_text), "+%d %s %d",
                   normal_upgrade, item_name, separate_upgrade);
    } else {
        ::snprintf(upgrade_text, sizeof(upgrade_text), "%s %d", item_name,
                   separate_upgrade);
    }

    const std::string message = std::string(character_name) + prefix +
                                upgrade_text + notice;
    if (message.size() > 0x77u) {
        return false;
    }
    ::memset(output, 0, kGuildBoardMessageCapacity);
    ::memcpy(output, message.data(), message.size());
    return true;
}

}  // namespace

// IDA_EA: 0x8684D0A
// IDA_NAME: _ZN15cUserHistoryLog19SeparateUpgradeItemEiiib
// DEMANGLED: cUserHistoryLog::SeparateUpgradeItem(int,int,int,bool)
int cUserHistoryLog::SeparateUpgradeItem(int item_index,
                                         int old_separate_upgrade,
                                         int new_separate_upgrade,
                                         bool succeeded) {
    return succeeded
        ? owner_->LogHistory("UpgradeSeparate+,%d,%d,%d", item_index,
                             old_separate_upgrade, new_separate_upgrade)
        : owner_->LogHistory("UpgradeSeparate-,%d,%d,%d", item_index,
                             old_separate_upgrade, new_separate_upgrade);
}

// IDA_EA: 0x867A242
// IDA_NAME: _ZN5CUser34makeGuildItemSeparateUpgradMessageEPKciibi
// DEMANGLED: CUser::makeGuildItemSeparateUpgradMessage(char const*,int,int,bool,int)
void CUser::makeGuildItemSeparateUpgradMessage(const char* item_name,
                                               int old_separate_upgrade,
                                               int new_separate_upgrade,
                                               bool succeeded,
                                               int normal_upgrade) {
    char board_message[kGuildBoardMessageCapacity];
    ::memset(board_message, 0, sizeof(board_message));

    char prefix[11];
    ::memset(prefix, 0, sizeof(prefix));
    ::strncpy(prefix,
              g_scriptStringManager_.findString(4, "game_server_msg_247", 0),
              10u);

    bool ready = false;
    if (succeeded && new_separate_upgrade > 5) {
        ready = BuildGuildBoardMessage(
            board_message, getCurCharacName(), prefix,
            g_scriptStringManager_.findString(4, "game_server_msg_248", 0),
            item_name, new_separate_upgrade, normal_upgrade);
    } else if (!succeeded && old_separate_upgrade > 5) {
        ready = BuildGuildBoardMessage(
            board_message, getCurCharacName(), prefix,
            g_scriptStringManager_.findString(4, "game_server_msg_249", 0),
            item_name, old_separate_upgrade + 1, normal_upgrade);
    }
    if (!ready) {
        return;
    }

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
    guild_proxy->SendGuildBoardWrite(
        board_message, 0u, static_cast<unsigned int>(getCurCharacNo()), -1,
        static_cast<char>(-1), static_cast<unsigned int>(get_charac_guildkey()));
}
