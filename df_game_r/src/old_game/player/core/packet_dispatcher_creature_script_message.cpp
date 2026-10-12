#include "old_game/player/core/packet_dispatcher_creature_script_message.h"

#include <stdint.h>
#include <string.h>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/channel/packet_dispatch/channel_packets.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_CreatureScriptMessage::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8204D8E
// IDA_NAME: _ZN32Dispatcher_CreatureScriptMessage12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CreatureScriptMessage::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CreatureScriptMessage::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacVill() ||
        !user->getCurCharacR()) {
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory || !inventory->GetCreatureMgrR()->IsEquippedCreature()) {
        return 0;
    }

    char chat_type;
    uint16_t item_space;
    uint32_t item_position;
    int message_length;
    char message[256];
    if (packet.get_byte(chat_type) != 1) {
        return LineFunc(40179, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(item_space) != 1) {
        return LineFunc(40180, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(item_position) != 1) {
        return LineFunc(40181, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(message_length) != 1) {
        return LineFunc(40182, kDispatchSignature, 0, 0);
    }
    if (packet.get_str(message, 256, message_length) != 1) {
        return LineFunc(40183, kDispatchSignature, 0, 0);
    }

    if (chat_type == 7 || chat_type == 1) {
        int target_name_length;
        char target_name[30];
        if (packet.get_int(target_name_length) != 1) {
            return LineFunc(40187, kDispatchSignature, 0, 0);
        }
        if (packet.get_str(target_name, 30, target_name_length) != 1) {
            return LineFunc(40188, kDispatchSignature, 0, 0);
        }

        if (chat_type == 7) {
            Packet_Monitor_Other_Channel_Chat monitor_packet;

            // IDA 0x820506f..0x820510b sets exactly these payload members.
            // The packet C2 owns header/chat/account/sender/item fields; this
            // concrete dispatcher only supplies the character and chat data.
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
        static_cast<ENUM_NOTIPACKET>(119));
    return 0;
}

// IDA_EA: 0x823383A
// IDA_NAME: _ZN32Dispatcher_CreatureScriptMessage4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CreatureScriptMessage::read(PacketBuf &,MSG_BASE &)
// The original is the genuine ten-byte `return 0` virtual body.
int Dispatcher_CreatureScriptMessage::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8233844
// IDA_NAME: _ZN32Dispatcher_CreatureScriptMessage7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CreatureScriptMessage::process(CUser *,MSG_BASE &,ParamBase &)
// The original is the genuine ten-byte `return 0` virtual body.
int Dispatcher_CreatureScriptMessage::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x8242AA8
// IDA_NAME: _ZN32Dispatcher_CreatureScriptMessageD2Ev
// IDA_EA: 0x8242AD8
// IDA_NAME: _ZN32Dispatcher_CreatureScriptMessageD0Ev
// DEMANGLED: Dispatcher_CreatureScriptMessage::~Dispatcher_CreatureScriptMessage()
Dispatcher_CreatureScriptMessage::~Dispatcher_CreatureScriptMessage() {}

// IDA_EA: 0x85960FC
// IDA_NAME: _ZN32Dispatcher_CreatureScriptMessageC2Ev
// DEMANGLED: Dispatcher_CreatureScriptMessage::Dispatcher_CreatureScriptMessage(void)
Dispatcher_CreatureScriptMessage::Dispatcher_CreatureScriptMessage()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
