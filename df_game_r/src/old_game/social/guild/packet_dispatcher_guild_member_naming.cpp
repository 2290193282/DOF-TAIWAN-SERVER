#include "old_game/social/guild/packet_dispatcher_guild_member_naming.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kGuildMemberNamingSignature[] =
    "virtual int Dispatcher_GuildMemberNaming::dispatch_sig(CUser*, PacketBuf&)";

// Reconstructed 0828 Guild/Monitor request contract.
//
// Evidence boundary:
//   * the old inter table pairs naming/sub-master replies at signals 100/101;
//   * the proven sub-master request is the immediately following type 0x0430;
//   * both the old guild member wire and the self-memo request store the title
//     in a 21-byte NUL-terminated member_message/memo field.
// No 0627 df_game_r constructor survived for type 0x042f, so keep this packet
// local to CMD125 instead of presenting it as an IDA-exact legacy value type.
enum {
    kMonitorGuildMemberNamingType = 0x042f,
    kMonitorGuildMemberNamingSize = 0x0045,
};

#pragma pack(push, 1)
struct MonitorGuildMemberNamingRequest : PacketHeader {
    uint32_t requester_charac_no;  // +0x0a
    uint32_t guild_key;            // +0x0e
    char target_name[30];          // +0x12; max 29 payload bytes
    char member_message[21];       // +0x30; max 20 payload bytes

    MonitorGuildMemberNamingRequest()
        : PacketHeader(kMonitorGuildMemberNamingType,
                       kMonitorGuildMemberNamingSize),
          requester_charac_no(0),
          guild_key(0) {
        std::memset(target_name, 0, sizeof(target_name));
        std::memset(member_message, 0, sizeof(member_message));
    }
};
#pragma pack(pop)

static_assert(sizeof(MonitorGuildMemberNamingRequest) ==
                  kMonitorGuildMemberNamingSize,
              "CMD125 reconstructed monitor request size changed");
static_assert(offsetof(MonitorGuildMemberNamingRequest,
                       requester_charac_no) == 0x0au &&
                  offsetof(MonitorGuildMemberNamingRequest, guild_key) ==
                      0x0eu &&
                  offsetof(MonitorGuildMemberNamingRequest, target_name) ==
                      0x12u &&
                  offsetof(MonitorGuildMemberNamingRequest,
                           member_message) == 0x30u,
              "CMD125 reconstructed monitor request fields moved");

}  // namespace

Dispatcher_GuildMemberNaming::Dispatcher_GuildMemberNaming() {}
Dispatcher_GuildMemberNaming::~Dispatcher_GuildMemberNaming() {}

int Dispatcher_GuildMemberNaming::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(__LINE__, kGuildMemberNamingSignature, 0, 0);
    }

    MonitorGuildMemberNamingRequest request;
    request.requester_charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    request.guild_key =
        static_cast<uint32_t>(user->get_charac_guildkey());

    int target_name_length = 0;
    if (packet.get_int(target_name_length) != 1 ||
        target_name_length <= 0 ||
        packet.get_str(request.target_name,
                       static_cast<int>(sizeof(request.target_name)),
                       target_name_length) != 1 ||
        static_cast<int>(std::strlen(request.target_name)) !=
            target_name_length) {
        return LineFunc(__LINE__, kGuildMemberNamingSignature, 0, 0);
    }

    int member_message_length = 0;
    if (packet.get_int(member_message_length) != 1 ||
        member_message_length <= 0 ||
        packet.get_str(request.member_message,
                       static_cast<int>(sizeof(request.member_message)),
                       member_message_length) != 1 ||
        static_cast<int>(std::strlen(request.member_message)) !=
            member_message_length) {
        return LineFunc(__LINE__, kGuildMemberNamingSignature, 0, 0);
    }
    if (G_CDataManager()->hasPreventString(request.member_message) ||
        G_CDataManager()->hasSpecialCharacterExceptSpace(
            request.member_message)) {
        // Reuse the proven guild-member memo validation result instead of
        // inventing a CMD125-only business result that its client handler
        // cannot display.
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILDMEMBER_NAMING, 108);
        return 0;
    }

    // Authority, active membership, cross-channel presence, persistence and
    // result codes are one Guild/Monitor transaction.  In particular, do not
    // emit a local success: signal 100 owns the only proven CMD125 response.
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendPacket(reinterpret_cast<char*>(&request),
                      static_cast<int>(sizeof(request)));
    return 0;
}

int Dispatcher_GuildMemberNaming::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_GuildMemberNaming::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
