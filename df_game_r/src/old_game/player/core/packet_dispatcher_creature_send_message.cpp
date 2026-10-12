#include "old_game/player/core/packet_dispatcher_creature_send_message.h"

#include <stdint.h>
#include <string.h>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/channel/packet_dispatch/channel_packets.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_CreatureSendMessage::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8204710
// IDA_NAME: _ZN30Dispatcher_CreatureSendMessage12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CreatureSendMessage::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CreatureSendMessage::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    char chat_type;
    uint16_t item_space;
    uint32_t item_position;
    int message_length;
    char message[256];
    if (packet.get_byte(chat_type) != 1) {
        return LineFunc(39997, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(item_space) != 1) {
        return LineFunc(39998, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(item_position) != 1) {
        return LineFunc(39999, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(message_length) != 1) {
        return LineFunc(40000, kDispatchSignature, 0, 0);
    }
    if (packet.get_str(message, 256, message_length) != 1) {
        return LineFunc(40001, kDispatchSignature, 0, 0);
    }

    G_CDataManager()->rewriteUsableEmoticon(*user, message);

    if (chat_type == 7 || chat_type == 1) {
        int target_name_length;
        char target_name[30];
        if (packet.get_int(target_name_length) != 1) {
            return LineFunc(40019, kDispatchSignature, 0, 0);
        }
        if (packet.get_str(target_name, 30, target_name_length) != 1) {
            return LineFunc(40020, kDispatchSignature, 0, 0);
        }

        if (chat_type == 7) {
            Packet_Monitor_Other_Channel_Chat monitor_packet;

            // IDA 0x82049BA..0x8204A23 writes precisely these four fields.
            // chat_type, account_id, sender_marker and item_position retain
            // the constructor's original state; this dispatcher does not
            // synthesize values for them.
            monitor_packet.charac_no =
                static_cast<uint32_t>(user->getCurCharacNo());
            monitor_packet.target_name_length =
                static_cast<unsigned char>(target_name_length);
            memcpy(monitor_packet.target_name,
                   target_name,
                   static_cast<size_t>(target_name_length));
            monitor_packet.message_length =
                static_cast<unsigned char>(message_length);
            memcpy(monitor_packet.message,
                   message,
                   static_cast<size_t>(message_length));
            monitor_packet.header.size = static_cast<uint16_t>(
                static_cast<unsigned char>(message_length) + 59u);

            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            proxy->SendPacket(reinterpret_cast<char*>(&monitor_packet),
                              static_cast<int>(monitor_packet.header.size));
            return 0;
        }
    }

    G_GameWorld()->send_chat_msg(
        user,
        chat_type,
        item_space,
        item_position,
        message,
        message_length,
        0,
        0,
        static_cast<ENUM_NOTIPACKET>(114));
    return 0;
}

// IDA_EA: 0x82337FE
// IDA_NAME: _ZN30Dispatcher_CreatureSendMessage4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CreatureSendMessage::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CreatureSendMessage::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8233808
// IDA_NAME: _ZN30Dispatcher_CreatureSendMessage7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CreatureSendMessage::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CreatureSendMessage::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x8242B92
// IDA_NAME: _ZN30Dispatcher_CreatureSendMessageD2Ev
// IDA_EA: 0x8242BC2
// IDA_NAME: _ZN30Dispatcher_CreatureSendMessageD0Ev
// DEMANGLED: Dispatcher_CreatureSendMessage::~Dispatcher_CreatureSendMessage()
Dispatcher_CreatureSendMessage::~Dispatcher_CreatureSendMessage() {}

// IDA_EA: 0x859601C
// IDA_NAME: _ZN30Dispatcher_CreatureSendMessageC2Ev
// DEMANGLED: Dispatcher_CreatureSendMessage::Dispatcher_CreatureSendMessage(void)
Dispatcher_CreatureSendMessage::Dispatcher_CreatureSendMessage()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
