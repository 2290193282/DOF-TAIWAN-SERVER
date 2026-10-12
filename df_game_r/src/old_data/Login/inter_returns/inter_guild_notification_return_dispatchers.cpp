#include "old_data/Login/inter_returns/inter_guild_notification_return_dispatchers.h"

#include <cstring>
#include <stdint.h>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

bool IsTargetUser(CUser* user, const char* payload, int source_line,
                  const char* function_name) {
    const int actual = user->get_charac_no(-1);
    const int expected = static_cast<int>(ReadU32(payload, 10));
    if (actual == expected) return true;
    LogManager::logFormat(1, "InterDispatcher.cpp", function_name, source_line,
                          "GUILD/MEMBER Inter return: pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                          actual, expected);
    return false;
}

}  // namespace

// IDA_EA: 0x84D28CA
// IDA_NAME: _ZN22Inter_ReplyGuildLetter12dispatch_sigEP5CUserPci
int Inter_ReplyGuildLetter::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() || !IsTargetUser(user, payload, 0x3340,
            "virtual int Inter_ReplyGuildLetter::dispatch_sig(CUser*, char*, int)")) return 0;
    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_MAILBOX_SEND);
    reply.put_byte(payload[18] ? 0 : 1);
    reply.finalize(true);
    user->Send(reply);
    return 0;
}
// IDA_EA: 0x84ED94C / 0x84F9432 / 0x84F9462
Inter_ReplyGuildLetter::Inter_ReplyGuildLetter() : ISigDispatcher() {}
Inter_ReplyGuildLetter::~Inter_ReplyGuildLetter() {}

// IDA_EA: 0x84D2A26
// IDA_NAME: _ZN29Inter_CallGuildInviteToCaller12dispatch_sigEP5CUserPci
int Inter_CallGuildInviteToCaller::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() || !IsTargetUser(user, payload, 0x335C,
            "virtual int Inter_CallGuildInviteToCaller::dispatch_sig(CUser*, char*, int)")) return 0;
    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_CALL_GUILD_INVITE);
    const uint32_t error = ReadU32(payload, 18);
    if (error) {
        reply.put_byte(0);
        reply.put_byte(static_cast<unsigned char>(error));
    } else {
        reply.put_byte(1);
    }
    reply.finalize(true);
    user->Send(reply);
    return 0;
}
// IDA_EA: 0x84ED968 / 0x84F93E4 / 0x84F9414
Inter_CallGuildInviteToCaller::Inter_CallGuildInviteToCaller() : ISigDispatcher() {}
Inter_CallGuildInviteToCaller::~Inter_CallGuildInviteToCaller() {}

// IDA_EA: 0x84D2DFE
// IDA_NAME: _ZN31Inter_ReplyGuildInviteToInvited12dispatch_sigEP5CUserPci
int Inter_ReplyGuildInviteToInvited::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() || !IsTargetUser(user, payload, 0x33AE,
            "virtual int Inter_ReplyGuildInviteToInvited::dispatch_sig(CUser*, char*, int)")) return 0;
    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_REPLY_GUILD_INVITE);
    const uint32_t error = ReadU32(payload, 18);
    if (error) {
        reply.put_byte(0);
        reply.put_int(static_cast<int>(error));
    } else {
        reply.put_byte(1);
    }
    reply.finalize(true);
    user->Send(reply);
    return 0;
}
// IDA_EA: 0x84ED9A0 / 0x84F9348 / 0x84F9378
Inter_ReplyGuildInviteToInvited::Inter_ReplyGuildInviteToInvited() : ISigDispatcher() {}
Inter_ReplyGuildInviteToInvited::~Inter_ReplyGuildInviteToInvited() {}

// IDA_EA: 0x84D2F6E
// IDA_NAME: _ZN30Inter_ReplyGuildInviteToCaller12dispatch_sigEP5CUserPci
int Inter_ReplyGuildInviteToCaller::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() || !IsTargetUser(user, payload, 0x33CF,
            "virtual int Inter_ReplyGuildInviteToCaller::dispatch_sig(CUser*, char*, int)")) return 0;
    const uint32_t result = ReadU32(payload, 18);
    if (result == 2) {
        user->Send_SoloPlay_Reject_NotiPack(payload + 22, 1, 4);
    } else if (result == 3) {
        user->Send_SoloPlay_Reject_NotiPack(payload + 22, 0, 4);
    } else {
        PacketGuard reply;
        reply.put_header(0, 148);
        reply.put_byte(static_cast<unsigned char>(result));
        if (result <= 1) {
            const int name_length = static_cast<int>(std::strlen(payload + 22));
            reply.put_int(name_length);
            reply.put_str(payload + 22, name_length);
        }
        reply.finalize(true);
        user->Send(reply);
    }
    return 0;
}
// IDA_EA: 0x84ED9BC / 0x84F92FA / 0x84F932A
Inter_ReplyGuildInviteToCaller::Inter_ReplyGuildInviteToCaller() : ISigDispatcher() {}
Inter_ReplyGuildInviteToCaller::~Inter_ReplyGuildInviteToCaller() {}

// IDA_EA: 0x84C9A32
// IDA_NAME: _ZN31Inter_NoticeGuildMasterDelegate12dispatch_sigEP5CUserPci
int Inter_NoticeGuildMasterDelegate::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() || !IsTargetUser(user, payload, 0x23A0,
            "virtual int Inter_NoticeGuildMasterDelegate::dispatch_sig(CUser*, char*, int)")) return 0;
    PacketGuard reply;
    reply.put_header(0, 149);
    const int length = static_cast<int>(std::strlen(payload + 18));
    reply.put_int(length);
    reply.put_str(payload + 18, length);
    reply.finalize(true);
    user->Send(reply);
    return 0;
}
// IDA_EA: 0x84EDA64 / 0x84FA14C / 0x84FA17C
Inter_NoticeGuildMasterDelegate::Inter_NoticeGuildMasterDelegate() : ISigDispatcher() {}
Inter_NoticeGuildMasterDelegate::~Inter_NoticeGuildMasterDelegate() {}

// IDA_EA: 0x84C9B9A
// IDA_NAME: _ZN35Inter_GuildReplyGuildMasterDelegate12dispatch_sigEP5CUserPci
int Inter_GuildReplyGuildMasterDelegate::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() || !IsTargetUser(user, payload, 0x23BA,
            "virtual int Inter_GuildReplyGuildMasterDelegate::dispatch_sig(CUser*, char*, int)")) return 0;
    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_GUILD_MASTER_DELEGATE);
    const uint32_t error = ReadU32(payload, 18);
    if (error) {
        reply.put_byte(0);
        reply.put_byte(static_cast<unsigned char>(error));
    } else {
        reply.put_byte(1);
    }
    reply.finalize(true);
    user->Send(reply);
    return 0;
}
// IDA_EA: 0x84EDA80 / 0x84FA0FE / 0x84FA12E
Inter_GuildReplyGuildMasterDelegate::Inter_GuildReplyGuildMasterDelegate() : ISigDispatcher() {}
Inter_GuildReplyGuildMasterDelegate::~Inter_GuildReplyGuildMasterDelegate() {}

// IDA_EA: 0x84C9D0A
// IDA_NAME: _ZN30Inter_NoticeHasBeenGuildMaster12dispatch_sigEP5CUserPci
int Inter_NoticeHasBeenGuildMaster::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() || !IsTargetUser(user, payload, 0x23DA,
            "virtual int Inter_NoticeHasBeenGuildMaster::dispatch_sig(CUser*, char*, int)")) return 0;
    PacketGuard reply;
    reply.put_header(0, 150);
    reply.finalize(true);
    user->Send(reply);
    return 0;
}
// IDA_EA: 0x84EDA9C / 0x84FA0B0 / 0x84FA0E0
Inter_NoticeHasBeenGuildMaster::Inter_NoticeHasBeenGuildMaster() : ISigDispatcher() {}
Inter_NoticeHasBeenGuildMaster::~Inter_NoticeHasBeenGuildMaster() {}
