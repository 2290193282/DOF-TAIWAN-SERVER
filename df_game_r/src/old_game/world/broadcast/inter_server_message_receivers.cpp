#include "old_data/db/taiwan/inter_slots_357_382.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/shared_server_message_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"

namespace {

// IDA unit: 99_misc_global/Packet_Monitor_Server_Message_Info.  The local
// receiver constructs this exact TCP body before forwarding it to monitor.
#pragma pack(push, 1)
struct Packet_Monitor_Server_Message_Info {
    PacketHeader header;         // +0x00
    uint8_t channel_no;          // +0x0a
    uint8_t share_type;          // +0x0b
    uint8_t message_index;       // +0x0c
    uint32_t message_len;        // +0x0d
    char message[50];            // +0x11
    uint32_t charac_name_len;    // +0x43
    char charac_name[20];        // +0x47
    uint32_t update_time;        // +0x5b

    Packet_Monitor_Server_Message_Info()
        : header(0xfaeu, 0x5fu),
          channel_no(0),
          share_type(0),
          message_index(0),
          message_len(0),
          charac_name_len(0),
          update_time(0) {
        std::memset(message, 0, sizeof(message));
        std::memset(charac_name, 0, sizeof(charac_name));
    }
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_Server_Message_Info) == 0x5fu,
              "Monitor server-message packet ABI changed");
static_assert(offsetof(Packet_Monitor_Server_Message_Info, channel_no) ==
                  0x0au &&
                  offsetof(Packet_Monitor_Server_Message_Info, message_len) ==
                      0x0du &&
                  offsetof(Packet_Monitor_Server_Message_Info, message) ==
                      0x11u &&
                  offsetof(Packet_Monitor_Server_Message_Info, charac_name_len) ==
                      0x43u &&
                  offsetof(Packet_Monitor_Server_Message_Info, charac_name) ==
                      0x47u &&
                  offsetof(Packet_Monitor_Server_Message_Info, update_time) ==
                      0x5bu,
              "Monitor server-message packet fields moved");

}  // namespace

// IDA_EA: 0x84DA3E6
// IDA_NAME: _ZN25Inter_UpdateServerMessage12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UpdateServerMessage::dispatch_sig(CUser *,char *,int)
int Inter_UpdateServerMessage::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)payload_size;
    const SIG_UPDATE_SERVER_MESSAGE& input =
        *reinterpret_cast<const SIG_UPDATE_SERVER_MESSAGE*>(payload);
    PacketGuard broadcast;

    if (input.kind) {
        if (input.kind == 1) {
            Packet_Monitor_Server_Message_Info monitor_packet;
            monitor_packet.channel_no =
                static_cast<uint8_t>(G_CEnvironment()->get_channel_no());
            monitor_packet.share_type = input.kind;
            monitor_packet.message_index = input.message_index;
            monitor_packet.message_len = input.message_len;
            monitor_packet.charac_name_len = input.charac_name_len;
            monitor_packet.update_time = input.update_time;
            std::strncpy(monitor_packet.message, input.message,
                         sizeof(monitor_packet.message));
            std::strncpy(monitor_packet.charac_name, input.charac_name,
                         sizeof(monitor_packet.charac_name));

            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
                ->SendTcpPacket(reinterpret_cast<char*>(&monitor_packet),
                                sizeof(monitor_packet));

            cMyTrace trace(
                "virtual int Inter_UpdateServerMessage::dispatch_sig(CUser*, char*, int)",
                17056, 0);
            trace("SERVER_MESSAGE : Send ServerMessage to monitor server(source_channel:%d, kind:%d, index:%d, msg_len:%d, msg:%s, name_len:%d, name:%s, time:%d",
                  static_cast<int>(monitor_packet.channel_no),
                  static_cast<int>(input.kind),
                  static_cast<int>(input.message_index),
                  static_cast<int>(input.message_len), input.message,
                  static_cast<int>(input.charac_name_len), input.charac_name,
                  static_cast<int>(input.update_time));
        } else {
            cMyTrace trace(
                "virtual int Inter_UpdateServerMessage::dispatch_sig(CUser*, char*, int)",
                17062, 0);
            trace("SERVER_MESSAGE : NOT EXIST SHARE TYPE");
        }
    } else {
        broadcast.put_header(0, 192);
        broadcast.put_byte(1);
        broadcast.put_byte(1);
        broadcast.put_byte(static_cast<char>(input.message_index));
        broadcast.put_int(static_cast<int>(input.message_len));
        broadcast.put_str(input.message, static_cast<int>(input.message_len));
        broadcast.put_int(static_cast<int>(input.charac_name_len));
        broadcast.put_str(input.charac_name,
                          static_cast<int>(input.charac_name_len));
        broadcast.finalize(true);
        G_GameWorld()->send_all_with_state(broadcast, 3);
    }
    return 0;
}

// IDA_EA: 0x84EE0D8 / 0x84F8016 / 0x84F8046
// IDA_NAME: _ZN25Inter_UpdateServerMessageC2Ev /
//           _ZN25Inter_UpdateServerMessageD2Ev /
//           _ZN25Inter_UpdateServerMessageD0Ev
Inter_UpdateServerMessage::Inter_UpdateServerMessage() {}
Inter_UpdateServerMessage::~Inter_UpdateServerMessage() {}

// IDA_EA: 0x84DA726
// IDA_NAME: _ZN29Inter_NoticeServerMessageInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticeServerMessageInfo::dispatch_sig(CUser *,char *,int)
int Inter_NoticeServerMessageInfo::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    (void)user;
    (void)payload_size;
    const SIG_NOTICE_SERVER_MESSAGE_INFO& input =
        *reinterpret_cast<const SIG_NOTICE_SERVER_MESSAGE_INFO*>(payload);

    if (input.source_channel !=
        static_cast<uint8_t>(G_CEnvironment()->get_channel_no())) {
        G_CGameManager()->GetSharedServerMessageManager()
            ->UpdateServerMessageByBroadcast(
                static_cast<int>(input.kind), const_cast<char*>(input.message),
                const_cast<char*>(input.charac_name),
                static_cast<int>(input.update_time));
    }

    cMyTrace trace(
        "virtual int Inter_NoticeServerMessageInfo::dispatch_sig(CUser*, char*, int)",
        17086, 0);
    trace("SERVER_MESSAGE : Recv ServerMessage from monitor server(source_channel:%d, kind:%d, index:%d, msg_len:%d, msg:%s, name_len:%d, name:%s, time:%d",
          static_cast<int>(input.source_channel),
          static_cast<int>(input.kind), 0,
          static_cast<int>(input.message_len), input.message,
          static_cast<int>(input.charac_name_len), input.charac_name,
          static_cast<int>(input.update_time));

    PacketGuard broadcast;
    broadcast.put_header(0, 192);
    broadcast.put_byte(1);
    broadcast.put_byte(1);
    broadcast.put_byte(static_cast<char>(input.kind));
    broadcast.put_int(static_cast<int>(input.message_len));
    broadcast.put_str(input.message, static_cast<int>(input.message_len));
    broadcast.put_int(static_cast<int>(input.charac_name_len));
    broadcast.put_str(input.charac_name,
                      static_cast<int>(input.charac_name_len));
    broadcast.finalize(true);
    G_GameWorld()->send_all_with_state(broadcast, 3);
    return 0;
}

// IDA_EA: 0x84EE110 / 0x84F7FC8 / 0x84F7FF8
// IDA_NAME: _ZN29Inter_NoticeServerMessageInfoC2Ev /
//           _ZN29Inter_NoticeServerMessageInfoD2Ev /
//           _ZN29Inter_NoticeServerMessageInfoD0Ev
Inter_NoticeServerMessageInfo::Inter_NoticeServerMessageInfo() {}
Inter_NoticeServerMessageInfo::~Inter_NoticeServerMessageInfo() {}
