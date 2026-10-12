#include "old_game/social/guild/guild_monitor_notice_dispatchers_a.h"

#include <cstring>
#include <ctime>
#include <stddef.h>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/log_manager.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

#pragma pack(push, 1)

// Shared transport prefix: the monitor signal header occupies bytes +0x00 to
// +0x0d.  All five original bodies read the target character no at +0x0e.
struct GuildEnterNoticePayload {
    uint8_t transport_prefix[0x0e];
    uint32_t target_charac_no;       // IDA +0x0e
    uint16_t guild_member_index;     // IDA +0x12
    uint32_t guild_key;              // IDA +0x14, client packet field 1
    uint8_t reserved_18_1b[4];
    uint32_t joined_charac_no;       // IDA +0x1c, client packet field 2
    char guild_name[0x17];           // IDA +0x20
    char character_name[0x1e];       // IDA +0x37
    uint32_t current_charac_no;      // IDA +0x55
    uint8_t guild_member_grade;      // IDA +0x59
};

struct GuildSecedeNoticePayload {
    uint8_t transport_prefix[0x0e];
    uint32_t target_charac_no;       // IDA +0x0e
    uint32_t guild_key;              // IDA +0x12, client packet field 1
    uint8_t reserved_16_19[4];
    uint32_t seceding_charac_no;     // IDA +0x1a, client packet field 2
    uint16_t guild_member_grade;     // IDA +0x1e, truncated by put_byte
    char guild_name[0x17];           // IDA +0x20
    char character_name[0x1e];       // IDA +0x37
};

struct GuildMarkChangeNoticePayload {
    uint8_t transport_prefix[0x0a];
    uint32_t channel_no;             // IDA +0x0a
    uint32_t target_charac_no;       // IDA +0x0e
    uint32_t guild_key;              // IDA +0x12
};

struct GuildDismissNoticePayload {
    uint8_t transport_prefix[0x0e];
    uint32_t target_charac_no;       // IDA +0x0e
};

struct GuildChatNoticePayload {
    uint8_t transport_prefix[0x0e];
    uint32_t target_charac_no;       // IDA +0x0e
    char speaker_name[0x1e];         // IDA +0x12
    uint8_t message_length;          // IDA +0x30
    char message[1];                 // IDA +0x31, length-prefixed bytes
};

#pragma pack(pop)

static_assert(offsetof(GuildEnterNoticePayload, target_charac_no) == 0x0eu &&
                  offsetof(GuildEnterNoticePayload, guild_member_index) ==
                      0x12u &&
                  offsetof(GuildEnterNoticePayload, guild_key) == 0x14u &&
                  offsetof(GuildEnterNoticePayload, joined_charac_no) ==
                      0x1cu &&
                  offsetof(GuildEnterNoticePayload, guild_name) == 0x20u &&
                  offsetof(GuildEnterNoticePayload, character_name) ==
                      0x37u &&
                  offsetof(GuildEnterNoticePayload, current_charac_no) ==
                      0x55u &&
                  offsetof(GuildEnterNoticePayload, guild_member_grade) ==
                      0x59u,
              "Guild-enter monitor payload must preserve the IDA layout");
static_assert(offsetof(GuildSecedeNoticePayload, target_charac_no) == 0x0eu &&
                  offsetof(GuildSecedeNoticePayload, guild_key) == 0x12u &&
                  offsetof(GuildSecedeNoticePayload, seceding_charac_no) ==
                      0x1au &&
                  offsetof(GuildSecedeNoticePayload, guild_member_grade) ==
                      0x1eu &&
                  offsetof(GuildSecedeNoticePayload, guild_name) == 0x20u &&
                  offsetof(GuildSecedeNoticePayload, character_name) ==
                      0x37u,
              "Guild-secede monitor payload must preserve the IDA layout");
static_assert(offsetof(GuildMarkChangeNoticePayload, channel_no) == 0x0au &&
                  offsetof(GuildMarkChangeNoticePayload, target_charac_no) ==
                      0x0eu &&
                  offsetof(GuildMarkChangeNoticePayload, guild_key) == 0x12u,
              "Guild-mark monitor payload must preserve the IDA layout");
static_assert(offsetof(GuildDismissNoticePayload, target_charac_no) == 0x0eu,
              "Guild-dismiss monitor payload must preserve the IDA layout");
static_assert(offsetof(GuildChatNoticePayload, target_charac_no) == 0x0eu &&
                  offsetof(GuildChatNoticePayload, speaker_name) == 0x12u &&
                  offsetof(GuildChatNoticePayload, message_length) == 0x30u &&
                  offsetof(GuildChatNoticePayload, message) == 0x31u,
              "Guild-chat monitor payload must preserve the IDA layout");

void LogWrongTarget(const char* pretty_function,
                    int source_line,
                    const char* message,
                    CUser* user,
                    uint32_t target_charac_no) {
    LogManager::logFormat(1, "InterDispatcher.cpp", pretty_function,
                          source_line, message, user->get_charac_no(-1),
                          target_charac_no);
}

}  // namespace

// IDA_EA: 0x8685A48
// IDA_NAME: _ZN15cUserHistoryLog9GuildJoinEjPKcjtc
// DEMANGLED: cUserHistoryLog::GuildJoin(unsigned int,char const*,unsigned int,
//                                       unsigned short,char)
int cUserHistoryLog::GuildJoin(unsigned int guild_key,
                               const char* guild_name,
                               unsigned int charac_no,
                               unsigned short guild_member_index,
                               char guild_member_grade) {
    std::tm local_time;
    const std::time_t current_time =
        static_cast<std::time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm* const time_info = localtime_r(&current_time, &local_time);
    return owner_->LogHistory("GuildJoin,\"%d/%d/%d\",%d,\"%s\",%d,%d,%d",
                              time_info->tm_year + 1900,
                              time_info->tm_mon + 1,
                              time_info->tm_mday,
                              guild_key,
                              guild_name,
                              charac_no,
                              guild_member_index,
                              guild_member_grade);
}

// IDA_EA: 0x84C9194
// IDA_NAME: _ZN29Inter_MonitorNoticeGuildEnter12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildEnter::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildEnter::dispatch_sig(CUser* user,
                                                char* payload,
                                                int) {
    if (!user->get_state()) {
        return 0;
    }

    const GuildEnterNoticePayload& notice =
        *reinterpret_cast<const GuildEnterNoticePayload*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(notice.target_charac_no)) {
        LogWrongTarget(
            "virtual int Inter_MonitorNoticeGuildEnter::dispatch_sig(CUser*, char*, int)",
            0x2305,
            "Inter_MonitorNoticeGuildEnter::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user, notice.target_charac_no);
        return 0;
    }

    // The packet is always forwarded after the target check.  The history row
    // is deliberately narrower: the old body writes it only for the current
    // selected character carried at payload +0x55.
    if (notice.current_charac_no ==
        static_cast<uint32_t>(user->getCurCharacNo())) {
        CUser_history_log(user)->GuildJoin(
            static_cast<unsigned int>(user->get_charac_guildkey()),
            notice.guild_name, notice.joined_charac_no,
            notice.guild_member_index,
            static_cast<char>(notice.guild_member_grade != 0));
    }

    PacketGuard packet;
    packet.put_header(0, 58);
    packet.put_int(static_cast<int>(notice.guild_key));
    packet.put_int(static_cast<int>(notice.joined_charac_no));

    const int guild_name_length = static_cast<int>(std::strlen(notice.guild_name));
    packet.put_int(guild_name_length);
    packet.put_str(notice.guild_name, guild_name_length);

    const int character_name_length =
        static_cast<int>(std::strlen(notice.character_name));
    packet.put_int(character_name_length);
    packet.put_str(notice.character_name, character_name_length);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EA4FE / 0x84EA51A / 0x84EA54A
// IDA_NAME: _ZN29Inter_MonitorNoticeGuildEnterC2Ev /
//           _ZN29Inter_MonitorNoticeGuildEnterD2Ev /
//           _ZN29Inter_MonitorNoticeGuildEnterD0Ev
Inter_MonitorNoticeGuildEnter::Inter_MonitorNoticeGuildEnter()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildEnter::~Inter_MonitorNoticeGuildEnter() {}

// IDA_EA: 0x84C957E
// IDA_NAME: _ZN30Inter_MonitorNoticeGuildSecede12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildSecede::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildSecede::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int) {
    if (!user->get_state()) {
        return 0;
    }

    const GuildSecedeNoticePayload& notice =
        *reinterpret_cast<const GuildSecedeNoticePayload*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(notice.target_charac_no)) {
        LogWrongTarget(
            "virtual int Inter_MonitorNoticeGuildSecede::dispatch_sig(CUser*, char*, int)",
            0x2348,
            "Inter_MonitorNoticeGuildSecede::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user, notice.target_charac_no);
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 59);
    packet.put_int(static_cast<int>(notice.guild_key));
    packet.put_int(static_cast<int>(notice.seceding_charac_no));
    // The old code loads the whole unsigned short at +0x1e and delegates the
    // low-byte conversion to InterfacePacketBuf::put_byte.
    packet.put_byte(static_cast<int>(notice.guild_member_grade));

    const int guild_name_length = static_cast<int>(std::strlen(notice.guild_name));
    packet.put_int(guild_name_length);
    packet.put_str(notice.guild_name, guild_name_length);

    const int character_name_length =
        static_cast<int>(std::strlen(notice.character_name));
    packet.put_int(character_name_length);
    packet.put_str(notice.character_name, character_name_length);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EA5D2 / 0x84EA5EE / 0x84EA61E
// IDA_NAME: _ZN30Inter_MonitorNoticeGuildSecedeC2Ev /
//           _ZN30Inter_MonitorNoticeGuildSecedeD2Ev /
//           _ZN30Inter_MonitorNoticeGuildSecedeD0Ev
Inter_MonitorNoticeGuildSecede::Inter_MonitorNoticeGuildSecede()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildSecede::~Inter_MonitorNoticeGuildSecede() {}

// IDA_EA: 0x84CA1E6
// IDA_NAME: _ZN34Inter_MonitorNoticeGuildMarkChange12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildMarkChange::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildMarkChange::dispatch_sig(CUser* user,
                                                     char* payload,
                                                     int) {
    if (!user->get_state()) {
        return 0;
    }

    const GuildMarkChangeNoticePayload& notice =
        *reinterpret_cast<const GuildMarkChangeNoticePayload*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(notice.target_charac_no)) {
        LogWrongTarget(
            "virtual int Inter_MonitorNoticeGuildMarkChange::dispatch_sig(CUser*, char*, int)",
            0x244A,
            "Inter_MonitorNoticeGuildMarkChange::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user, notice.target_charac_no);
        return 0;
    }

    // IDA 0x84CA291..0x84CA2DD passes all three fields, despite the older
    // decompiler display eliding two variadic arguments.
    cMyTrace trace(
        "virtual int Inter_MonitorNoticeGuildMarkChange::dispatch_sig(CUser*, char*, int)",
        0x244F, 0);
    trace("[GUILD MARK CHANGE] Send to client. (channel:%d, character:%u, guildkey:%u)",
          notice.channel_no, notice.target_charac_no, notice.guild_key);

    PacketGuard packet;
    packet.put_header(0, 60);
    packet.put_int(static_cast<int>(notice.guild_key));
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EA63C / 0x84EA658 / 0x84EA688
// IDA_NAME: _ZN34Inter_MonitorNoticeGuildMarkChangeC2Ev /
//           _ZN34Inter_MonitorNoticeGuildMarkChangeD2Ev /
//           _ZN34Inter_MonitorNoticeGuildMarkChangeD0Ev
Inter_MonitorNoticeGuildMarkChange::Inter_MonitorNoticeGuildMarkChange()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildMarkChange::~Inter_MonitorNoticeGuildMarkChange() {}

// IDA_EA: 0x84CA00C
// IDA_NAME: _ZN31Inter_MonitorNoticeGuildDismiss12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildDismiss::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildDismiss::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int) {
    if (!user->get_state()) {
        return 0;
    }

    const GuildDismissNoticePayload& notice =
        *reinterpret_cast<const GuildDismissNoticePayload*>(payload);
    if (user->get_charac_no(-1) != static_cast<int>(notice.target_charac_no)) {
        LogWrongTarget(
            "virtual int Inter_MonitorNoticeGuildDismiss::dispatch_sig(CUser*, char*, int)",
            0x2429,
            "Inter_MonitorNoticeGuildDismiss::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user, notice.target_charac_no);
        return 0;
    }

    // Preserve the original pair of GetGuildDBInfo/get_charac_no reads:
    // they bracket the trace and the conditional create-right reset.
    const uint32_t guild_master_charac_no =
        user->GetGuildDBInfo()->guild_master_charac_no;
    const int charac_no = user->get_charac_no(-1);
    cMyTrace trace(
        "virtual int Inter_MonitorNoticeGuildDismiss::dispatch_sig(CUser*, char*, int)",
        0x242D, 0);
    trace("Guild Create Right Init! Char id : %d, Master Id : %d", charac_no,
          guild_master_charac_no);

    if (user->GetGuildDBInfo()->guild_master_charac_no ==
        static_cast<uint32_t>(user->get_charac_no(-1))) {
        user->set_guild_create_right(0);
    }
    user->set_charac_guildkey(0);

    PacketGuard packet;
    packet.put_header(0, 61);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EA6A6 / 0x84EA6C2 / 0x84EA6F2
// IDA_NAME: _ZN31Inter_MonitorNoticeGuildDismissC2Ev /
//           _ZN31Inter_MonitorNoticeGuildDismissD2Ev /
//           _ZN31Inter_MonitorNoticeGuildDismissD0Ev
Inter_MonitorNoticeGuildDismiss::Inter_MonitorNoticeGuildDismiss()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildDismiss::~Inter_MonitorNoticeGuildDismiss() {}

// IDA_EA: 0x84C9E30
// IDA_NAME: _ZN31Inter_MonitorNoticeGuildChatMsg12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildChatMsg::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildChatMsg::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    if (!user || !payload || !user->get_state() ||
        payload_size < static_cast<int>(offsetof(
                           GuildChatNoticePayload, message))) {
        return 0;
    }

    const GuildChatNoticePayload& notice =
        *reinterpret_cast<const GuildChatNoticePayload*>(payload);
    const int expected_size =
        static_cast<int>(offsetof(GuildChatNoticePayload, message)) +
        static_cast<int>(notice.message_length);
    if (payload_size != expected_size) {
        return 0;
    }
    if (user->get_charac_no(-1) != static_cast<int>(notice.target_charac_no)) {
        LogWrongTarget(
            "virtual int Inter_MonitorNoticeGuildChatMsg::dispatch_sig(CUser*, char*, int)",
            0x23F1,
            "Inter_MonitorNoticeGuildChatMsg::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user, notice.target_charac_no);
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 65);
    packet.put_byte(6);
    packet.put_byte(0);

    int speaker_name_length = 0;
    while (speaker_name_length <
               static_cast<int>(sizeof(notice.speaker_name)) &&
           notice.speaker_name[speaker_name_length] != '\0') {
        ++speaker_name_length;
    }
    packet.put_int(speaker_name_length);
    packet.put_str(notice.speaker_name, speaker_name_length);
    packet.put_byte(0);
    packet.put_int(static_cast<int>(notice.message_length));
    packet.put_str(notice.message, static_cast<int>(notice.message_length));
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EA710 / 0x84EA72C / 0x84EA75C
// IDA_NAME: _ZN31Inter_MonitorNoticeGuildChatMsgC2Ev /
//           _ZN31Inter_MonitorNoticeGuildChatMsgD2Ev /
//           _ZN31Inter_MonitorNoticeGuildChatMsgD0Ev
Inter_MonitorNoticeGuildChatMsg::Inter_MonitorNoticeGuildChatMsg()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildChatMsg::~Inter_MonitorNoticeGuildChatMsg() {}
