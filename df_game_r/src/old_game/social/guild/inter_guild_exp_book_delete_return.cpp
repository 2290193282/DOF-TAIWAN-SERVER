#include "old_game/social/guild/inter_guild_exp_book_delete_return.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/db/pvp_party/db_guild_exp_book_delete.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketHeader.h"

namespace {

#pragma pack(push, 1)
struct PacketNoCache {
    PacketNoCache() : header(0x1b6du, sizeof(PacketNoCache)), user_uid(0),
                      server_group(0), reserved(0) {}

    PacketHeader header;
    uint32_t user_uid;
    uint32_t server_group;
    uint32_t reserved;
};
#pragma pack(pop)

static_assert(sizeof(PacketNoCache) == 0x16u,
              "Packet_No_Cache must preserve its original 22-byte wire ABI");

uint32_t ReadU32(const char* data, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, data + offset, sizeof(value));
    return value;
}

void ForwardNoCache(uint32_t user_uid, uint32_t server_group) {
    if (!GlobalData::s_monitor_proxy_mgr) {
        return;
    }

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(server_group));
    if (!proxy) {
        return;
    }

    PacketNoCache packet;
    packet.user_uid = user_uid;
    packet.server_group = server_group;
    proxy->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}

}  // namespace

// IDA_EA: 0x84E2F1E
// IDA_NAME: _ZN24Inter_GuildExpBookDelete12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildExpBookDelete::dispatch_sig(CUser *,char *,int)
int Inter_GuildExpBookDelete::dispatch_sig(CUser*, char* payload,
                                           int payload_size) {
    if (!payload || payload_size < 26) {
        return 0;
    }

    const uint32_t target_uid = ReadU32(payload, 10);
    const uint32_t charac_no = ReadU32(payload, 14);
    const uint32_t no_cache_uid = ReadU32(payload, 18);
    const uint32_t server_group = ReadU32(payload, 22);
    CUser* const target = G_CGameManager()->getUser(
        static_cast<int>(target_uid), 0);

    if (target && target->getCurCharacR()) {
        DB_GuildExpBookDelete::makeRequest(
            charac_no, 0, static_cast<ENUM_SERVER_GROUP>(server_group));
        target->deleteGuildExpBook();
        return 0;
    }

    DB_GuildExpBookDelete::makeRequest(
        charac_no, 1, static_cast<ENUM_SERVER_GROUP>(server_group));
    ForwardNoCache(no_cache_uid, server_group);
    return 0;
}

// IDA_EA: 0x84EE90C / 0x84F69D8 / 0x84F6A08
Inter_GuildExpBookDelete::Inter_GuildExpBookDelete() : ISigDispatcher() {}
Inter_GuildExpBookDelete::~Inter_GuildExpBookDelete() {}
