#include "old_game/social/guild/guild_monitor_state_dispatchers_a.h"

#include <cstddef>
#include <cstdint>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

// Every Inter payload starts with the ten-byte server-packet header.  These
// projections name only the fields consumed by the original handlers.
#pragma pack(push, 1)
struct MonitorSetGuildKeyPayload {
    uint8_t server_packet_header[10];
    uint32_t reserved_0a;       // +0x0a
    uint32_t charac_no;         // +0x0e, recv->m_uCharID
    uint32_t guild_key;         // +0x12, recv->m_uGuildKey
};

struct MonitorNoticeGuildCreatePayload {
    uint8_t server_packet_header[10];
    uint32_t guild_key;         // +0x0a
    uint32_t charac_no;         // +0x0e, recv->m_uCharID
    char guild_name[1];         // +0x12, NUL-terminated in the old packet
};

struct MonitorGuildMemberNamingReplyPayload {
    uint8_t server_packet_header[10];
    uint32_t result;            // +0x0a
    uint32_t reserved_0e;       // +0x0e
    uint32_t charac_no;         // +0x12, recv->m_uCharID
};
#pragma pack(pop)

static_assert(offsetof(MonitorSetGuildKeyPayload, charac_no) == 0x0eu &&
                  offsetof(MonitorSetGuildKeyPayload, guild_key) == 0x12u,
              "Inter_MonitorSetGuildKey payload offsets must retain old ABI");
static_assert(offsetof(MonitorNoticeGuildCreatePayload, guild_key) == 0x0au &&
                  offsetof(MonitorNoticeGuildCreatePayload, charac_no) ==
                      0x0eu &&
                  offsetof(MonitorNoticeGuildCreatePayload, guild_name) ==
                      0x12u,
              "Inter_MonitorNoticeGuildCreate payload offsets must retain old ABI");
static_assert(offsetof(MonitorGuildMemberNamingReplyPayload, result) ==
                  0x0au &&
                  offsetof(MonitorGuildMemberNamingReplyPayload, charac_no) ==
                      0x12u,
              "Inter_MonitorGuildMemberNamingReply payload offsets must retain old ABI");

void SendNativeGuildKeyProjection(CUser* user, uint32_t guild_key) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_GUILD_CREATE);
    // DNF.exe 0828 sub_5AA8C0 consumes exactly one uint32 guild key.  The
    // legacy guild-name tail belongs only to the server-to-server packet.
    packet.put_int(guild_key);
    packet.finalize(true);
    user->Send(packet);
}

void SendGuildIdentityRefresh(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(2);
    user->make_basic_info(packet, 2);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

// IDA_EA: 0x866C04E
// IDA_NAME: _ZN5CUser28set_charac_guildkey_in_loginEjj
// DEMANGLED: CUser::set_charac_guildkey_in_login(unsigned int,unsigned int)
char CUser::set_charac_guildkey_in_login(uint32_t guild_key,
                                         uint32_t charac_no) {
    // The original explicitly gates this persisted-character update on state
    // 2 and walks CUser::characters_, not the selected-character pointer.
    char result = static_cast<char>(get_state() == 2);
    if (!result) {
        return result;
    }

    for (uint32_t index = 0;
         characters_.size() > static_cast<std::size_t>(index);
         ++index) {
        _Charac_info& character = characters_[index];
        if (static_cast<uint32_t>(character.fields_.character_no_) ==
            charac_no) {
            character.fields_.guild_id_ = guild_key;
            // 0x866C0B7 returns EAX directly after vector::operator[].  The
            // legacy declaration is `char`, therefore retain the observed
            // low-byte return value even though this caller ignores it.
            return static_cast<char>(
                reinterpret_cast<uintptr_t>(&character));
        }
    }
    return 0;
}

GuildKeySync::Result GuildKeySync::SynchronizeAuthoritativeNonZero(
    CUser* user,
    uint32_t charac_no,
    uint32_t guild_key,
    GuildKeySync::Policy policy) {
    if (!user || guild_key == 0 || user->get_state() <= 0) {
        return kRejected;
    }

    // During character selection there is no selected-character object to
    // notify.  Project the authoritative key into the matching character-list
    // row; SELECT_CHARACTER will subsequently serialize that row to 0828.
    if (user->get_state() == 2) {
        user->set_charac_guildkey_in_login(guild_key, charac_no);
        return kSynchronized;
    }

    if (user->get_state() <= 2 ||
        static_cast<uint32_t>(user->get_charac_no(-1)) != charac_no) {
        return kRejected;
    }

    const uint32_t cached_key =
        static_cast<uint32_t>(user->get_charac_guildkey());
    if (policy == kSnapshotRepair && cached_key != 0 &&
        cached_key != guild_key) {
        // Preserve the official stale-page guard: a delayed member-list page
        // may repair key 0, but it may not roll back a newer non-zero key.
        return kConflict;
    }

    const bool changed = cached_key != guild_key;
    if (changed) {
        user->set_charac_guildkey(guild_key);
    }

    // A create confirmation must be replayable even if signal 97 won the
    // race and populated the cache first.  Transition/snapshot paths emit the
    // native assignment only when they actually repair or change state.
    if (changed || policy == kConfirmation) {
        SendNativeGuildKeyProjection(user, guild_key);
    }
    if (changed) {
        SendGuildIdentityRefresh(user);
    }
    return kSynchronized;
}

// IDA_EA: 0x84CBD26
// IDA_NAME: _ZN24Inter_MonitorSetGuildKey12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorSetGuildKey::dispatch_sig(CUser *,char *,int)
int Inter_MonitorSetGuildKey::dispatch_sig(CUser* user, char* payload, int) {
    const MonitorSetGuildKeyPayload& reply =
        *reinterpret_cast<const MonitorSetGuildKeyPayload*>(payload);

    if (user->get_state() == 2) {
        if (reply.guild_key != 0) {
            GuildKeySync::SynchronizeAuthoritativeNonZero(
                user,
                reply.charac_no,
                reply.guild_key,
                GuildKeySync::kTransition);
        } else {
            // Zero retains the official login-list removal semantics.
            user->set_charac_guildkey_in_login(0, reply.charac_no);
        }
        return 0;
    }

    if (user->get_state() <= 2) {
        return 9940;
    }

    if (static_cast<uint32_t>(user->get_charac_no(-1)) != reply.charac_no) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorSetGuildKey::dispatch_sig(CUser*, char*, int)",
            0x26a7,
            "Inter_MonitorSetGuildKey::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1),
            reply.charac_no);
        return 0;
    }

    if (reply.guild_key != 0) {
        GuildKeySync::SynchronizeAuthoritativeNonZero(
            user,
            reply.charac_no,
            reply.guild_key,
            GuildKeySync::kTransition);
        return 0;
    } else {
        user->resetGuildDBInfo();

        PacketGuard packet;
        packet.clear();
        packet.put_header(0, ENUM_NOTIPACKET_GUILD_SECEDE_TO_USER);
        packet.finalize(true);
        user->Send(packet);
        user->send_skill_info();

        packet.clear();
        packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
        packet.put_byte(1);
        packet.put_short(1);
        user->make_basic_info(packet, 1);
        packet.finalize(true);
        user->Send(packet);
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(2);
    user->make_basic_info(packet, 2);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EAACA / 0x84EAAE6 / 0x84EAB16
Inter_MonitorSetGuildKey::Inter_MonitorSetGuildKey() : ISigDispatcher() {}
Inter_MonitorSetGuildKey::~Inter_MonitorSetGuildKey() {}

// IDA_EA: 0x84CBB94
// IDA_NAME: _ZN30Inter_MonitorNoticeGuildCreate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildCreate::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildCreate::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int) {
    if (!user->get_state()) {
        return 9852;
    }

    const MonitorNoticeGuildCreatePayload& reply =
        *reinterpret_cast<const MonitorNoticeGuildCreatePayload*>(payload);
    if (static_cast<uint32_t>(user->get_charac_no(-1)) != reply.charac_no) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorNoticeGuildCreate::dispatch_sig(CUser*, char*, int)",
            0x2682,
            "Packet_Monitor_Notice_Guild_Create_ToUser::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1),
            reply.charac_no);
        return 0;
    }

    GuildKeySync::SynchronizeAuthoritativeNonZero(
        user,
        reply.charac_no,
        reply.guild_key,
        GuildKeySync::kConfirmation);
    return 0;
}

// IDA_EA: 0x84EAB34 / 0x84EAB50 / 0x84EAB80
Inter_MonitorNoticeGuildCreate::Inter_MonitorNoticeGuildCreate()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildCreate::~Inter_MonitorNoticeGuildCreate() {}

// IDA_EA: 0x84CC23C
// IDA_NAME: _ZN35Inter_MonitorGuildMemberNamingReply12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorGuildMemberNamingReply::dispatch_sig(CUser *,char *,int)
int Inter_MonitorGuildMemberNamingReply::dispatch_sig(CUser* user,
                                                      char* payload,
                                                      int) {
    if (user->get_state()) {
        const MonitorGuildMemberNamingReplyPayload& reply =
            *reinterpret_cast<const MonitorGuildMemberNamingReplyPayload*>(
                payload);
        if (static_cast<uint32_t>(user->get_charac_no(-1)) ==
            reply.charac_no) {
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_GUILDMEMBER_NAMING);
            packet.put_byte(reply.result != 0 ? 0 : 1);
            packet.put_byte(static_cast<unsigned char>(reply.result));
            packet.finalize(true);
            user->Send(packet);
        }
    }
    return 0;
}

// IDA_EA: 0x84ED524 / 0x84FA062 / 0x84FA092
Inter_MonitorGuildMemberNamingReply::Inter_MonitorGuildMemberNamingReply()
    : ISigDispatcher() {}
Inter_MonitorGuildMemberNamingReply::~Inter_MonitorGuildMemberNamingReply() {}
