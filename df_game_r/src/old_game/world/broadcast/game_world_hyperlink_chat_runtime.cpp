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
#include "old_game/social/party/packet_dispatcher_item_hyperlink_message.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>
#include <string>

namespace {

const int kHyperlinkRecordSize = 104;

void put_hyperlink_records(PacketGuard& packet,
                           const MSG_ITEM_HYPER_LINK_MESSAGE& message) {
    packet.put_byte(message.hyperlink_count);
    for (int i = 0; i < message.hyperlink_count; ++i) {
        packet.put_binary(
            reinterpret_cast<const char*>(message.hyperlinks[i]),
            kHyperlinkRecordSize);
    }
}

void make_private_hyperlink_packet(
    PacketGuard& packet,
    const MSG_ITEM_HYPER_LINK_MESSAGE& message,
    unsigned char result,
    const char* sender_name) {
    packet.put_header(0, 371);
    packet.put_byte(message.chat_type);
    packet.put_byte(result);
    const int name_length = static_cast<int>(std::strlen(sender_name));
    packet.put_int(name_length);
    packet.put_str(sender_name, name_length);
    packet.put_byte(0);
    packet.put_int(message.message_length);
    packet.put_str(message.message, message.message_length);
    put_hyperlink_records(packet, message);
    packet.finalize(true);
}

}  // namespace

// IDA_EA: 0x86D0EA4
// IDA_NAME: _ZN9GameWorld31make_packet_chat_msg_hyper_linkER11PacketGuardP5CUserR27MSG_ITEM_HYPER_LINK_MESSAGE15ENUM_NOTIPACKET
// DEMANGLED: GameWorld::make_packet_chat_msg_hyper_link(PacketGuard &,CUser *,MSG_ITEM_HYPER_LINK_MESSAGE &,ENUM_NOTIPACKET)
// Client CMD371 framing for an item hyperlink message.
int GameWorld::make_packet_chat_msg_hyper_link(
    PacketGuard& packet,
    CUser* user,
    MSG_ITEM_HYPER_LINK_MESSAGE& message,
    ENUM_NOTIPACKET notification) {
    packet.clear();
    packet.put_header(0, static_cast<int>(notification));
    packet.put_byte(message.chat_type);
    if (user) {
        packet.put_short(user->get_unique_id());
        packet.put_byte(user->GetServerGroup());
    } else {
        packet.put_short(0);
        packet.put_byte(0);
    }
    packet.put_int(message.message_length);
    packet.put_str(message.message, message.message_length);
    put_hyperlink_records(packet, message);
    return packet.finalize(true);
}

// IDA_EA: 0x86CFA10
// IDA_NAME: _ZN9GameWorld24send_chat_msg_hyper_linkEP5CUserR27MSG_ITEM_HYPER_LINK_MESSAGE15ENUM_NOTIPACKET
// DEMANGLED: GameWorld::send_chat_msg_hyper_link(CUser *,MSG_ITEM_HYPER_LINK_MESSAGE &,ENUM_NOTIPACKET)
void GameWorld::send_chat_msg_hyper_link(
    CUser* user,
    MSG_ITEM_HYPER_LINK_MESSAGE& message,
    ENUM_NOTIPACKET notification) {
    const unsigned int type_value =
        static_cast<unsigned int>(message.chat_type);
    if (type_value <= 0x1Eu &&
        ((1u << type_value) & 0x7FFF0411u) != 0u && user) {
        user->DisConnSig(static_cast<DISCONN_SIG>(24), true, 0);
        return;
    }

    if (message.chat_type != 0) {
        const std::string filtered =
            CSyncSlangFilter::GetInstance()->Filter(
                std::string(message.message));
        if (filtered.empty()) {
            return;
        }
        std::strncpy(message.message, filtered.c_str(), filtered.size());
    }

    PacketGuard packet;
    WriteLogChatMsg(user, static_cast<char>(message.chat_type), message.message);

    switch (message.chat_type) {
    case 1: {
        CUser* const recipient = find_from_world(message.item_space);
        if (!recipient) {
            Packet_Monitor_Other_Channel_Chat_Hyper_Link other_channel;
            other_channel.chat_type =
                static_cast<uint32_t>(message.chat_type);
            other_channel.charac_no =
                static_cast<uint32_t>(user->getCurCharacNo());
            other_channel.item_position = message.item_position;
            other_channel.target_name_length =
                static_cast<unsigned char>(message.target_name_length);
            std::memcpy(other_channel.target_name,
                        message.target_name,
                        static_cast<size_t>(message.target_name_length));
            other_channel.hyperlink_count = message.hyperlink_count;
            for (int i = 0; i < message.hyperlink_count; ++i) {
                std::memcpy(other_channel.hyperlinks[i],
                            message.hyperlinks[i],
                            kHyperlinkRecordSize);
            }
            other_channel.message_length =
                static_cast<unsigned char>(message.message_length);
            std::memcpy(other_channel.message,
                        message.message,
                        static_cast<size_t>(message.message_length));

            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            proxy->SendPacket(
                reinterpret_cast<char*>(&other_channel),
                message.message_length + 372);
            break;
        }

        PacketGuard whisper;
        if (recipient->isBlackUser(user->get_charac_no(-1)) ||
            user->isBlackUser(recipient->get_charac_no(-1))) {
            make_private_hyperlink_packet(whisper,
                                          message,
                                          77,
                                          recipient->getCurCharacName());
            user->Send(whisper);
        } else if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
                   user->GetServerGroup() != recipient->GetServerGroup()) {
            make_private_hyperlink_packet(whisper,
                                          message,
                                          21,
                                          recipient->getCurCharacName());
            user->Send(whisper);
        } else {
            make_private_hyperlink_packet(whisper,
                                          message,
                                          0,
                                          user->getCurCharacName());
            recipient->Send(whisper);
        }
        break;
    }

    case 2:
        if (user) {
            make_packet_chat_msg_hyper_link(packet,
                                             user,
                                             message,
                                             notification);
            CParty* const party = user->GetParty();
            if (party) {
                party->send_to_party(packet);
            }
        }
        break;

    case 3:
    case 39: {
        Village* const village = G_GameWorld()->getUserVillage(user);
        bool send_area_message = false;
        if (village) {
            send_area_message =
                village->get_gate_area() == user->get_area(false) ||
                user->getCurCharacVill() == 8 ||
                static_cast<unsigned int>(notification) == 114u;
            if (!send_area_message) {
                ServerParameterScript* const parameters =
                    G_CDataManager()->get_server_parameter_script();
                const int increment = parameters->GetCleanChattingCount(
                    user->get_charac_level());
                const int multiplier = user->isDuplicationMessage(
                    std::string(message.message))
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
        } else {
            cMyTrace trace(
                "void GameWorld::send_chat_msg_hyper_link(CUser*, MSG_ITEM_HYPER_LINK_MESSAGE&, ENUM_NOTIPACKET)",
                6452,
                5);
            trace("getUserVillage() call failed");
        }

        if (send_area_message && user) {
            make_packet_chat_msg_hyper_link(packet,
                                             user,
                                             message,
                                             notification);
            send_to_area(user->getCurCharacVill(),
                         user->get_area(false),
                         packet);
        }
        break;
    }

    case 4:
        make_packet_chat_msg_hyper_link(packet, user, message, notification);
        send_all(packet);
        break;

    case 5:
        if (user) {
            make_packet_chat_msg_hyper_link(packet,
                                             user,
                                             message,
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
        if (user->getCurCharacNo() && user->get_charac_guildkey()) {
            Packet_Monitor_Guild_Chat_Hyper_Link guild;
            guild.guild_key =
                static_cast<uint32_t>(user->get_charac_guildkey());
            guild.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
            guild.hyperlink_count = message.hyperlink_count;
            for (int i = 0; i < message.hyperlink_count; ++i) {
                std::memcpy(guild.hyperlinks[i],
                            message.hyperlinks[i],
                            kHyperlinkRecordSize);
            }
            guild.message_length =
                static_cast<unsigned char>(message.message_length);
            std::memcpy(guild.message,
                        message.message,
                        static_cast<size_t>(message.message_length));

            CGuildServerProxy* const proxy =
                GlobalData::s_guild_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            proxy->SendPacket(reinterpret_cast<char*>(&guild),
                              message.message_length + 332);
        } else {
            LogManager::logFormat(
                1,
                "world.cpp",
                "void GameWorld::send_chat_msg_hyper_link(CUser*, MSG_ITEM_HYPER_LINK_MESSAGE&, ENUM_NOTIPACKET)",
                0x19B8,
                "GUILD_CHAT_HYPER_LINK_ERROR : charac no == %d || guild key == %d ",
                user->getCurCharacNo(),
                user->get_charac_guildkey());
        }
        break;

    case 8:
        if (user->getCurCharacNo() && user->get_charac_memberkey()) {
            Packet_Monitor_Member_Chat_Hyper_Link member;
            member.member_key =
                static_cast<uint32_t>(user->get_charac_memberkey());
            member.hyperlink_count = message.hyperlink_count;
            for (int i = 0; i < message.hyperlink_count; ++i) {
                std::memcpy(member.hyperlinks[i],
                            message.hyperlinks[i],
                            kHyperlinkRecordSize);
            }
            member.message_length =
                static_cast<unsigned char>(message.message_length);
            std::memcpy(member.message,
                        message.message,
                        static_cast<size_t>(message.message_length));

            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            proxy->SendPacket(reinterpret_cast<char*>(&member),
                              message.message_length + 328);
        } else {
            LogManager::logFormat(
                1,
                "world.cpp",
                "void GameWorld::send_chat_msg_hyper_link(CUser*, MSG_ITEM_HYPER_LINK_MESSAGE&, ENUM_NOTIPACKET)",
                0x19D6,
                "MEMBER_CHAT_HYPER_LINK_ERROR : member key == %d ",
                user->get_charac_memberkey());
        }
        break;

    case 9:
        if (user) {
            make_packet_chat_msg_hyper_link(packet,
                                             user,
                                             message,
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
    case 14:
    case 35:
        if (user->getCurrCharacBlackCount() <= 300) {
            Packet_Monitor_MegaPhone_Hyper_Link megaphone;
            megaphone.set(
                static_cast<unsigned char>(G_CEnvironment()->get_channel_no()),
                static_cast<uint16_t>(user->get_unique_id()),
                user->getCurCharacName(),
                static_cast<unsigned char>(message.message_length),
                message.message,
                message.chat_type,
                message.hyperlink_count,
                message.hyperlinks);
            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            proxy->SendPacket(reinterpret_cast<char*>(&megaphone),
                              message.message_length + 359);
        }
        break;

    case 13:
    case 15: {
        PacketGuard megaphone;
        megaphone.put_header(0, 372);
        megaphone.put_byte(message.chat_type);
        megaphone.put_byte(G_CEnvironment()->get_channel_no());
        megaphone.put_short(user->get_unique_id());
        const int name_length =
            static_cast<int>(std::strlen(user->getCurCharacName()));
        megaphone.put_int(name_length);
        megaphone.put_str(user->getCurCharacName(), name_length);
        megaphone.put_int(message.message_length);
        megaphone.put_str(message.message, message.message_length);
        put_hyperlink_records(megaphone, message);
        megaphone.finalize(true);
        G_GameWorld()->send_all(megaphone);
        break;
    }

    case 34:
        if (user) {
            make_packet_chat_msg_hyper_link(packet,
                                             user,
                                             message,
                                             notification);
            send_to_area(user->getCurCharacVill(),
                         user->get_area(false),
                         packet);
        }
        break;

    case 40:
        if (user) {
            make_packet_chat_msg_hyper_link(packet,
                                             user,
                                             message,
                                             notification);
            BlueMarble* const blue_marble = user->getBlueMarble();
            if (blue_marble) {
                blue_marble->sendToBlueMarble(packet);
            }
        }
        break;

    default:
        // Exact raw default: no broadcast or implicit fallback.
        break;
    }
}
