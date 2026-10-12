#include "old_game/social/guild/guild_server_proxy_abi.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x846EB0E
// IDA_NAME: _ZN17CGuildServerProxy26ChangeGuildAgitFlagInProxyEjh
// DEMANGLED: CGuildServerProxy::ChangeGuildAgitFlagInProxy(unsigned int,unsigned char)
void CGuildServerProxy::ChangeGuildAgitFlagInProxy(
    unsigned int charac_guildkey,
    unsigned char guild_agit_flag) {
    std::map<unsigned int, STGuildProxy>::iterator found =
        guild_map_.find(charac_guildkey);
    if (found == guild_map_.end()) {
        LogManager::logFormat(
            1, "DF_GuildServerProxy.cpp",
            "void CGuildServerProxy::ChangeGuildAgitFlagInProxy(unsigned int, unsigned char)",
            0x86e, "Find fail from guild info proxy(guild_id:%d)",
            charac_guildkey);
    } else {
        // Despite the exported function name, IDA 0x846EB0E writes byte
        // +0x18.  The DB loader and CUser::SetGuildInfoProxy prove that this
        // physical byte is power_side; +0x19 is the actual guild-agit flag.
        // Preserve the original write target rather than correcting its
        // historical semantic mismatch.
        found->second.power_side = guild_agit_flag;
    }
}
