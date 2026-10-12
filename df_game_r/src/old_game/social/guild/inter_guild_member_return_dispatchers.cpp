#include "old_game/social/guild/inter_guild_member_return_dispatchers.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <utility>
#include <vector>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/social/party/party.h"
#include "old_game/social/guild/guild_member_packet_runtime.h"
#include "old_game/social/guild/guild_monitor_state_dispatchers_a.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint16_t ReadU16(const char* payload, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84CAA16
// IDA_NAME: _ZN33Inter_MonitorCallGuildMembersNext12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCallGuildMembersNext::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCallGuildMembersNext::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int) {
    if (!user->get_state()) {
        return 0;
    }
    if (user->get_charac_no(-1) != static_cast<int>(ReadU32(payload, 14))) {
        LogManager::logFormat(
            1, "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildMembersNext::dispatch_sig(CUser*, char*, int)",
            0x24E7,
            "Inter_MonitorCallGuildMembersNext::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1), ReadU32(payload, 14));
        return 0;
    }
    const uint32_t authoritative_key = ReadU32(payload, 18);
    const GuildKeySync::Result key_sync =
        GuildKeySync::SynchronizeAuthoritativeNonZero(
            user,
            ReadU32(payload, 14),
            authoritative_key,
            GuildKeySync::kSnapshotRepair);
    if (key_sync != GuildKeySync::kSynchronized) {
        LogManager::logFormat(
            1, "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildMembersNext::dispatch_sig(CUser*, char*, int)",
            0x24ED,
            "Inter_MonitorCallGuildMembersNext::dispatch_sig() : guild key sync rejected (cached=%u authoritative=%u result=%d)",
            static_cast<unsigned int>(user->get_charac_guildkey()),
            static_cast<unsigned int>(authoritative_key),
            static_cast<int>(key_sync));
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GUILD_MEMER_LIST_NEXT);
    packet.put_byte(1);
    packet.put_int(authoritative_key);
    FunctionGuildMember::MakePacketGuildMembersInfo(
        packet, static_cast<unsigned char>(payload[22]),
        reinterpret_cast<ST_Guild_Mem_Info*>(payload + 23));
    packet.finalize(true);
    user->Send(packet);
    FunctionGuildMember::SyncCurrentMemberGrade(user, ReadU32(payload, 14), ReadU32(payload, 18),
        static_cast<unsigned char>(payload[22]), reinterpret_cast<ST_Guild_Mem_Info*>(payload + 23));
    return 0;
}

// IDA_EA: 0x84CABDA
// IDA_NAME: _ZN32Inter_MonitorCallGuildAllMembers12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCallGuildAllMembers::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCallGuildAllMembers::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int) {
    if (!user->get_state()) {
        return 0;
    }
    if (user->get_charac_no(-1) != static_cast<int>(ReadU32(payload, 14))) {
        LogManager::logFormat(
            1, "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildAllMembers::dispatch_sig(CUser*, char*, int)",
            0x250C,
            "Inter_MonitorCallGuildAllMembers::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1), ReadU32(payload, 14));
        return 0;
    }
    if (user->get_charac_guildkey() != static_cast<int>(ReadU32(payload, 18))) {
        LogManager::logFormat(
            1, "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildAllMembers::dispatch_sig(CUser*, char*, int)",
            0x2512,
            "Inter_MonitorCallGuildAllMembers::dispatch_sig() : pUser->get_charac_guildkey() != recv->m_uGuildKey");
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GUILD_ALL_MEMBER_LIST);
    packet.put_byte(1);
    packet.put_int(ReadU32(payload, 18));
    const int guild_name_length = static_cast<int>(std::strlen(payload + 22));
    packet.put_int(guild_name_length);
    packet.put_str(payload + 22, guild_name_length);
    packet.put_int(ReadU32(payload, 45));
    packet.put_short(ReadU16(payload, 49));
    FunctionGuildMember::MakePacketGuildMembersInfo(
        packet, static_cast<unsigned char>(payload[51]),
        reinterpret_cast<ST_Guild_Mem_Info*>(payload + 52));
    packet.finalize(true);
    user->Send(packet);
    FunctionGuildMember::SyncCurrentMemberGrade(user, ReadU32(payload, 14), ReadU32(payload, 18),
        static_cast<unsigned char>(payload[51]), reinterpret_cast<ST_Guild_Mem_Info*>(payload + 52));
    return 0;
}

// IDA_EA: 0x84CAE0A
// IDA_NAME: _ZN36Inter_MonitorCallGuildAllMembersNext12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCallGuildAllMembersNext::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCallGuildAllMembersNext::dispatch_sig(CUser* user,
                                                       char* payload,
                                                       int) {
    if (!user->get_state()) {
        return 0;
    }
    if (user->get_charac_no(-1) != static_cast<int>(ReadU32(payload, 14))) {
        LogManager::logFormat(
            1, "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildAllMembersNext::dispatch_sig(CUser*, char*, int)",
            0x2539,
            "Inter_MonitorCallGuildAllMembersNext::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1), ReadU32(payload, 14));
        return 0;
    }
    if (user->get_charac_guildkey() != static_cast<int>(ReadU32(payload, 18))) {
        LogManager::logFormat(
            1, "InterDispatcher.cpp",
            "virtual int Inter_MonitorCallGuildAllMembersNext::dispatch_sig(CUser*, char*, int)",
            0x253F,
            "Inter_MonitorCallGuildAllMembersNext::dispatch_sig() : pUser->get_charac_guildkey() != recv->m_uGuildKey");
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GUILD_ALL_MEMBER_LIST_NEXT);
    packet.put_byte(1);
    packet.put_int(ReadU32(payload, 18));
    FunctionGuildMember::MakePacketGuildMembersInfo(
        packet, static_cast<unsigned char>(payload[22]),
        reinterpret_cast<ST_Guild_Mem_Info*>(payload + 23));
    packet.finalize(true);
    user->Send(packet);
    FunctionGuildMember::SyncCurrentMemberGrade(user, ReadU32(payload, 14), ReadU32(payload, 18),
        static_cast<unsigned char>(payload[22]), reinterpret_cast<ST_Guild_Mem_Info*>(payload + 23));
    return 0;
}

// IDA_EA: 0x84EA7E4 / 0x84EA800 / 0x84EA830
// IDA_NAME: _ZN33Inter_MonitorCallGuildMembersNextC2Ev / D2Ev / D0Ev
Inter_MonitorCallGuildMembersNext::Inter_MonitorCallGuildMembersNext()
    : ISigDispatcher() {}
Inter_MonitorCallGuildMembersNext::~Inter_MonitorCallGuildMembersNext() {}

// IDA_EA: 0x84EA84E / 0x84EA86A / 0x84EA89A
// IDA_NAME: _ZN32Inter_MonitorCallGuildAllMembersC2Ev / D2Ev / D0Ev
Inter_MonitorCallGuildAllMembers::Inter_MonitorCallGuildAllMembers()
    : ISigDispatcher() {}
Inter_MonitorCallGuildAllMembers::~Inter_MonitorCallGuildAllMembers() {}

// IDA_EA: 0x84EA8B8 / 0x84EA8D4 / 0x84EA904
// IDA_NAME: _ZN36Inter_MonitorCallGuildAllMembersNextC2Ev / D2Ev / D0Ev
Inter_MonitorCallGuildAllMembersNext::Inter_MonitorCallGuildAllMembersNext()
    : ISigDispatcher() {}
Inter_MonitorCallGuildAllMembersNext::~Inter_MonitorCallGuildAllMembersNext() {}

// IDA_EA: 0x84CC35C
// IDA_NAME: _ZN35Inter_MonitorSetSubGuildMasterReply12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorSetSubGuildMasterReply::dispatch_sig(CUser *,char *,int)
int Inter_MonitorSetSubGuildMasterReply::dispatch_sig(CUser* user,
                                                      char* payload,
                                                      int) {
    if (user->get_state() &&
        user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 18))) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_SET_SUB_GUILD_MASTER);

        const uint32_t result = ReadU32(payload, 10);
        if (result) {
            packet.put_byte(0);
            packet.put_byte(static_cast<unsigned char>(result));
        } else {
            packet.put_byte(1);
            packet.put_byte(static_cast<unsigned char>(result));

            const int target_name_length =
                static_cast<int>(std::strlen(payload + 23));
            packet.put_int(target_name_length);
            packet.put_str(payload + 23, target_name_length);
            packet.put_byte(static_cast<unsigned char>(payload[22]));

            user->history_log().GuildChangeGrade(
                static_cast<unsigned int>(user->get_charac_guildkey()),
                user->GetGuildDBInfo()->guild_name,
                ReadU32(payload, 18),
                ReadU32(payload, 54),
                static_cast<unsigned char>(payload[53]),
                static_cast<unsigned char>(payload[22]));
        }

        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84CC768
// IDA_NAME: _ZN34Inter_MonitorNoticeGuildMemberInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeGuildMemberInfo::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeGuildMemberInfo::dispatch_sig(CUser* user,
                                                     char* payload,
                                                     int) {
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
        user->setGuildMemberGrade(static_cast<unsigned char>(payload[39]));

        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_GUILD_MEMBER_INFO);
        packet.put_byte(static_cast<unsigned char>(payload[39]));
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
    return 0;
}

// IDA_EA: 0x84CCBA0
// IDA_NAME: _ZN48Inter_MonitorNoticeRequestMemberEnterToRequester12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeRequestMemberEnterToRequester::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeRequestMemberEnterToRequester::dispatch_sig(
    CUser* user,
    char* payload,
    int) {
    if (user->get_state()) {
        if (user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_REQUEST_MEMBER_ENTER);
            if (payload[18]) {
                packet.put_byte(0);
                packet.put_byte(static_cast<unsigned char>(payload[18]));
            } else {
                packet.put_byte(1);
            }

            const int character_name_length =
                static_cast<int>(std::strlen(payload + 19));
            packet.put_int(character_name_length);
            packet.put_str(payload + 19, character_name_length);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeRequestMemberEnterToRequester::dispatch_sig(CUser*, char*, int)",
                0x27E8,
                "MEMBER : Inter_MonitorNoticeRequestMemberEnterToRequester::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(ReadU32(payload, 14)));
        }
    }
    return 0;
}

// IDA_EA: 0x84CCD54
// IDA_NAME: _ZN48Inter_MonitorNoticeRequestMemberEnterToResponser12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeRequestMemberEnterToResponser::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeRequestMemberEnterToResponser::dispatch_sig(
    CUser* user,
    char* payload,
    int) {
    if (user->get_state()) {
        if (user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
            if (user->alonePlay_Mode()) {
                Packet_Monitor_Member_Enter_Reply reply;
                reply.uid = static_cast<uint32_t>(user->GetUID());
                reply.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
                reply.reply_kind = 3;
                GlobalData::s_monitor_proxy_mgr
                    ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(
                        user->GetServerGroup()))
                    ->SendTcpPacket(reinterpret_cast<char*>(&reply), 0x13u);
            } else if (user->GetGameOptionRef()->get_etc_option(0x1d) == 1) {
                Packet_Monitor_Member_Enter_Reply reply;
                reply.uid = static_cast<uint32_t>(user->GetUID());
                reply.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
                reply.reply_kind = 4;
                GlobalData::s_monitor_proxy_mgr
                    ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(
                        user->GetServerGroup()))
                    ->SendTcpPacket(reinterpret_cast<char*>(&reply), 0x13u);
            } else {
                PacketGuard packet;
                packet.put_header(
                    0, ENUM_NOTIPACKET_REQUEST_MEMBER_ENTER_TO_RESPONSER);
                packet.put_short(ReadU16(payload, 18));
                const int character_name_length =
                    static_cast<int>(std::strlen(payload + 20));
                packet.put_int(character_name_length);
                packet.put_str(payload + 20, character_name_length);
                packet.finalize(true);
                user->Send(packet);
            }
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeRequestMemberEnterToResponser::dispatch_sig(CUser*, char*, int)",
                0x280C,
                "MEMBER : Inter_MonitorNoticeRequestMemberEnterToResponser::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(ReadU32(payload, 14)));
        }
    }
    return 0;
}

// IDA_EA: 0x84CD218
// IDA_NAME: _ZN32Inter_MonitorNoticeMemberEnterOk12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeMemberEnterOk::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeMemberEnterOk::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int) {
    if (user->get_state()) {
        if (user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 17))) {
            switch (static_cast<unsigned char>(payload[10])) {
            case 2: {
                PacketGuard packet;
                packet.put_header(0, ENUM_NOTIPACKET_MEMBER_ENTER_OK);
                packet.put_byte(static_cast<unsigned char>(payload[10]));
                packet.put_byte(static_cast<unsigned char>(payload[11]));
                packet.put_byte(static_cast<unsigned char>(payload[21]));
                const int character_name_length =
                    static_cast<int>(std::strlen(payload + 22));
                packet.put_int(character_name_length);
                packet.put_str(payload + 22, character_name_length);
                packet.finalize(true);
                user->Send(packet);
                break;
            }
            case 3:
                user->Send_SoloPlay_Reject_NotiPack(payload + 22, 1, 6);
                break;
            case 4:
                user->Send_SoloPlay_Reject_NotiPack(payload + 22, 0, 6);
                break;
            default: {
                if (payload[11] == 1 && payload[12] == 1) {
                    user->set_connect_upper_member(ReadU32(payload, 53));
                    user->set_uppermember_exp_level(
                        static_cast<unsigned char>(payload[52]));
                    CParty* const party = user->GetParty();
                    if (party) {
                        party->send_party_realtime_info();
                    }
                }
                user->set_charac_memberkey(ReadU32(payload, 17));

                PacketGuard packet;
                packet.put_header(0, ENUM_NOTIPACKET_MEMBER_ENTER_OK);
                packet.put_byte(static_cast<unsigned char>(payload[10]));
                packet.put_byte(static_cast<unsigned char>(payload[11]));
                packet.put_byte(static_cast<unsigned char>(payload[21]));
                const int character_name_length =
                    static_cast<int>(std::strlen(payload + 22));
                packet.put_int(character_name_length);
                packet.put_str(payload + 22, character_name_length);
                packet.finalize(true);
                user->Send(packet);
                break;
            }
            }
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeMemberEnterOk::dispatch_sig(CUser*, char*, int)",
                0x288B,
                "MEMBER : Inter_MonitorNoticeMemberEnterOk::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(ReadU32(payload, 17)));
        }
    }
    return 0;
}

// IDA_EA: 0x84CD5CA
// IDA_NAME: _ZN35Inter_MonitorNoticeMemberEnterReply12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeMemberEnterReply::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeMemberEnterReply::dispatch_sig(CUser* user,
                                                      char* payload,
                                                      int) {
    if (user->get_state()) {
        if (user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 17))) {
            const unsigned char action = static_cast<unsigned char>(payload[10]);
            if (action != 2 && action != 3 && action != 4) {
                if (payload[11] == 1 || payload[11] == 2) {
                    if (payload[11] == 1 && payload[12] == 1) {
                        user->set_uppermember_exp_level(
                            static_cast<unsigned char>(payload[52]));
                        user->set_connect_upper_member(ReadU32(payload, 53));
                        CParty* const party = user->GetParty();
                        if (party) {
                            party->send_party_realtime_info();
                        }
                    }
                    user->set_charac_memberkey(ReadU32(payload, 17));
                }

                PacketGuard packet;
                packet.put_header(1, ENUM_CMDPACKET_MEMBER_ENTER_REPLY);
                packet.put_byte(1);
                packet.put_byte(static_cast<unsigned char>(payload[11]));
                packet.put_byte(static_cast<unsigned char>(payload[21]));
                const int character_name_length =
                    static_cast<int>(std::strlen(payload + 22));
                packet.put_int(character_name_length);
                packet.put_str(payload + 22, character_name_length);
                packet.finalize(true);
                user->Send(packet);
            }
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeMemberEnterReply::dispatch_sig(CUser*, char*, int)",
                0x28D9,
                "MEMBER : Inter_MonitorNoticeMemberEnterReply::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(ReadU32(payload, 17)));
        }
    }
    return 0;
}

// IDA_EA: 0x84CD83A
// IDA_NAME: _ZN31Inter_MonitorNoticeMemberSecede12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeMemberSecede::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeMemberSecede::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int) {
    if (user->get_state()) {
        if (user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
            const unsigned char reason = static_cast<unsigned char>(payload[18]);
            if (reason == 1) {
                user->set_connect_upper_member(0);
                user->set_uppermember_exp_level(0);
                CParty* const party = user->GetParty();
                if (party) {
                    party->send_party_realtime_info();
                }
            }

            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_MEMBER_SECEDE);
            packet.put_byte(reason == 1 || reason == 2 ? 1 : 0);
            const int character_name_length =
                static_cast<int>(std::strlen(payload + 19));
            packet.put_int(character_name_length);
            packet.put_str(payload + 19, character_name_length);
            packet.put_byte(reason == 1 || reason == 2 ? 1 : reason);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeMemberSecede::dispatch_sig(CUser*, char*, int)",
                0x2914,
                "MEMBER : Inter_MonitorNoticeMemberSecede::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCallerCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(ReadU32(payload, 14)));
        }
    }
    return 0;
}

// IDA_EA: 0x84CDA74
// IDA_NAME: _ZN40Inter_MonitorNoticeMemberSecedeToSeceder12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeMemberSecedeToSeceder::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeMemberSecedeToSeceder::dispatch_sig(CUser* user,
                                                           char* payload,
                                                           int) {
    if (user->get_state()) {
        if (user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
            if (payload[18] == 1) {
                user->set_connect_upper_member(0);
                user->set_uppermember_exp_level(0);
                CParty* const party = user->GetParty();
                if (party) {
                    party->send_party_realtime_info();
                }
            }

            PacketGuard packet;
            packet.put_header(0, ENUM_NOTIPACKET_MEMBER_SECEDE);
            const int character_name_length =
                static_cast<int>(std::strlen(payload + 19));
            packet.put_int(character_name_length);
            packet.put_str(payload + 19, character_name_length);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeMemberSecedeToSeceder::dispatch_sig(CUser*, char*, int)",
                0x2947,
                "MEMBER : Inter_MonitorNoticeMemberSecede::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uSecededCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(ReadU32(payload, 14)));
        }
    }
    return 0;
}

// IDA_EA: 0x84CDC2C
// IDA_NAME: _ZN33Inter_MonitorNoticeDeleteMemberId12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeDeleteMemberId::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeDeleteMemberId::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int) {
    if (user->get_state()) {
        if (user->get_charac_no(-1) == static_cast<int>(ReadU32(payload, 14))) {
            user->set_connect_upper_member(0);
            user->set_uppermember_exp_level(0);
            user->set_charac_memberkey(0);
            CParty* const party = user->GetParty();
            if (party) {
                party->send_party_realtime_info();
            }

            PacketGuard packet;
            packet.put_header(0, ENUM_NOTIPACKET_DELETE_MEMBER_ID);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeDeleteMemberId::dispatch_sig(CUser*, char*, int)",
                0x296D,
                "MEMBER : Inter_MonitorNoticeDeleteMemberId::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(ReadU32(payload, 14)));
        }
    }
    return 0;
}

// IDA_EA: 0x84CCFDA
// IDA_NAME: _ZN32Inter_MonitorNoticeMemberChatMsg12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeMemberChatMsg::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeMemberChatMsg::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int) {
    if (user->get_state()) {
        if (user->get_charac_memberkey()) {
            if (user->get_charac_no(-1) ==
                static_cast<int>(ReadU32(payload, 14))) {
                PacketGuard packet;
                packet.put_header(0, ENUM_NOTIPACKET_MESSAGE_OTHER_CHANNEL);
                packet.put_byte(8);
                packet.put_byte(0);
                const int sender_name_length =
                    static_cast<int>(std::strlen(payload + 18));
                packet.put_int(sender_name_length);
                packet.put_str(payload + 18, sender_name_length);
                packet.put_byte(0);
                const int message_length =
                    static_cast<unsigned char>(payload[48]);
                packet.put_int(message_length);
                packet.put_str(payload + 49, message_length);
                packet.finalize(true);
                user->Send(packet);
            } else {
                LogManager::logFormat(
                    1,
                    "InterDispatcher.cpp",
                    "virtual int Inter_MonitorNoticeMemberChatMsg::dispatch_sig(CUser*, char*, int)",
                    0x2859,
                    "MEMBER : Inter_MonitorNoticeMemberChatMsg::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                    user->get_charac_no(-1),
                    static_cast<int>(ReadU32(payload, 14)));
            }
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorNoticeMemberChatMsg::dispatch_sig(CUser*, char*, int)",
                0x2852,
                "MEMBER : Inter_MonitorNoticeMemberChatMsg::dispatch_sig() : 0 == pUser->get_charac_memberkey() (Char id : %d)",
                user->get_charac_no(-1));
        }
    }
    return 0;
}

// IDA_EA: 0x84CDDAC
// IDA_NAME: _ZN27Inter_MonitorCallMemberList12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCallMemberList::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCallMemberList::dispatch_sig(CUser* user,
                                              char* payload,
                                              int) {
    if (user->get_state()) {
        if (user->get_charac_memberkey()) {
            if (user->get_charac_no(-1) ==
                static_cast<int>(ReadU32(payload, 14))) {
                PacketGuard packet;
                packet.put_header(1, ENUM_CMDPACKET_CALL_MEMER_LIST);
                packet.put_byte(1);

                const char* const leader = payload + 18;
                packet.put_byte(static_cast<unsigned char>(leader[0]));
                packet.put_byte(static_cast<unsigned char>(leader[1]));
                const int leader_name_length =
                    static_cast<int>(std::strlen(leader + 2));
                packet.put_int(leader_name_length);
                packet.put_str(leader + 2, leader_name_length);
                packet.put_byte(static_cast<unsigned char>(leader[32]));
                packet.put_byte(static_cast<unsigned char>(leader[33]));
                packet.put_int(static_cast<int>(ReadU32(leader, 34)));
                packet.put_int(static_cast<int>(ReadU32(leader, 38)));

                const unsigned char member_count =
                    static_cast<unsigned char>(leader[42]);
                packet.put_byte(member_count);
                const char* const members = leader + 43;
                for (int index = 0; index < member_count; ++index) {
                    const char* const member = members + index * 42;
                    packet.put_byte(static_cast<unsigned char>(member[0]));
                    packet.put_byte(static_cast<unsigned char>(member[1]));
                    const int member_name_length =
                        static_cast<int>(std::strlen(member + 2));
                    packet.put_int(member_name_length);
                    packet.put_str(member + 2, member_name_length);
                    packet.put_byte(static_cast<unsigned char>(member[32]));
                    packet.put_byte(static_cast<unsigned char>(member[33]));
                    packet.put_int(static_cast<int>(ReadU32(member, 34)));
                    packet.put_int(static_cast<int>(ReadU32(member, 38)));
                }
                packet.finalize(true);
                user->Send(packet);
            } else {
                LogManager::logFormat(
                    1,
                    "InterDispatcher.cpp",
                    "virtual int Inter_MonitorCallMemberList::dispatch_sig(CUser*, char*, int)",
                    0x2993,
                    "MEMBER : Inter_MonitorCallMemberList::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                    user->get_charac_no(-1),
                    static_cast<int>(ReadU32(payload, 14)));
            }
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_MonitorCallMemberList::dispatch_sig(CUser*, char*, int)",
                0x298C,
                "MEMBER : Inter_MonitorCallMemberList::dispatch_sig() : 0 == pUser->get_charac_memberkey() (Char id : %d)",
                user->get_charac_no(-1));
        }
    }
    return 0;
}

// IDA_EA: 0x84CE164
// IDA_NAME: _ZN26Inter_MonitorPayTaxToUpper12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorPayTaxToUpper::dispatch_sig(CUser *,char *,int)
int Inter_MonitorPayTaxToUpper::dispatch_sig(CUser* user,
                                             char* payload,
                                             int) {
    if (!user->get_state()) {
        return 0;
    }
    if (!user->get_charac_memberkey()) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorPayTaxToUpper::dispatch_sig(CUser*, char*, int)",
            0x29E7,
            "MEMBER : Inter_MonitorPayTaxToUpper::dispatch_sig() : 0 == pUser->get_charac_memberkey() (Char id : %d)",
            user->get_charac_no(-1));
        return 0;
    }
    if (user->get_charac_no(-1) != static_cast<int>(ReadU32(payload, 14))) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_MonitorPayTaxToUpper::dispatch_sig(CUser*, char*, int)",
            0x29ED,
            "MEMBER : Inter_MonitorCallMemberList::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
            user->get_charac_no(-1),
            static_cast<int>(ReadU32(payload, 14)));
        return 0;
    }

    const uint32_t money = ReadU32(payload, 18);
    if (money) {
        user->getCurCharacInvenW()->gain_money(
            static_cast<int>(money), static_cast<eMoneyAddReason>(10), true, 0);
    }

    int granted_fatigue = 0;
    bool member_bonus_fatigue_limit_hit = false;
    const uint32_t requested_fatigue = ReadU32(payload, 22);
    if (requested_fatigue) {
        const ServerParameterScript* const script =
            G_CDataManager()->get_server_parameter_script();
        const int charac_remaining =
            script->get_member_bonus_fatigue_charac_limit() -
            static_cast<int>(static_cast<int16_t>(
                user->getCurCharacMemberBonusFatigue()));
        const int account_remaining =
            script->get_member_bonus_fatigue_account_limit() -
            static_cast<int>(static_cast<int16_t>(
                user->getAccountMemberBonusFatigue()));
        const unsigned int available_fatigue =
            static_cast<unsigned int>(std::max(
                0, std::min(charac_remaining, account_remaining)));
        if (available_fatigue) {
            if (available_fatigue <= requested_fatigue) {
                granted_fatigue = static_cast<int>(available_fatigue);
                member_bonus_fatigue_limit_hit = true;
            } else {
                granted_fatigue = static_cast<int>(requested_fatigue);
            }
        }

        if (granted_fatigue > 0) {
            user->gainCurCharacFatigue(granted_fatigue);
            user->incCurCharacMemberBonusFatigue(
                static_cast<int16_t>(granted_fatigue));
            user->EnableSaveMemberBonusFatigue();
            user->incAccountMemberBonusFatigue(
                static_cast<int16_t>(granted_fatigue));
        }
    }

    if (money || granted_fatigue) {
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_PAY_TAX_TO_UPPER);
        packet.put_int(static_cast<int>(money));
        packet.put_short(granted_fatigue);
        const int character_name_length =
            static_cast<int>(std::strlen(payload + 26));
        packet.put_int(character_name_length);
        packet.put_str(payload + 26, character_name_length);
        packet.finalize(true);
        user->Send(packet);

        if (member_bonus_fatigue_limit_hit) {
            PacketGuard limit_packet;
            limit_packet.put_header(
                0, ENUM_NOTIPACKET_MEMBER_BONUS_FATIGUE_LIMIT);
            limit_packet.finalize(true);
            user->Send(limit_packet);
        }
    }
    return 0;
}

// IDA_EA: 0x84CE7F6
// IDA_NAME: _ZN25Inter_MonitorGuildWarInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorGuildWarInfo::dispatch_sig(CUser *,char *,int)
int Inter_MonitorGuildWarInfo::dispatch_sig(CUser* user, char* payload, int) {
    if (user->get_state()) {
        GameWorld* const game_world = G_GameWorld();
        if (game_world->GetChannelType() == 6) {
            if (user->get_charac_no(-1) ==
                static_cast<int>(ReadU32(payload, 14))) {
                PacketGuard packet;
                packet.put_header(1, ENUM_CMDPACKET_GUILD_WAR_INFO);
                packet.put_byte(1);

                int record_count = 0;
                for (int index = 0; index <= 9; ++index) {
                    if (ReadU32(payload, 33 * index + 18)) {
                        ++record_count;
                    }
                }
                packet.put_byte(record_count);

                // The original serializes the first count records rather
                // than compacting the nonzero entries found above.
                for (int index = 0; index < record_count; ++index) {
                    const int offset = 33 * index + 18;
                    packet.put_int(ReadU32(payload, offset));
                    packet.put_int(ReadU32(payload, offset + 4));
                    packet.put_short(ReadU16(payload, offset + 8));
                    const int name_length = static_cast<int>(
                        std::strlen(payload + offset + 10));
                    packet.put_int(name_length);
                    packet.put_str(payload + offset + 10, name_length);
                }
                packet.finalize(true);
                user->Send(packet);
            } else {
                LogManager::logFormat(
                    1, "InterDispatcher.cpp",
                    "virtual int Inter_MonitorGuildWarInfo::dispatch_sig(CUser*, char*, int)",
                    0x2AD4,
                    "GuildWarInfo : Inter_MonitorGuildWarInfo::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                    user->get_charac_no(-1), ReadU32(payload, 14));
            }
        }
    }
    return 0;
}

// IDA_EA: 0x82A5A4C
// IDA_NAME: _ZN26Inter_MonitorGuildPointDel17getLastDeleteTimeEv
// DEMANGLED: Inter_MonitorGuildPointDel::getLastDeleteTime(void)
int Inter_MonitorGuildPointDel::getLastDeleteTime() {
    return static_cast<int>(g_lastDeleteTime);
}

// IDA_EA: 0x84CE6D2
// IDA_NAME: _ZN26Inter_MonitorGuildPointDel16_makeDefaultTimeEv
// DEMANGLED: Inter_MonitorGuildPointDel::_makeDefaultTime(void)
time_t Inter_MonitorGuildPointDel::_makeDefaultTime() {
    time_t timer = ::time(0);
    struct tm local_time;
    struct tm* const local = ::localtime_r(&timer, &local_time);
    local->tm_mday = 1;
    local->tm_hour = 5;
    local->tm_min = 0;
    local->tm_sec = 0;
    if (local->tm_mon == 6 && local->tm_year == 106) {
        local->tm_mday = 6;
    }
    return ::mktime(local);
}

time_t Inter_MonitorGuildPointDel::g_lastDeleteTime =
    Inter_MonitorGuildPointDel::_makeDefaultTime();

// IDA_EA: 0x84CE756
// IDA_NAME: _ZN26Inter_MonitorGuildPointDel12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorGuildPointDel::dispatch_sig(CUser *,char *,int)
int Inter_MonitorGuildPointDel::dispatch_sig(CUser*, char*, int) {
    std::vector<std::pair<int, int> > matched_items;
    matched_items.push_back(std::make_pair(3204, 0));
    G_GameWorld()->deleteSpecificItems(&matched_items);
    setLastDeleteTime();
    return 0;
}

// IDA_EA: 0x84EB100
// IDA_NAME: _ZN26Inter_MonitorGuildPointDel17setLastDeleteTimeEv
// DEMANGLED: Inter_MonitorGuildPointDel::setLastDeleteTime(void)
time_t Inter_MonitorGuildPointDel::setLastDeleteTime() {
    g_lastDeleteTime = ::time(0);
    return g_lastDeleteTime;
}

// IDA_EA: 0x84ED540 / 0x84FA014 / 0x84FA044
// IDA_NAME: _ZN35Inter_MonitorSetSubGuildMasterReplyC2Ev / D2Ev / D0Ev
Inter_MonitorSetSubGuildMasterReply::Inter_MonitorSetSubGuildMasterReply()
    : ISigDispatcher() {}
Inter_MonitorSetSubGuildMasterReply::~Inter_MonitorSetSubGuildMasterReply() {}

// IDA_EA: 0x84ED55C / 0x84F9F2A / 0x84F9F5A
// IDA_NAME: _ZN34Inter_MonitorNoticeGuildMemberInfoC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeGuildMemberInfo::Inter_MonitorNoticeGuildMemberInfo()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildMemberInfo::~Inter_MonitorNoticeGuildMemberInfo() {}

// IDA_EA: 0x84EAC08 / 0x84EAC24 / 0x84EAC54
// IDA_NAME: _ZN48Inter_MonitorNoticeRequestMemberEnterToRequesterC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeRequestMemberEnterToRequester::
    Inter_MonitorNoticeRequestMemberEnterToRequester()
    : ISigDispatcher() {}
Inter_MonitorNoticeRequestMemberEnterToRequester::
    ~Inter_MonitorNoticeRequestMemberEnterToRequester() {}

// IDA_EA: 0x84EAC72 / 0x84EAC8E / 0x84EACBE
// IDA_NAME: _ZN48Inter_MonitorNoticeRequestMemberEnterToResponserC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeRequestMemberEnterToResponser::
    Inter_MonitorNoticeRequestMemberEnterToResponser()
    : ISigDispatcher() {}
Inter_MonitorNoticeRequestMemberEnterToResponser::
    ~Inter_MonitorNoticeRequestMemberEnterToResponser() {}

// IDA_EA: 0x84EACDC / 0x84EACF8 / 0x84EAD28
// IDA_NAME: _ZN32Inter_MonitorNoticeMemberEnterOkC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeMemberEnterOk::Inter_MonitorNoticeMemberEnterOk()
    : ISigDispatcher() {}
Inter_MonitorNoticeMemberEnterOk::~Inter_MonitorNoticeMemberEnterOk() {}

// IDA_EA: 0x84EAD46 / 0x84EAD62 / 0x84EAD92
// IDA_NAME: _ZN35Inter_MonitorNoticeMemberEnterReplyC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeMemberEnterReply::Inter_MonitorNoticeMemberEnterReply()
    : ISigDispatcher() {}
Inter_MonitorNoticeMemberEnterReply::~Inter_MonitorNoticeMemberEnterReply() {}

// IDA_EA: 0x84EADB0 / 0x84EADCC / 0x84EADFC
// IDA_NAME: _ZN31Inter_MonitorNoticeMemberSecedeC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeMemberSecede::Inter_MonitorNoticeMemberSecede()
    : ISigDispatcher() {}
Inter_MonitorNoticeMemberSecede::~Inter_MonitorNoticeMemberSecede() {}

// IDA_EA: 0x84EAE1A / 0x84EAE36 / 0x84EAE66
// IDA_NAME: _ZN40Inter_MonitorNoticeMemberSecedeToSecederC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeMemberSecedeToSeceder::
    Inter_MonitorNoticeMemberSecedeToSeceder()
    : ISigDispatcher() {}
Inter_MonitorNoticeMemberSecedeToSeceder::
    ~Inter_MonitorNoticeMemberSecedeToSeceder() {}

// IDA_EA: 0x84EAE84 / 0x84EAEA0 / 0x84EAED0
// IDA_NAME: _ZN33Inter_MonitorNoticeDeleteMemberIdC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeDeleteMemberId::Inter_MonitorNoticeDeleteMemberId()
    : ISigDispatcher() {}
Inter_MonitorNoticeDeleteMemberId::~Inter_MonitorNoticeDeleteMemberId() {}

// IDA_EA: 0x84EAEEE / 0x84EAF0A / 0x84EAF3A
// IDA_NAME: _ZN32Inter_MonitorNoticeMemberChatMsgC2Ev / D2Ev / D0Ev
Inter_MonitorNoticeMemberChatMsg::Inter_MonitorNoticeMemberChatMsg()
    : ISigDispatcher() {}
Inter_MonitorNoticeMemberChatMsg::~Inter_MonitorNoticeMemberChatMsg() {}

// IDA_EA: 0x84EAF58 / 0x84EAF74 / 0x84EAFA4
// IDA_NAME: _ZN27Inter_MonitorCallMemberListC2Ev / D2Ev / D0Ev
Inter_MonitorCallMemberList::Inter_MonitorCallMemberList()
    : ISigDispatcher() {}
Inter_MonitorCallMemberList::~Inter_MonitorCallMemberList() {}

// IDA_EA: 0x84EAFC2 / 0x84EAFDE / 0x84EB00E
// IDA_NAME: _ZN26Inter_MonitorPayTaxToUpperC2Ev / D2Ev / D0Ev
Inter_MonitorPayTaxToUpper::Inter_MonitorPayTaxToUpper()
    : ISigDispatcher() {}
Inter_MonitorPayTaxToUpper::~Inter_MonitorPayTaxToUpper() {}

// IDA_EA: 0x84EB184 / 0x84EB1A0 / 0x84EB1D0
// IDA_NAME: _ZN25Inter_MonitorGuildWarInfoC2Ev / D2Ev / D0Ev
Inter_MonitorGuildWarInfo::Inter_MonitorGuildWarInfo()
    : ISigDispatcher() {}
Inter_MonitorGuildWarInfo::~Inter_MonitorGuildWarInfo() {}

// IDA_EA: 0x84EB11A / 0x84EB136 / 0x84EB166
// IDA_NAME: _ZN26Inter_MonitorGuildPointDelC2Ev / D2Ev / D0Ev
Inter_MonitorGuildPointDel::Inter_MonitorGuildPointDel()
    : ISigDispatcher() {}
Inter_MonitorGuildPointDel::~Inter_MonitorGuildPointDel() {}
