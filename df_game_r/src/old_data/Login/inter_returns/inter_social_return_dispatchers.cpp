#include "old_data/Login/inter_returns/inter_social_return_dispatchers.h"

#include <cstring>
#include <stdint.h>

#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint16_t ReadU16(const char* payload, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84D14B0
// IDA_NAME: _ZN21Inter_MonitorAddBuddy12dispatch_sigEP5CUserPci
int Inter_MonitorAddBuddy::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() ||
        user->getCurCharacNo() != static_cast<int>(ReadU32(payload, 10))) {
        return 0;
    }

    PacketGuard reply;
    reply.clear();
    reply.put_header(1, ENUM_CMDPACKET_ADD_FRIEND);
    if (payload[58]) {
        reply.put_byte(0);
        reply.put_byte(payload[58]);
    } else {
        reply.put_byte(1);
        const int name_length = static_cast<int>(std::strlen(payload + 18));
        reply.put_int(name_length);
        reply.put_str(payload + 18, name_length);
        reply.put_byte(ReadU16(payload, 48));
        reply.put_byte(payload[50]);
        reply.put_byte(payload[51]);
        reply.put_int(static_cast<int>(ReadU32(payload, 52)));
        reply.put_byte(payload[56]);
        reply.put_byte(static_cast<unsigned char>(payload[57]));
    }
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x84ED818 / 0x84F973E / 0x84F976E
Inter_MonitorAddBuddy::Inter_MonitorAddBuddy() : ISigDispatcher() {}
Inter_MonitorAddBuddy::~Inter_MonitorAddBuddy() {}

// IDA_EA: 0x84D16F6
// IDA_NAME: _ZN21Inter_MonitorDelBuddy12dispatch_sigEP5CUserPci
int Inter_MonitorDelBuddy::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state() ||
        user->getCurCharacNo() != static_cast<int>(ReadU32(payload, 10))) {
        return 0;
    }

    PacketGuard reply;
    reply.clear();
    reply.put_header(1, ENUM_CMDPACKET_REMOVE_FRIEND);
    if (payload[48]) {
        reply.put_byte(0);
        reply.put_byte(payload[48]);
    } else {
        reply.put_byte(1);
        const int name_length = static_cast<int>(std::strlen(payload + 18));
        reply.put_int(name_length);
        reply.put_str(payload + 18, name_length);
    }
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x84ED834 / 0x84F96F0 / 0x84F9720
Inter_MonitorDelBuddy::Inter_MonitorDelBuddy() : ISigDispatcher() {}
Inter_MonitorDelBuddy::~Inter_MonitorDelBuddy() {}

// IDA_EA: 0x84D18AC
// IDA_NAME: _ZN22Inter_ChangeLetterStat12dispatch_sigEP5CUserPci
int Inter_ChangeLetterStat::dispatch_sig(CUser* user, char* payload, int) {
    if (user->get_state() <= 2 || !user->getCurCharacR() ||
        user->getCurCharacNo() != static_cast<int>(ReadU32(payload, 0))) {
        return 12450;
    }

    const uint32_t letter_id = ReadU32(payload, 4);
    const uint16_t status = ReadU16(payload, 8);
    WongWork::CMailBox* const mailbox = user->GetMailBox();
    if (status == 3) {
        if (mailbox) {
            mailbox->InsertLetterKeepCount(letter_id);
            mailbox->DecLoadedLetterCount();
        }
    } else if (status == 0 && mailbox &&
               mailbox->DeleteLetterKeepCount(letter_id) != 1) {
        mailbox->DecLoadedLetterCount();
    }

    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_CHANGE_LETTER_STAT);
    if (letter_id) {
        reply.put_byte(1);
        reply.put_int(static_cast<int>(letter_id));
        reply.put_short(status);
    } else {
        reply.put_byte(0);
        reply.put_byte(2);
    }
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x84ED850 / 0x84F96A2 / 0x84F96D2
Inter_ChangeLetterStat::Inter_ChangeLetterStat() : ISigDispatcher() {}
Inter_ChangeLetterStat::~Inter_ChangeLetterStat() {}

// IDA_EA: 0x84D22FC
// IDA_NAME: _ZN24Inter_ReqQueryCharacInfo12dispatch_sigEP5CUserPci
int Inter_ReqQueryCharacInfo::dispatch_sig(CUser* user, char* payload, int) {
    if (user->get_state() <= 2) return 0;
    const uint32_t error = ReadU32(payload, 32);
    if (error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_QUERY_CHARAC_INFO,
                                 static_cast<unsigned char>(error));
        return 0;
    }

    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_QUERY_CHARAC_INFO);
    reply.put_byte(1);
    const int name_length = static_cast<int>(std::strlen(payload));
    reply.put_int(name_length);
    reply.put_str(payload, name_length);
    reply.put_short(ReadU16(payload, 42));
    reply.put_byte(payload[40]);
    reply.put_byte(payload[44]);
    reply.put_byte(payload[45]);
    reply.put_byte(payload[46]);
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x84ED8A4 / 0x84F95B8 / 0x84F95E8
Inter_ReqQueryCharacInfo::Inter_ReqQueryCharacInfo() : ISigDispatcher() {}
Inter_ReqQueryCharacInfo::~Inter_ReqQueryCharacInfo() {}

// IDA_EA: 0x84CC55E
// IDA_NAME: _ZN41Inter_MonitorNoticeChangeGuildMemberGrade12dispatch_sigEP5CUserPci
int Inter_MonitorNoticeChangeGuildMemberGrade::dispatch_sig(
    CUser* user, char* payload, int) {
    if (user->get_state() &&
        user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
        PacketGuard reply;
        reply.put_header(0, 140);
        reply.put_byte(static_cast<unsigned char>(payload[18]));
        reply.finalize(true);
        user->Send(reply);
    }
    return 0;
}

// IDA_EA: 0x84ED578 / 0x84F9FC6 / 0x84F9FF6
Inter_MonitorNoticeChangeGuildMemberGrade::Inter_MonitorNoticeChangeGuildMemberGrade()
    : ISigDispatcher() {}
Inter_MonitorNoticeChangeGuildMemberGrade::~Inter_MonitorNoticeChangeGuildMemberGrade() {}

// IDA_EA: 0x84CC650
// IDA_NAME: _ZN29Inter_GuildNoticeGuildMessage12dispatch_sigEP5CUserPci
int Inter_GuildNoticeGuildMessage::dispatch_sig(CUser* user,
                                                char* payload,
                                                int) {
    if (user->get_state() &&
        user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
        PacketGuard reply;
        reply.put_header(0, 141);
        const int length = static_cast<int>(std::strlen(payload + 18));
        reply.put_int(length);
        reply.put_str(payload + 18, length);
        reply.finalize(true);
        user->Send(reply);
    }
    return 0;
}

// IDA_EA: 0x84ED594 / 0x84F9F78 / 0x84F9FA8
Inter_GuildNoticeGuildMessage::Inter_GuildNoticeGuildMessage()
    : ISigDispatcher() {}
Inter_GuildNoticeGuildMessage::~Inter_GuildNoticeGuildMessage() {}
