#include "old_game/social/guild/inter_guild_pending_return_dispatchers.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/social/guild/guild_monitor_state_dispatchers_a.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

// The incoming Inter payload begins with the original ten-byte server packet
// header.  The following fields and offsets are the ones consumed by the old
// 0x84D2B96 response body.
#pragma pack(push, 1)
struct GuildInviteToInvitedPayload {
    uint8_t server_packet_header[10];
    uint32_t target_charac_no;       // +0x0a
    uint32_t inviter_guild_key;      // +0x0e
    char inviter_name[30];           // +0x12
    char guild_name[30];             // +0x30
};

struct GuildSecedeReplyPayload {
    uint8_t server_packet_header[10];
    uint32_t guild_key;              // +0x0a
    uint16_t guild_member_index;     // +0x0e
    uint32_t target_charac_no;       // +0x10
    uint32_t reserved_14;            // +0x14
    uint32_t result_code;            // +0x18
    uint8_t guild_member_grade;      // +0x1c
    char charac_name[30];            // +0x1d
    char guild_name[30];             // +0x3b
};

struct GuildCreateReplyPayload {
    uint8_t server_packet_header[10];
    uint32_t target_charac_no;       // +0x0a
    uint32_t reserved_0e;            // +0x0e
    uint32_t result_code;            // +0x12
    char guild_proxy_name[22];       // +0x16
};
#pragma pack(pop)

static_assert(offsetof(GuildInviteToInvitedPayload, target_charac_no) ==
                  0x0au &&
                  offsetof(GuildInviteToInvitedPayload, inviter_name) ==
                      0x12u &&
                  offsetof(GuildInviteToInvitedPayload, guild_name) == 0x30u,
              "guild-invite return payload offsets must retain old ABI");
static_assert(offsetof(GuildSecedeReplyPayload, target_charac_no) == 0x10u &&
                  offsetof(GuildSecedeReplyPayload, result_code) == 0x18u &&
                  offsetof(GuildSecedeReplyPayload, guild_member_grade) ==
                      0x1cu &&
                  offsetof(GuildSecedeReplyPayload, charac_name) == 0x1du &&
                  offsetof(GuildSecedeReplyPayload, guild_name) == 0x3bu,
              "guild-secede return payload offsets must retain old ABI");
static_assert(offsetof(GuildCreateReplyPayload, target_charac_no) == 0x0au &&
                  offsetof(GuildCreateReplyPayload, result_code) == 0x12u &&
                  offsetof(GuildCreateReplyPayload, guild_proxy_name) ==
                      0x16u,
              "guild-create return payload offsets must retain old ABI");

CGuildServerProxy* GuildProxyFor(CUser* user) {
    return GlobalData::s_guild_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
}

void LogCharacterMismatch(const char* function_name,
                          int source_line,
                          const char* text,
                          CUser* user,
                          uint32_t expected_charac_no) {
    LogManager::logFormat(1,
                          "InterDispatcher.cpp",
                          function_name,
                          source_line,
                          text,
                          user->get_charac_no(-1),
                          expected_charac_no);
}

}  // namespace

// IDA_EA: 0x84D2B96
// IDA_NAME: _ZN30Inter_CallGuildInviteToInvited12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CallGuildInviteToInvited::dispatch_sig(CUser *,char *,int)
int Inter_CallGuildInviteToInvited::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int) {
    if (!user || !payload || !user->get_state()) {
        return 0;
    }

    const GuildInviteToInvitedPayload& reply =
        *reinterpret_cast<const GuildInviteToInvitedPayload*>(payload);
    if (static_cast<uint32_t>(user->get_charac_no(-1)) !=
        reply.target_charac_no) {
        LogCharacterMismatch(
            "virtual int Inter_CallGuildInviteToInvited::dispatch_sig(CUser*, char*, int)",
            0x337b,
            "MEMBER : Inter_CallGuildInviteToInvited::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user,
            reply.target_charac_no);
        return 0;
    }

    if (user->alonePlay_Mode()) {
        GuildProxyFor(user)->SendReplyGuildInvite(
            static_cast<unsigned int>(user->getCurCharacNo()), 2u);
        return 0;
    }

    if (user->GetGameOptionRef()->get_etc_option(0x1e) != 0) {
        GuildProxyFor(user)->SendReplyGuildInvite(
            static_cast<unsigned int>(user->getCurCharacNo()), 3u);
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_CALL_GUILD_INVITE_TO_INVITED);
    const int guild_name_length = static_cast<int>(std::strlen(reply.guild_name));
    packet.put_int(guild_name_length);
    packet.put_str(reply.guild_name, guild_name_length);
    const int inviter_name_length =
        static_cast<int>(std::strlen(reply.inviter_name));
    packet.put_int(inviter_name_length);
    packet.put_str(reply.inviter_name, inviter_name_length);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84ED984 / 0x84F9396 / 0x84F93C6
Inter_CallGuildInviteToInvited::Inter_CallGuildInviteToInvited()
    : ISigDispatcher() {}
Inter_CallGuildInviteToInvited::~Inter_CallGuildInviteToInvited() {}

// IDA_EA: 0x84C9768
// IDA_NAME: _ZN22Inter_ReplyGuildSecede12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReplyGuildSecede::dispatch_sig(CUser *,char *,int)
int Inter_ReplyGuildSecede::dispatch_sig(CUser* user, char* payload, int) {
    if (!user || !payload || !user->get_state()) {
        return 0;
    }

    const GuildSecedeReplyPayload& reply =
        *reinterpret_cast<const GuildSecedeReplyPayload*>(payload);
    if (static_cast<uint32_t>(user->get_charac_no(-1)) !=
        reply.target_charac_no) {
        LogCharacterMismatch(
            "virtual int Inter_ReplyGuildSecede::dispatch_sig(CUser*, char*, int)",
            0x236b,
            "Inter_ReplyGuildSecede::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user,
            reply.target_charac_no);
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REQ_GUILD_SECEDE);
    if (reply.result_code != 0) {
        packet.put_byte(0);
        packet.put_byte(static_cast<unsigned char>(reply.result_code));
        packet.finalize(true);
        user->Send(packet);
    } else {
        packet.put_byte(1);
        packet.put_byte(reply.guild_member_grade);
        const int guild_name_length =
            static_cast<int>(std::strlen(reply.guild_name));
        packet.put_int(guild_name_length);
        packet.put_str(reply.guild_name, guild_name_length);
        const int charac_name_length =
            static_cast<int>(std::strlen(reply.charac_name));
        packet.put_int(charac_name_length);
        packet.put_str(reply.charac_name, charac_name_length);
        packet.finalize(true);
        user->Send(packet);

        CUser_history_log(user)->GuildOut(
            reply.guild_key,
            reply.guild_name,
            reply.target_charac_no,
            static_cast<char>(reply.guild_member_grade),
            static_cast<unsigned short>(reply.guild_member_index - 1u));
    }

    // The original resends every lock list for both result branches.
    user->getCurCharacInvenR()->SendItemLockListInven();
    user->getCurCharacInvenR()->SendItemLockListAvatar();
    user->getCurCharacCargoR()->SendItemLockList();
    user->getCurCharacInvenR()->SendItemLockListCreature();
    return 0;
}

// IDA_EA: 0x84EDA48 / 0x84FA19A / 0x84FA1CA
Inter_ReplyGuildSecede::Inter_ReplyGuildSecede() : ISigDispatcher() {}
Inter_ReplyGuildSecede::~Inter_ReplyGuildSecede() {}

// IDA_EA: 0x84D368A
// IDA_NAME: _ZN22Inter_ReplyGuildCreate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReplyGuildCreate::dispatch_sig(CUser *,char *,int)
int Inter_ReplyGuildCreate::dispatch_sig(CUser* user, char* payload, int) {
    if (!user || !payload) {
        return 0;
    }

    const GuildCreateReplyPayload& reply =
        *reinterpret_cast<const GuildCreateReplyPayload*>(payload);
    if (user->get_state() <= 2) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_ReplyGuildCreate::dispatch_sig(CUser*, char*, int)",
            0x3464,
            "GUILD : Inter_ReplyGuildCreate:: recv->m_uCharID(%d)",
            reply.target_charac_no);
        return 0;
    }

    if (static_cast<uint32_t>(user->get_charac_no(-1)) !=
        reply.target_charac_no) {
        LogCharacterMismatch(
            "virtual int Inter_ReplyGuildCreate::dispatch_sig(CUser*, char*, int)",
            0x346a,
            "GUILD : Inter_ReplyGuildCreate:: : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user,
            reply.target_charac_no);
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT);
    if (reply.result_code != 0) {
        packet.put_byte(0);
        packet.put_byte(static_cast<unsigned char>(reply.result_code));

        const int guild_create_cost =
            G_CDataManager()->GetGuildParameterScriptForBuild()
                ->guild_create_cost;
        user->getCurCharacInvenW()->gain_money(
            guild_create_cost, static_cast<eMoneyAddReason>(25), true, 0);
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 0);
    } else {
        packet.put_byte(1);
        CUser_history_log(user)->GuildCreate(
            static_cast<unsigned int>(user->get_charac_guildkey()),
            reply.guild_proxy_name,
            static_cast<unsigned int>(user->get_charac_no(-1)));
    }
    packet.finalize(true);
    user->Send(packet);

    if (reply.result_code != 0) {
        return 0;
    }

    // Signal 237 confirms only the operation result and guild name; its
    // fixed 0x2d payload has no guild-key field.  If signal 97/98 won the
    // queue race, replay that already-authoritative cached key immediately.
    // Otherwise leave the result pending: the newly restored 0x040b -> 97
    // route (or a later 91/198 member snapshot) will supply and project it.
    const uint32_t authoritative_key =
        static_cast<uint32_t>(user->get_charac_guildkey());
    if (authoritative_key != 0) {
        GuildKeySync::SynchronizeAuthoritativeNonZero(
            user,
            reply.target_charac_no,
            authoritative_key,
            GuildKeySync::kConfirmation);
    } else {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_ReplyGuildCreate::dispatch_sig(CUser*, char*, int)",
            0x346b,
            "[GUILD][CREATE_SYNC_PENDING] charac_no=%u signal237_has_no_guild_key=1",
            static_cast<unsigned int>(reply.target_charac_no));
    }

    STGuildProxy guild_proxy;
    guild_proxy.guild_name[sizeof(guild_proxy.guild_name) - 1u] = '\0';
    std::memcpy(guild_proxy.guild_name,
                reply.guild_proxy_name,
                sizeof(guild_proxy.guild_name) - 1u);
    guild_proxy.guild_level = 0;
    guild_proxy.power_side = 0;
    user->SetGuildInfoProxy(guild_proxy);

    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
    return 0;
}

// IDA_EA: 0x84EDAF0 / 0x84F90D8 / 0x84F9108
Inter_ReplyGuildCreate::Inter_ReplyGuildCreate() : ISigDispatcher() {}
Inter_ReplyGuildCreate::~Inter_ReplyGuildCreate() {}
