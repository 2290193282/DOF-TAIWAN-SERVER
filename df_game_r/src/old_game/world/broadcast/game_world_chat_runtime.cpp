#include "old_game/world/region/game_world.h"

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "core/config/env/environment.h"
#include "old_game/gm/core/cgm_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/slang_filter.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/channel/packet_dispatch/channel_packets.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>
#include <string>

// IDA_EA: 0x86C975E
// IDA_NAME: _ZN9GameWorld13send_chat_msgEP5CUserctjPciS2_i15ENUM_NOTIPACKET
// DEMANGLED: GameWorld::send_chat_msg(CUser *,char,unsigned short,unsigned int,char *,int,char *,int,ENUM_NOTIPACKET)
void GameWorld::send_chat_msg(CUser* user,
                              char chat_type,
                              uint16_t item_space,
                              uint32_t item_position,
                              char* message,
                              int message_length,
                              char* target_name,
                              int target_name_length,
                              ENUM_NOTIPACKET notification) {
    const unsigned int type_value = static_cast<unsigned int>(chat_type);
    if (type_value <= 0x1Eu &&
        ((1u << type_value) & 0x7FFF0411u) != 0u &&
        user) {
        user->DisConnSig(static_cast<DISCONN_SIG>(24), true, 0);
        return;
    }

    if (chat_type != 0) {
        const std::string filtered =
            CSyncSlangFilter::GetInstance()->Filter(std::string(message));
        if (filtered.empty()) {
            return;
        }
        std::strncpy(message, filtered.c_str(), filtered.size());
    }

    PacketGuard packet;
    if (user && IsForbiddenChat(user->getCurCharacName())) {
        const unsigned int remaining =
            GetLeftTimeOfForbiddenChat(user->getCurCharacName());
        if (remaining) {
            packet.put_header(0, 12);
            packet.put_byte(10);
            packet.put_short(0);
            cMyTrace trace(
                "void GameWorld::send_chat_msg(CUser*, char, short unsigned int, unsigned int, char*, int, char*, int, ENUM_NOTIPACKET)",
                3571,
                0);
            trace("%s is forbidden to chat for %d msec.\n",
                  user->getCurCharacName(),
                  remaining);
            packet.put_int(static_cast<int>(remaining / 60000u));
            packet.finalize(true);
            user->Send(packet);
        } else {
            EnableChat(user->getCurCharacName());
        }
        return;
    }

    WriteLogChatMsg(user, chat_type, message);
    switch (chat_type) {
    case 0:
        if (user) {
            user->get_acc_id();
        } else {
            make_packet_chat_msg(packet,
                                 0,
                                 message,
                                 message_length,
                                 chat_type,
                                 notification);
            send_all(packet);
        }
        break;

    case 1:
    case 33: {
        CUser* const recipient = find_from_world(item_space);
        if (recipient) {
            PacketGuard whisper;
            if (recipient->isBlackUser(user->get_charac_no(-1)) ||
                user->isBlackUser(recipient->get_charac_no(-1))) {
                whisper.put_header(0, 65);
                whisper.put_byte(chat_type);
                whisper.put_byte(77);
                const int name_length =
                    static_cast<int>(std::strlen(
                        recipient->getCurCharacName()));
                whisper.put_int(name_length);
                whisper.put_str(
                    const_cast<char*>(recipient->getCurCharacName()),
                    name_length);
                whisper.put_byte(0);
                whisper.put_int(message_length);
                whisper.put_str(message, message_length);
                whisper.finalize(true);
                user->Send(whisper);
            } else if (GlobalData::s_GM_Manager->IsGm(
                           recipient->get_acc_id())) {
                whisper.put_header(0, 65);
                whisper.put_byte(chat_type);
                whisper.put_byte(90);
                const int name_length =
                    static_cast<int>(std::strlen(
                        recipient->getCurCharacName()));
                whisper.put_int(name_length);
                whisper.put_str(
                    const_cast<char*>(recipient->getCurCharacName()),
                    name_length);
                whisper.put_byte(0);
                whisper.put_int(message_length);
                whisper.put_str(message, message_length);
                whisper.finalize(true);
                user->Send(whisper);
            } else if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
                       user->GetServerGroup() !=
                           recipient->GetServerGroup()) {
                whisper.put_header(0, 65);
                whisper.put_byte(chat_type);
                whisper.put_byte(21);
                const int name_length =
                    static_cast<int>(std::strlen(
                        recipient->getCurCharacName()));
                whisper.put_int(name_length);
                whisper.put_str(
                    const_cast<char*>(recipient->getCurCharacName()),
                    name_length);
                whisper.put_byte(0);
                whisper.put_int(message_length);
                whisper.put_str(message, message_length);
                whisper.finalize(true);
                user->Send(whisper);
            } else {
                whisper.put_header(0, 65);
                whisper.put_byte(chat_type);
                whisper.put_byte(0);
                const int name_length =
                    static_cast<int>(std::strlen(user->getCurCharacName()));
                whisper.put_int(name_length);
                whisper.put_str(
                    const_cast<char*>(user->getCurCharacName()),
                    name_length);
                whisper.put_byte(0);
                whisper.put_int(message_length);
                whisper.put_str(message, message_length);
                whisper.finalize(true);
                recipient->Send(whisper);
            }
        } else {
            Packet_Monitor_Other_Channel_Chat other_channel;
            other_channel.chat_type =
                static_cast<unsigned char>(chat_type);
            other_channel.charac_no =
                static_cast<uint32_t>(user->getCurCharacNo());
            other_channel.item_position = item_position;
            other_channel.target_name_length =
                static_cast<unsigned char>(target_name_length);
            std::memcpy(other_channel.target_name,
                        target_name,
                        static_cast<size_t>(target_name_length));
            other_channel.message_length =
                static_cast<unsigned char>(message_length);
            std::memcpy(other_channel.message,
                        message,
                        static_cast<size_t>(message_length));
            other_channel.header.size =
                static_cast<uint16_t>(message_length + 59);
            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(
                        user->GetServerGroup()));
            proxy->SendPacket(
                reinterpret_cast<char*>(&other_channel),
                static_cast<int>(other_channel.header.size));
        }
        break;
    }

    case 2:
        if (user) {
            make_packet_chat_msg(packet,
                                 user,
                                 message,
                                 message_length,
                                 chat_type,
                                 notification);
            CParty* const party = user->GetParty();
            if (party) {
                party->send_to_party(packet);
            }
        }
        break;

    case 3: {
        Village* const village = G_GameWorld()->getUserVillage(user);
        if (!village) {
            cMyTrace trace(
                "void GameWorld::send_chat_msg(CUser*, char, short unsigned int, unsigned int, char*, int, char*, int, ENUM_NOTIPACKET)",
                3869,
                5);
            trace("getUserVillage() call failed");
            break;
        }

        bool send_area_message =
            village->get_gate_area() == user->get_area(false) ||
            user->getCurCharacVill() == 8 ||
            static_cast<unsigned int>(notification) == 114u ||
            static_cast<unsigned int>(notification) == 119u;
        if (!send_area_message) {
            ServerParameterScript* const parameters =
                G_CDataManager()->get_server_parameter_script();
            const int increment =
                parameters->GetCleanChattingCount(
                    user->get_charac_level());
            const int multiplier =
                user->isDuplicationMessage(std::string(message))
                    ? parameters->GetPaneltyDupMessage()
                    : 1;
            user->incChattingMessageCount(increment * multiplier);
            if (user->getChattingMessageCount() <=
                parameters->GetMaxCleanChattingCount()) {
                send_area_message = true;
            } else {
                user->setChattingMessageCount(0);
                user->reqHumanCertify4ClearMap(false);
            }
        }
        if (send_area_message) {
            make_packet_chat_msg(packet,
                                 user,
                                 message,
                                 message_length,
                                 chat_type,
                                 notification);
            send_to_area(user->getCurCharacVill(),
                         user->get_area(false),
                         packet);
        }
        break;
    }

    case 4:
        make_packet_chat_msg(packet,
                             user,
                             message,
                             message_length,
                             chat_type,
                             notification);
        send_all(packet);
        break;

    case 5:
        if (user) {
            make_packet_chat_msg(packet,
                                 user,
                                 message,
                                 message_length,
                                 chat_type,
                                 notification);
            if (user->GetAssaultPlace()) {
                pvp_assault::GetInstanceAssaultMgr()->SendPacket(
                    user, false, &packet);
            } else {
                PvP_Room* const room = G_CGameManager()->GetPvp(
                    user->GetPvpIndex(), user, 0);
                if (room) {
                    if (!zombie_mode::SendFactionChat(room, user, packet))
                        room->send_to_pvp(packet);
                }
            }
        }
        break;

    case 6:
        if (user->getCurCharacNo() &&
            user->get_charac_guildkey()) {
            Packet_Monitor_Guild_Chat guild;
            guild.guild_key =
                static_cast<uint32_t>(user->get_charac_guildkey());
            guild.charac_no =
                static_cast<uint32_t>(user->getCurCharacNo());
            guild.message_length =
                static_cast<unsigned char>(message_length);
            std::memcpy(guild.message,
                        message,
                        static_cast<size_t>(message_length));
            guild.header.size =
                static_cast<uint16_t>(message_length + 19);
            CGuildServerProxy* const proxy =
                GlobalData::s_guild_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(
                        user->GetServerGroup()));
            proxy->SendPacket(reinterpret_cast<char*>(&guild),
                              static_cast<int>(guild.header.size));
        } else {
            LogManager::logFormat(
                1,
                "world.cpp",
                "void GameWorld::send_chat_msg(CUser*, char, short unsigned int, unsigned int, char*, int, char*, int, ENUM_NOTIPACKET)",
                0xF8C,
                "GUILD_CHAT_ERROR : charac no == %d || guild key == %d ",
                user->getCurCharacNo(),
                user->get_charac_guildkey());
        }
        break;

    case 8:
        if (user->getCurCharacNo() &&
            user->get_charac_memberkey()) {
            Packet_Monitor_Member_Chat member;
            member.member_key =
                static_cast<uint32_t>(user->get_charac_memberkey());
            member.message_length =
                static_cast<unsigned char>(message_length);
            std::memcpy(member.message,
                        message,
                        static_cast<size_t>(message_length));
            member.header.size =
                static_cast<uint16_t>(message_length + 15);
            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(
                        user->GetServerGroup()));
            proxy->SendPacket(reinterpret_cast<char*>(&member),
                              static_cast<int>(member.header.size));
        } else {
            LogManager::logFormat(
                1,
                "world.cpp",
                "void GameWorld::send_chat_msg(CUser*, char, short unsigned int, unsigned int, char*, int, char*, int, ENUM_NOTIPACKET)",
                0xFA3,
                "MEMBER_CHAT_ERROR : member key == %d ",
                user->get_charac_memberkey());
        }
        break;

    case 9:
        if (user) {
            make_packet_chat_msg(packet,
                                 user,
                                 message,
                                 message_length,
                                 chat_type,
                                 notification);
            if (user->GetAssaultPlace()) {
                pvp_assault::GetInstanceAssaultMgr()->SendPacket(
                    user, true, &packet);
            } else {
                PvP_Room* const room = G_CGameManager()->GetPvp(
                    user->GetPvpIndex(), user, 0);
                if (room) {
                    if (!zombie_mode::SendFactionChat(room, user, packet))
                        room->send_to_pvp_team(packet, user);
                }
            }
        }
        break;

    case 11:
    case 12:
    case 14: {
        Packet_Monitor_MegaPhone megaphone;
        megaphone.set(
            static_cast<unsigned char>(G_CEnvironment()->get_channel_no()),
            static_cast<uint16_t>(user->get_unique_id()),
            user->getCurCharacName(),
            static_cast<unsigned char>(message_length),
            message,
            static_cast<unsigned char>(chat_type));
        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(
                    user->GetServerGroup()));
        proxy->SendPacket(reinterpret_cast<char*>(&megaphone),
                          message_length + 46);
        break;
    }

    case 13:
    case 15: {
        PacketGuard megaphone;
        megaphone.put_header(0, 118);
        megaphone.put_byte(chat_type);
        megaphone.put_byte(G_CEnvironment()->get_channel_no());
        megaphone.put_short(user->get_unique_id());
        const int name_length =
            static_cast<int>(std::strlen(user->getCurCharacName()));
        megaphone.put_int(name_length);
        megaphone.put_str(
            const_cast<char*>(user->getCurCharacName()), name_length);
        megaphone.put_int(message_length);
        megaphone.put_str(message, message_length);
        megaphone.finalize(true);
        G_GameWorld()->send_all(megaphone);
        break;
    }

    case 34:
        if (user) {
            make_packet_chat_msg(packet,
                                 user,
                                 message,
                                 message_length,
                                 chat_type,
                                 notification);
            send_to_area(user->getCurCharacVill(),
                         user->get_area(false),
                         packet);
        }
        break;

    case 40:
        if (user) {
            make_packet_chat_msg(packet,
                                 user,
                                 message,
                                 message_length,
                                 chat_type,
                                 notification);
            BlueMarble* const blue_marble = user->getBlueMarble();
            if (blue_marble) {
                blue_marble->sendToBlueMarble(packet);
            }
        }
        break;

    default:
        break;
    }
}
