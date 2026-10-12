#include "old_game/social/guild/packet_dispatcher_guild_join_list.h"

#include "core/config/env/environment.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x8597506
// IDA_NAME: _ZN24Dispatcher_GuildJoinListC2Ev
// DEMANGLED: Dispatcher_GuildJoinList::Dispatcher_GuildJoinList(void)
Dispatcher_GuildJoinList::Dispatcher_GuildJoinList() {}

// IDA_EA: 0x82450BE
// IDA_NAME: _ZN24Dispatcher_GuildJoinListD2Ev
// DEMANGLED: Dispatcher_GuildJoinList::~Dispatcher_GuildJoinList()
// IDA_EA: 0x82450EE
// IDA_NAME: _ZN24Dispatcher_GuildJoinListD0Ev
// DEMANGLED: Dispatcher_GuildJoinList::~Dispatcher_GuildJoinList()
Dispatcher_GuildJoinList::~Dispatcher_GuildJoinList() {}

// IDA_EA: 0x81D529C
// IDA_NAME: _ZN24Dispatcher_GuildJoinList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildJoinList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildJoinList::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D52A6
// IDA_NAME: _ZN24Dispatcher_GuildJoinList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildJoinList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildJoinList::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)message;
    (void)param;

    if (user->get_state() != 3) {
        return 0;
    }
    if (!user->IsGuildMaster() && !user->IsSubGuildMaster()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_JOIN_LIST, 36);
        return 0;
    }

    ENUM_SERVER_GROUP server_group;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        server_group =
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup());
    } else {
        server_group = static_cast<ENUM_SERVER_GROUP>(
            G_CEnvironment()->get_server_group());
    }

    DB_GuildJoinList::makeRequest(
        user->GetUID(),
        static_cast<unsigned int>(user->get_charac_guildkey()),
        server_group);
    return 0;
}
