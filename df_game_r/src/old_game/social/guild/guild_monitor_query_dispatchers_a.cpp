#include "old_game/social/guild/guild_monitor_query_dispatchers_a.h"

#include <cstddef>
#include <cstring>
#include <stdint.h>

#include "common/manager/global_tools.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/social/guild/guild_member_packet_runtime.h"
#include "old_game/social/guild/guild_monitor_state_dispatchers_a.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"

namespace {

// Every response handled in this unit starts with the ten-byte Monitor
// PacketHeader, then its four-byte routing uid and the recipient character
// number.  The raw dispatchers compare m_uCharID at +0x0e, not the routing uid
// at +0x0a.
#pragma pack(push, 1)
struct GuildMonitorRecipientPrefix {
    PacketHeader header;          // +0x00
    uint32_t routing_uid;         // +0x0a
    uint32_t charac_no;           // +0x0e, recv->m_uCharID
};

struct GuildMonitorGuildPrefix {
    GuildMonitorRecipientPrefix recipient;
    uint32_t guild_key;           // +0x12, recv->m_uGuildKey
};

// Packet_Monitor_Call_Guild_Members return prefix.  Member data starts at
// +0x34 and consists of count contiguous ST_Guild_Mem_Info records.
struct GuildMembersReplyPrefix {
    GuildMonitorGuildPrefix target;
    char guild_name[23];          // +0x16
    uint8_t member_count;         // +0x2d
    uint32_t member_list_cursor;  // +0x2e
    uint16_t member_list_page;    // +0x32
};

// Packet_Monitor_Notice_Guild_Char_Names return prefix.  The following wire
// tail has member_count fixed 30-byte names; unlike normal client strings the
// old sender copies all thirty bytes for every name.
struct GuildNamesReplyPrefix {
    GuildMonitorGuildPrefix target;
    uint8_t member_count;         // +0x16
};

// Packet_Monitor_Call_Guild_Info return prefix.  The response rows begin at
// +0x13 and are packed at the original 29-byte stride.
struct GuildInfoReplyPrefix {
    GuildMonitorRecipientPrefix recipient;
    uint8_t guild_count;          // +0x12
};

struct GuildInfoReplyRow {
    uint32_t guild_key;           // +0x00
    uint8_t guild_level;          // +0x04
    uint8_t guild_flag;           // +0x05
    char guild_name[23];          // +0x06
};

// Packet_Monitor_Notice_Guild_Info contains a live STGuildDBInfoOnly image
// at +0x16.  The one byte at +0xd3 is part of the Monitor record; the final
// text supplied to the client begins at +0xd4.
struct GuildInfoNoticeReplyPrefix {
    GuildMonitorGuildPrefix target;
    STGuildDBInfoOnly guild_info; // +0x16
    uint8_t reserved_d3;          // +0xd3
};
#pragma pack(pop)

static_assert(sizeof(GuildMonitorRecipientPrefix) == 0x12u,
              "Guild Monitor recipient prefix changed");
static_assert(sizeof(GuildMonitorGuildPrefix) == 0x16u,
              "Guild Monitor guild prefix changed");
static_assert(offsetof(GuildMembersReplyPrefix, guild_name) == 0x16u &&
                  offsetof(GuildMembersReplyPrefix, member_count) == 0x2du &&
                  sizeof(GuildMembersReplyPrefix) == 0x34u,
              "Guild-members reply offsets changed");
static_assert(sizeof(GuildNamesReplyPrefix) == 0x17u,
              "Guild-names reply prefix changed");
static_assert(sizeof(GuildInfoReplyPrefix) == 0x13u &&
                  sizeof(GuildInfoReplyRow) == 0x1du,
              "Guild-info reply row ABI changed");
static_assert(offsetof(GuildInfoNoticeReplyPrefix, guild_info) == 0x16u &&
                  sizeof(GuildInfoNoticeReplyPrefix) == 0xd4u,
              "Guild-info notice prefix changed");

}  // namespace

// IDA_EA: 0x84CA7E6
// IDA_NAME: _ZN29Inter_MonitorCallGuildMembers12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCallGuildMembers::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCallGuildMembers::dispatch_sig(CUser* user,
                                                char* payload,
                                                int) {
    if (!user->get_state()) {
        return 0;
    }

    GuildMembersReplyPrefix* const reply =
        reinterpret_cast<GuildMembersReplyPrefix*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(reply->target.recipient.charac_no)) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildMembers::dispatch_sig(CUser*, char*, int)",
            0x24b9,
            "Inter_MonitorCallGuildMembers::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1),
            reply->target.recipient.charac_no);
        return 0;
    }

    const GuildKeySync::Result key_sync =
        GuildKeySync::SynchronizeAuthoritativeNonZero(
            user,
            reply->target.recipient.charac_no,
            reply->target.guild_key,
            GuildKeySync::kSnapshotRepair);
    if (key_sync != GuildKeySync::kSynchronized) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildMembers::dispatch_sig(CUser*, char*, int)",
            0x24bf,
            "Inter_MonitorCallGuildMembers::dispatch_sig() : guild key sync rejected (cached=%u authoritative=%u result=%d)",
            static_cast<unsigned int>(user->get_charac_guildkey()),
            static_cast<unsigned int>(reply->target.guild_key),
            static_cast<int>(key_sync));
        return 0;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_GUILD_MEMER_LIST);
    packet.put_byte(1);
    packet.put_int(reply->target.guild_key);
    const int guild_name_length = static_cast<int>(std::strlen(reply->guild_name));
    packet.put_int(guild_name_length);
    packet.put_str(reply->guild_name, guild_name_length);
    packet.put_int(reply->member_list_cursor);
    packet.put_short(reply->member_list_page);
    ST_Guild_Mem_Info* const members = reinterpret_cast<ST_Guild_Mem_Info*>(
        payload + sizeof(GuildMembersReplyPrefix));
    FunctionGuildMember::MakePacketGuildMembersInfo(
        packet, static_cast<unsigned char>(reply->member_count), members);
    packet.finalize(true);
    user->Send(packet);
    FunctionGuildMember::SyncCurrentMemberGrade(user, reply->target.recipient.charac_no,
        reply->target.guild_key, static_cast<unsigned char>(reply->member_count), members);
    return 0;
}

// IDA_EA: 0x84EA77A / 0x84EA796 / 0x84EA7C6
// IDA_NAME: _ZN29Inter_MonitorCallGuildMembersC2Ev / D2Ev / D0Ev
Inter_MonitorCallGuildMembers::Inter_MonitorCallGuildMembers()
    : ISigDispatcher() {}
Inter_MonitorCallGuildMembers::~Inter_MonitorCallGuildMembers() {}

// IDA_EA: 0x84CAFCE
// IDA_NAME: _ZN29Inter_MonitorNoticeGuildNames12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildNames::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildNames::dispatch_sig(CUser* user,
                                                char* payload,
                                                int) {
    if (!user->get_state()) {
        return 0;
    }

    GuildNamesReplyPrefix* const reply =
        reinterpret_cast<GuildNamesReplyPrefix*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(reply->target.recipient.charac_no)) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorNoticeGuildNames::dispatch_sig(CUser*, char*, int)",
            0x255f,
            "Packet_Monitor_Notice_Guild_Char_Names::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1),
            reply->target.recipient.charac_no);
        return 0;
    }

    if (user->get_charac_guildkey() != static_cast<int>(reply->target.guild_key)) {
        // This message names CallGuildMembers in the original binary, even
        // though it is emitted from the GuildNames dispatcher.
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorNoticeGuildNames::dispatch_sig(CUser*, char*, int)",
            0x2565,
            "Inter_MonitorCallGuildMembers::dispatch_sig() : pUser->get_charac_guildkey() != recv->m_uGuildKey");
        return 0;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GUILD_CHAR_NAMES);
    packet.put_byte(static_cast<unsigned char>(reply->member_count));
    const char* const member_names = payload + sizeof(GuildNamesReplyPrefix);
    for (int index = 0; index < static_cast<unsigned char>(reply->member_count);
         ++index) {
        packet.put_str(member_names + 30 * index, 30);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EA922 / 0x84EA93E / 0x84EA96E
// IDA_NAME: _ZN29Inter_MonitorNoticeGuildNamesC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeGuildNames::Inter_MonitorNoticeGuildNames()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildNames::~Inter_MonitorNoticeGuildNames() {}

// IDA_EA: 0x84CC006
// IDA_NAME: _ZN26Inter_MonitorCallGuildInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCallGuildInfo::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCallGuildInfo::dispatch_sig(CUser* user,
                                             char* payload,
                                             int) {
    if (!user->get_state()) {
        return 0;
    }

    GuildInfoReplyPrefix* const reply =
        reinterpret_cast<GuildInfoReplyPrefix*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(reply->recipient.charac_no)) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildInfo::dispatch_sig(CUser*, char*, int)",
            0x26e4,
            "Inter_MonitorCallGuildInfo::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1),
            reply->recipient.charac_no);
        return 0;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET, ENUM_CMDPACKET_GUILD_INFO);
    packet.put_byte(1);
    packet.put_byte(static_cast<unsigned char>(reply->guild_count));
    const GuildInfoReplyRow* const guilds =
        reinterpret_cast<const GuildInfoReplyRow*>(payload + sizeof(GuildInfoReplyPrefix));
    for (int index = 0; index < static_cast<unsigned char>(reply->guild_count);
         ++index) {
        const GuildInfoReplyRow& guild = guilds[index];
        packet.put_int(guild.guild_key);
        packet.put_byte(static_cast<unsigned char>(guild.guild_level));
        packet.put_byte(static_cast<unsigned char>(guild.guild_flag));
        const int guild_name_length = static_cast<int>(std::strlen(guild.guild_name));
        packet.put_int(guild_name_length);
        packet.put_str(guild.guild_name, guild_name_length);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EA9F6 / 0x84EAA12 / 0x84EAA42
// IDA_NAME: _ZN26Inter_MonitorCallGuildInfoC2Ev / D2Ev / D0Ev
Inter_MonitorCallGuildInfo::Inter_MonitorCallGuildInfo()
    : ISigDispatcher() {}
Inter_MonitorCallGuildInfo::~Inter_MonitorCallGuildInfo() {}

// IDA_EA: 0x84CB726
// IDA_NAME: _ZN28Inter_MonitorNoticeGuildInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildInfo::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildInfo::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user || !payload || !user->get_state() ||
        payload_size < static_cast<int>(sizeof(GuildInfoNoticeReplyPrefix) + 1u)) {
        return 0;
    }

    GuildInfoNoticeReplyPrefix* const reply =
        reinterpret_cast<GuildInfoNoticeReplyPrefix*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(reply->target.recipient.charac_no)) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorNoticeGuildInfo::dispatch_sig(CUser*, char*, int)",
            0x2613,
            "Inter_MonitorNoticeGuildInfo::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1),
            reply->target.recipient.charac_no);
        return 0;
    }

    const unsigned int skill_count = static_cast<unsigned char>(reply->guild_info.skill_count);
    if (skill_count > sizeof(reply->guild_info.skills) / sizeof(reply->guild_info.skills[0]) ||
        !std::memchr(reply->guild_info.guild_name, 0, sizeof(reply->guild_info.guild_name)) ||
        !std::memchr(reply->guild_info.raw_a4, 0, sizeof(reply->guild_info.raw_a4)) ||
        !std::memchr(payload + sizeof(GuildInfoNoticeReplyPrefix), 0,
                     payload_size - sizeof(GuildInfoNoticeReplyPrefix))) {
        LogManager::logFormat(1, "InterDispatcher.cpp", __FUNCTION__, __LINE__,
            "[GUILD][SNAPSHOT_REJECT] char=%d skills=%u size=%d",
            user->getCurCharacNo(), skill_count, payload_size);
        return 0;
    }

    const int previous_stat_level = user->IsGuildSkillLearn(200);
    const int previous_exp_level = user->IsGuildSkillLearn(201);
    // 0x84CB7DD copies the DB image before constructing the client packet.
    user->SetGuildDBInfo(reply->guild_info);

    // Restore the display/proxy cache from this complete authoritative
    // snapshot.  A missing startup cache must not make later guild changes
    // fail.  Do not change membership or import another guild's cache here.
    const unsigned int guild_key = static_cast<unsigned int>(user->get_charac_guildkey());
    if (guild_key && guild_key == reply->target.guild_key && GlobalData::s_guild_proxy_mgr) {
        CGuildServerProxy* const proxy = GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        if (proxy) {
            STGuildProxy cached;
            std::memcpy(cached.guild_name, reply->guild_info.guild_name, sizeof(cached.guild_name));
            cached.guild_level = reply->guild_info.guild_level;
            cached.power_side = reply->guild_info.power_side;
            cached.guild_agit_flag = reply->guild_info.guild_agit_flag;
            if (!proxy->InsertGuildInfoProxy(guild_key, cached)) {
                proxy->ModifyGuildInfoProxy(guild_key, cached);
            }
        }
    }
    LogManager::logFormat(1, "InterDispatcher.cpp", __FUNCTION__, __LINE__,
        "[GUILD][SNAPSHOT] char=%d guild=%u level=%u grade=%u skills=%u stat200=%d exp201=%d",
        user->getCurCharacNo(), guild_key, static_cast<unsigned int>(reply->guild_info.guild_level),
        static_cast<unsigned int>(static_cast<unsigned char>(user->getGuildMemberGrade())),
        skill_count, user->IsGuildSkillLearn(200), user->IsGuildSkillLearn(201));

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_GUILD_INFO);
    const int guild_name_length =
        static_cast<int>(std::strlen(reply->guild_info.guild_name));
    packet.put_int(guild_name_length);
    packet.put_str(reply->guild_info.guild_name, guild_name_length);
    packet.put_byte(user->get_charac_no(-1) ==
                            static_cast<int>(
                                reply->guild_info.guild_master_charac_no)
                        ? 1
                        : 0);
    packet.put_byte(static_cast<unsigned char>(reply->guild_info.guild_level));
    packet.put_byte((reply->guild_info.guild_flag & 2u) != 0u);
    packet.put_short(reply->guild_info.guild_member_count);
    packet.put_byte(user->get_guild_create_right());
    packet.put_int(reply->guild_info.raw_24);
    packet.put_byte(static_cast<unsigned char>(reply->guild_info.raw_28));
    packet.put_int(reply->guild_info.guild_experience);
    packet.put_short(reply->guild_info.raw_42);

    int skill_count_index = packet.get_index();
    int emitted_skill_count = 0;
    packet.put_byte(0);
    for (int index = 0;
         index < static_cast<unsigned char>(reply->guild_info.skill_count);
         ++index) {
        const STGuildSkill& skill = reply->guild_info.skills[index];
        // The old body invokes getGuildMemberGrade twice in the condition.
        if (static_cast<unsigned char>(user->getGuildMemberGrade()) &&
            static_cast<unsigned char>(user->getGuildMemberGrade()) &&
            skill.skill_id > 199) {
            packet.put_int(skill.skill_id);
            packet.put_byte(skill.level);
            ++emitted_skill_count;
        }
    }
    packet.put_byte(skill_count_index, emitted_skill_count);

    const char* const guild_extra_text =
        payload + sizeof(GuildInfoNoticeReplyPrefix);
    const int guild_extra_text_length =
        static_cast<int>(std::strlen(guild_extra_text));
    packet.put_int(guild_extra_text_length);
    packet.put_str(guild_extra_text, guild_extra_text_length);
    packet.put_byte(static_cast<unsigned char>(reply->guild_info.power_side));
    packet.put_byte(static_cast<unsigned char>(reply->guild_info.raw_9f));
    packet.put_int(reply->guild_info.guild_fund);
    const int guild_tail_text_length =
        static_cast<int>(std::strlen(reinterpret_cast<const char*>(reply->guild_info.raw_a4)));
    packet.put_int(guild_tail_text_length);
    packet.put_str(reinterpret_cast<const char*>(reply->guild_info.raw_a4),
                   guild_tail_text_length);
    packet.finalize(true);
    user->Send(packet);
    if (previous_stat_level != user->IsGuildSkillLearn(200) ||
        previous_exp_level != user->IsGuildSkillLearn(201)) {
        FunctionGuildMember::RefreshGuildBenefits(user);
    }
    return 0;
}

// IDA_EA: 0x84EAA60 / 0x84EAA7C / 0x84EAAAC
// IDA_NAME: _ZN28Inter_MonitorNoticeGuildInfoC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeGuildInfo::Inter_MonitorNoticeGuildInfo()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildInfo::~Inter_MonitorNoticeGuildInfo() {}
