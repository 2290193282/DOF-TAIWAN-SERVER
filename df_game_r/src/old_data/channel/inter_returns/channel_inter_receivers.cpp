#include "old_data/channel/inter_returns/channel_inter_receivers.h"

#include <cstring>

#include "infra/log/format/trace.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/channel/channel_db_payloads.h"
#include "old_data/channel/packet_dispatch/pvp_channel_info_dispatcher.h"
#include "old_game/pvp/match/league_match_static_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/world_area_allocators.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x84BF7EA
// IDA_NAME: _ZN19Inter_DeleteChannel12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_DeleteChannel::dispatch_sig(CUser *,char *,int)
int Inter_DeleteChannel::dispatch_sig(CUser* user,
                                      char* payload,
                                      int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_DELETE_CHANNEL* const result =
        reinterpret_cast<const SIG_DELETE_CHANNEL*>(payload);
    if (result->reserved_result == 1) {
        return 2054;
    }
    return 0;
}

// IDA_EA: 0x84E9BE2
// IDA_NAME: _ZN19Inter_DeleteChannelC2Ev
// DEMANGLED: Inter_DeleteChannel::Inter_DeleteChannel(void)
Inter_DeleteChannel::Inter_DeleteChannel() {}

// IDA_EA: 0x84E9BFE
// IDA_NAME: _ZN19Inter_DeleteChannelD2Ev
// IDA_EA: 0x84E9C2E
// IDA_NAME: _ZN19Inter_DeleteChannelD0Ev
// DEMANGLED: Inter_DeleteChannel::~Inter_DeleteChannel()
Inter_DeleteChannel::~Inter_DeleteChannel() {}

// IDA_EA: 0x84D8096
// IDA_NAME: _ZN20Inter_PvPChannelInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PvPChannelInfo::dispatch_sig(CUser *,char *,int)
int Inter_PvPChannelInfo::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)payload_size;

    if (user) {
        Packet_PvPChannelInfo* const reply =
            reinterpret_cast<Packet_PvPChannelInfo*>(payload);
        if (user->getCurCharacR() &&
            user->getCurCharacNo() == static_cast<int>(reply->charac_no)) {
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_PVP_CHANNEL_INFO);
            packet.put_byte(1);
            packet.put_int(static_cast<int>(reply->school_no));
            packet.put_byte(reply->channel_count);
            for (uint32_t index = 0; index < reply->channel_count; ++index) {
                PvPChannelInfoEntry& entry = reply->entries[index];
                packet.put_int(static_cast<int>(entry.value_0));
                packet.put_int(static_cast<int>(entry.value_1));
                if (entry.server_state == 2u) {
                    entry.client_state = 4u;
                }
                packet.put_int(static_cast<int>(entry.client_state));
            }
            packet.finalize(true);
            user->Send(packet);
        }
    }
    return 0;
}

// IDA_EA: 0x84EDEC4
// IDA_NAME: _ZN20Inter_PvPChannelInfoC2Ev
// DEMANGLED: Inter_PvPChannelInfo::Inter_PvPChannelInfo(void)
Inter_PvPChannelInfo::Inter_PvPChannelInfo() {}

// IDA_EA: 0x84F8592
// IDA_NAME: _ZN20Inter_PvPChannelInfoD2Ev
// IDA_EA: 0x84F85C2
// IDA_NAME: _ZN20Inter_PvPChannelInfoD0Ev
// DEMANGLED: Inter_PvPChannelInfo::~Inter_PvPChannelInfo()
Inter_PvPChannelInfo::~Inter_PvPChannelInfo() {}


// IDA_EA: 0x84D8262
// IDA_NAME: _ZN25Inter_PvPChannelUserCount12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PvPChannelUserCount::dispatch_sig(CUser *,char *,int)
int Inter_PvPChannelUserCount::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)user;
    (void)payload_size;

    SIG_PVP_CHANNEL_USER_COUNT& reply =
        *reinterpret_cast<SIG_PVP_CHANNEL_USER_COUNT*>(payload);
    reply.school_user_count = static_cast<uint32_t>(
        g_schoolMgr.GetUserCount(reply.school_no));
    reply.channel_no = static_cast<uint32_t>(G_CEnvironment()->get_channel_no());
    reply.players_per_team = static_cast<uint32_t>(
        CLeagueMatch::GetPlayCountPerOneTeam());
    reply.battle_mode = static_cast<uint32_t>(CLeagueMatch::GetBattleMode());

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(reply.reply_server_group));
    proxy->SendTcpPacket(payload, sizeof(SIG_PVP_CHANNEL_USER_COUNT));
    return 0;
}

// IDA_EA: 0x84EDEE0
// IDA_NAME: _ZN25Inter_PvPChannelUserCountC2Ev
// DEMANGLED: Inter_PvPChannelUserCount::Inter_PvPChannelUserCount(void)
Inter_PvPChannelUserCount::Inter_PvPChannelUserCount() {}

// IDA_EA: 0x84F8544
// IDA_NAME: _ZN25Inter_PvPChannelUserCountD2Ev
// IDA_EA: 0x84F8574
// IDA_NAME: _ZN25Inter_PvPChannelUserCountD0Ev
// DEMANGLED: Inter_PvPChannelUserCount::~Inter_PvPChannelUserCount()
Inter_PvPChannelUserCount::~Inter_PvPChannelUserCount() {}

// IDA_EA: 0x84CB1AC
// IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChat12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeOtherChannelChat::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeOtherChannelChat::dispatch_sig(CUser* user,
                                                       char* payload,
                                                       int payload_size) {
    (void)payload_size;

    if (user->get_state()) {
        const SIG_MONITOR_OTHER_CHANNEL_CHAT* const reply =
            reinterpret_cast<const SIG_MONITOR_OTHER_CHANNEL_CHAT*>(payload);
        if (reply->relay_type == 1) {
            PacketGuard packet;
            packet.put_header(0, 65);
            packet.put_byte(reply->chat_type);
            packet.put_byte(21);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(0);
            packet.put_int(0);
            packet.put_str("", 0);
            packet.finalize(true);
            user->Send(packet);
        } else if (reply->relay_type == 2) {
            PacketGuard packet;
            packet.put_header(0, 65);
            packet.put_byte(reply->chat_type);
            packet.put_byte(77);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(0);
            packet.put_int(0);
            packet.put_str("", 0);
            packet.finalize(true);
            user->Send(packet);
        } else if (reply->relay_type == 3) {
            PacketGuard packet;
            packet.put_header(0, 65);
            packet.put_byte(reply->chat_type);
            packet.put_byte(90);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(0);
            packet.put_int(0);
            packet.put_str("", 0);
            packet.finalize(true);
            user->Send(packet);
        } else if (user->get_charac_no(-1) ==
                   static_cast<int>(reply->target_charac_no)) {
            PacketGuard packet;
            packet.put_header(0, 65);
            packet.put_byte(reply->chat_type);
            packet.put_byte(0);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(reply->sender_marker);
            packet.put_int(reply->message_length);
            packet.put_str(reply->message, reply->message_length);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeOtherChannelChat::dispatch_sig(CUser*, char*, int)",
                9710,
                "Inter_MonitorNoticeBuddyChat::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(reply->target_charac_no));
        }
    }
    return 0;
}

// IDA_EA: 0x84EA98C
// IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChatC2Ev
// DEMANGLED: Inter_MonitorNoticeOtherChannelChat::Inter_MonitorNoticeOtherChannelChat(void)
Inter_MonitorNoticeOtherChannelChat::Inter_MonitorNoticeOtherChannelChat() {}

// IDA_EA: 0x84EA9A8
// IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChatD2Ev
// IDA_EA: 0x84EA9D8
// IDA_NAME: _ZN35Inter_MonitorNoticeOtherChannelChatD0Ev
// DEMANGLED: Inter_MonitorNoticeOtherChannelChat::~Inter_MonitorNoticeOtherChannelChat()
Inter_MonitorNoticeOtherChannelChat::~Inter_MonitorNoticeOtherChannelChat() {}


// IDA_EA: 0x84E5528
// IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLink12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeOtherChannelChatHyperLink::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeOtherChannelChatHyperLink::dispatch_sig(
    CUser* user,
    char* payload,
    int payload_size) {
    (void)payload_size;

    if (user->get_state()) {
        const SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK* const reply =
            reinterpret_cast<const SIG_MONITOR_OTHER_CHANNEL_CHAT_HYPERLINK*>(
                payload);
        if (reply->relay_type == 1) {
            PacketGuard packet;
            packet.put_header(0, 371);
            packet.put_byte(reply->chat_type);
            packet.put_byte(21);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(0);
            packet.put_int(0);
            packet.put_str("", 0);
            packet.put_byte(reply->hyperlink_count);
            for (uint32_t index = 0; index < reply->hyperlink_count; ++index) {
                packet.put_binary(
                    reinterpret_cast<const char*>(
                        reply->hyperlinks[index].serialized),
                    static_cast<int>(sizeof(reply->hyperlinks[index].serialized)));
            }
            packet.finalize(true);
            user->Send(packet);
        } else if (reply->relay_type == 2) {
            PacketGuard packet;
            packet.put_header(0, 371);
            packet.put_byte(reply->chat_type);
            packet.put_byte(77);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(0);
            packet.put_int(0);
            packet.put_str("", 0);
            packet.put_byte(reply->hyperlink_count);
            for (uint32_t index = 0; index < reply->hyperlink_count; ++index) {
                packet.put_binary(
                    reinterpret_cast<const char*>(
                        reply->hyperlinks[index].serialized),
                    static_cast<int>(sizeof(reply->hyperlinks[index].serialized)));
            }
            packet.finalize(true);
            user->Send(packet);
        } else if (reply->relay_type == 3) {
            PacketGuard packet;
            packet.put_header(0, 371);
            packet.put_byte(reply->chat_type);
            packet.put_byte(90);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(0);
            packet.put_int(0);
            packet.put_str("", 0);
            packet.put_byte(reply->hyperlink_count);
            for (uint32_t index = 0; index < reply->hyperlink_count; ++index) {
                packet.put_binary(
                    reinterpret_cast<const char*>(
                        reply->hyperlinks[index].serialized),
                    static_cast<int>(sizeof(reply->hyperlinks[index].serialized)));
            }
            packet.finalize(true);
            user->Send(packet);
        } else if (user->get_charac_no(-1) ==
                   static_cast<int>(reply->target_charac_no)) {
            PacketGuard packet;
            packet.put_header(0, 371);
            packet.put_byte(reply->chat_type);
            packet.put_byte(0);
            const int sender_name_length =
                static_cast<int>(std::strlen(reply->sender_name));
            packet.put_int(sender_name_length);
            packet.put_str(reply->sender_name, sender_name_length);
            packet.put_byte(0);
            packet.put_int(reply->message_length);
            packet.put_str(reply->message, reply->message_length);
            packet.put_byte(reply->hyperlink_count);
            for (uint32_t index = 0; index < reply->hyperlink_count; ++index) {
                packet.put_binary(
                    reinterpret_cast<const char*>(
                        reply->hyperlinks[index].serialized),
                    static_cast<int>(sizeof(reply->hyperlinks[index].serialized)));
            }
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeOtherChannelChatHyperLink::dispatch_sig(CUser*, char*, int)",
                25751,
                "Inter_MonitorNoticeOtherChannelChatHyperLink::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(reply->target_charac_no));
        }
    }
    return 0;
}

// IDA_EA: 0x84EEC54
// IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLinkC2Ev
// DEMANGLED: Inter_MonitorNoticeOtherChannelChatHyperLink::Inter_MonitorNoticeOtherChannelChatHyperLink(void)
Inter_MonitorNoticeOtherChannelChatHyperLink::
    Inter_MonitorNoticeOtherChannelChatHyperLink() {}

// IDA_EA: 0x84F5F7C
// IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLinkD2Ev
// IDA_EA: 0x84F5FAC
// IDA_NAME: _ZN44Inter_MonitorNoticeOtherChannelChatHyperLinkD0Ev
// DEMANGLED: Inter_MonitorNoticeOtherChannelChatHyperLink::~Inter_MonitorNoticeOtherChannelChatHyperLink()
Inter_MonitorNoticeOtherChannelChatHyperLink::
    ~Inter_MonitorNoticeOtherChannelChatHyperLink() {}


namespace {

#pragma pack(push, 1)
struct SIG_MONITOR_MEMBER_LOGIN_OUT {
    PacketHeader header;             // +0x00
    uint8_t event_flag;              // +0x0a
    uint8_t reserved_0b_0e[4];       // +0x0b
    uint32_t target_charac_no;       // +0x0f
    uint8_t notification_type;       // +0x13
    uint8_t upper_member_changed;    // +0x14
    char member_name[30];            // +0x15
    uint8_t upper_member_exp_level;  // +0x33
    uint32_t upper_member_key;       // +0x34
};
#pragma pack(pop)

static_assert(sizeof(SIG_MONITOR_MEMBER_LOGIN_OUT) == 0x38u,
              "member login-out monitor record ABI must stay 0x38 bytes");
static_assert(offsetof(SIG_MONITOR_MEMBER_LOGIN_OUT, target_charac_no) ==
                  0x0fu &&
                  offsetof(SIG_MONITOR_MEMBER_LOGIN_OUT, notification_type) ==
                      0x13u &&
                  offsetof(SIG_MONITOR_MEMBER_LOGIN_OUT, upper_member_changed) ==
                      0x14u &&
                  offsetof(SIG_MONITOR_MEMBER_LOGIN_OUT, member_name) ==
                      0x15u &&
                  offsetof(SIG_MONITOR_MEMBER_LOGIN_OUT, upper_member_exp_level) ==
                      0x33u &&
                  offsetof(SIG_MONITOR_MEMBER_LOGIN_OUT, upper_member_key) ==
                      0x34u,
              "member login-out monitor offsets changed");

}  // namespace


// IDA_EA: 0x84CC90A
// IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOut12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeMemberLoginOut::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeMemberLoginOut::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int payload_size) {
    (void)payload_size;

    if (user->get_state()) {
        if (user->get_charac_memberkey()) {
            const SIG_MONITOR_MEMBER_LOGIN_OUT& reply =
                *reinterpret_cast<const SIG_MONITOR_MEMBER_LOGIN_OUT*>(payload);
            if (user->get_charac_no(-1) ==
                static_cast<int>(reply.target_charac_no)) {
                if (reply.event_flag == 1 && reply.upper_member_changed == 1) {
                    user->set_connect_upper_member(reply.upper_member_key);
                    user->set_uppermember_exp_level(reply.upper_member_exp_level);
                } else if (!reply.event_flag && reply.upper_member_changed == 1) {
                    user->set_connect_upper_member(0);
                    user->set_uppermember_exp_level(0);
                }

                PacketGuard packet;
                packet.put_header(0,
                                  ENUM_NOTIPACKET_MEMBER_MEMBER_LOGIN_OUT);
                packet.put_byte(reply.event_flag);
                packet.put_byte(reply.notification_type);
                packet.put_byte(reply.upper_member_changed);
                const int member_name_length =
                    static_cast<int>(std::strlen(reply.member_name));
                packet.put_int(member_name_length);
                packet.put_str(reply.member_name, member_name_length);
                packet.finalize(true);
                user->Send(packet);
            } else {
                LogManager::logFormat(
                    1,
                    "InterDispatcher.cpp",
                    "virtual int Inter_MonitorNoticeMemberLoginOut::dispatch_sig(CUser*, char*, int)",
                    10172,
                    "MEMBER : Inter_MonitorNoticeMemberLoginOut::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                    user->get_charac_no(-1),
                    static_cast<int>(reply.target_charac_no));
            }
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeMemberLoginOut::dispatch_sig(CUser*, char*, int)",
                10165,
                "MEMBER : Inter_MonitorNoticeMemberLoginOut::dispatch_sig() : 0 == pUser->get_charac_memberkey() (Char id : %d)",
                user->get_charac_no(-1));
        }
    }
    return 0;
}

// IDA_EA: 0x84EAB9E
// IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOutC2Ev
// DEMANGLED: Inter_MonitorNoticeMemberLoginOut::Inter_MonitorNoticeMemberLoginOut(void)
Inter_MonitorNoticeMemberLoginOut::Inter_MonitorNoticeMemberLoginOut() {}

// IDA_EA: 0x84EABBA
// IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOutD2Ev
// IDA_EA: 0x84EABEA
// IDA_NAME: _ZN33Inter_MonitorNoticeMemberLoginOutD0Ev
// DEMANGLED: Inter_MonitorNoticeMemberLoginOut::~Inter_MonitorNoticeMemberLoginOut()
Inter_MonitorNoticeMemberLoginOut::~Inter_MonitorNoticeMemberLoginOut() {}


// IDA_EA: 0x84C93E4
// IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOut12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildLoginOut::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildLoginOut::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    (void)payload_size;

    if (user->get_state()) {
        const SIG_MONITOR_GUILD_LOGIN_OUT& reply =
            *reinterpret_cast<const SIG_MONITOR_GUILD_LOGIN_OUT*>(payload);
        if (user->get_charac_no(-1) == static_cast<int>(reply.target_charac_no)) {
            PacketGuard packet;
            packet.put_header(0, ENUM_NOTIPACKET_GUILD_MEMBER_LOGIN_OUT);
            packet.put_byte(static_cast<int>(
                static_cast<int8_t>(reply.event_flag)));
            packet.put_byte(static_cast<unsigned int>(reply.notification_type));
            const int member_name_length =
                static_cast<int>(std::strlen(reply.member_name));
            packet.put_int(member_name_length);
            packet.put_str(reply.member_name, member_name_length);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeGuildLoginOut::dispatch_sig(CUser*, char*, int)",
                9004,
                "Inter_MonitorNoticeGuildLoginOut::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(reply.target_charac_no));
        }
    }
    return 0;
}

// IDA_EA: 0x84EA568
// IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOutC2Ev
// DEMANGLED: Inter_MonitorNoticeGuildLoginOut::Inter_MonitorNoticeGuildLoginOut(void)
Inter_MonitorNoticeGuildLoginOut::Inter_MonitorNoticeGuildLoginOut() {}

// IDA_EA: 0x84EA584
// IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOutD2Ev
// IDA_EA: 0x84EA5B4
// IDA_NAME: _ZN32Inter_MonitorNoticeGuildLoginOutD0Ev
// DEMANGLED: Inter_MonitorNoticeGuildLoginOut::~Inter_MonitorNoticeGuildLoginOut()
Inter_MonitorNoticeGuildLoginOut::~Inter_MonitorNoticeGuildLoginOut() {}
