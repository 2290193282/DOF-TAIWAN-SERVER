#include "old_game/pvp/battlefield/packet_dispatcher_pvp_buddy.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/pvp/battlefield/inter_pvp_buddy_runtime.h"

#include <stdint.h>
#include <cstring>

#include "common/manager/community_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x8596FDC
// IDA_NAME: _ZN30Dispatcher_ResponseAddPvPBuddyC2Ev
// DEMANGLED: Dispatcher_ResponseAddPvPBuddy::Dispatcher_ResponseAddPvPBuddy(void)
Dispatcher_ResponseAddPvPBuddy::Dispatcher_ResponseAddPvPBuddy() {}

// IDA_EA: 0x8240972
// IDA_NAME: _ZN30Dispatcher_ResponseAddPvPBuddyD2Ev
// DEMANGLED: Dispatcher_ResponseAddPvPBuddy::~Dispatcher_ResponseAddPvPBuddy()
// IDA_EA: 0x82409A2
// IDA_NAME: _ZN30Dispatcher_ResponseAddPvPBuddyD0Ev
// DEMANGLED: Dispatcher_ResponseAddPvPBuddy::~Dispatcher_ResponseAddPvPBuddy()
Dispatcher_ResponseAddPvPBuddy::~Dispatcher_ResponseAddPvPBuddy() {}

// IDA_EA: 0x8224AB4
// IDA_NAME: _ZN30Dispatcher_ResponseAddPvPBuddy12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ResponseAddPvPBuddy::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ResponseAddPvPBuddy::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (!G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return 0;
    }

    uint8_t response_kind = 0;
    int target_name_size = 0;
    char target_name[30] = {};
    if (packet.get_byte(response_kind) != 1) {
        return LineFunc(
            57171,
            "virtual int Dispatcher_ResponseAddPvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_int(target_name_size) != 1) {
        return LineFunc(
            57174,
            "virtual int Dispatcher_ResponseAddPvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_str(target_name, sizeof(target_name), target_name_size) !=
        1) {
        return LineFunc(
            57177,
            "virtual int Dispatcher_ResponseAddPvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    GlobalData::s_community_proxy->SendResAddPvPBuddy(
        static_cast<uint32_t>(user->get_acc_id()),
        static_cast<uint32_t>(user->getCurCharacNo()),
        static_cast<char>(response_kind),
        target_name,
        target_name_size);
    return 0;
}

// IDA_EA: 0x82340E6
// IDA_NAME: _ZN30Dispatcher_ResponseAddPvPBuddy4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ResponseAddPvPBuddy::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ResponseAddPvPBuddy::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82340F0
// IDA_NAME: _ZN30Dispatcher_ResponseAddPvPBuddy7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ResponseAddPvPBuddy::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ResponseAddPvPBuddy::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596FF8
// IDA_NAME: _ZN25Dispatcher_RemovePvPBuddyC2Ev
// DEMANGLED: Dispatcher_RemovePvPBuddy::Dispatcher_RemovePvPBuddy(void)
Dispatcher_RemovePvPBuddy::Dispatcher_RemovePvPBuddy() {}

// IDA_EA: 0x8240924
// IDA_NAME: _ZN25Dispatcher_RemovePvPBuddyD2Ev
// DEMANGLED: Dispatcher_RemovePvPBuddy::~Dispatcher_RemovePvPBuddy()
// IDA_EA: 0x8240954
// IDA_NAME: _ZN25Dispatcher_RemovePvPBuddyD0Ev
// DEMANGLED: Dispatcher_RemovePvPBuddy::~Dispatcher_RemovePvPBuddy()
Dispatcher_RemovePvPBuddy::~Dispatcher_RemovePvPBuddy() {}

// IDA_EA: 0x8224C5E
// IDA_NAME: _ZN25Dispatcher_RemovePvPBuddy12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RemovePvPBuddy::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RemovePvPBuddy::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return 0;
    }

    uint8_t remove_kind = 0;
    int target_name_size = 0;
    char target_name[30] = {};
    if (packet.get_byte(remove_kind) != 1) {
        return LineFunc(
            57194,
            "virtual int Dispatcher_RemovePvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_int(target_name_size) != 1) {
        return LineFunc(
            57197,
            "virtual int Dispatcher_RemovePvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_str(target_name, sizeof(target_name), target_name_size) !=
        1) {
        return LineFunc(
            57200,
            "virtual int Dispatcher_RemovePvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    GlobalData::s_community_proxy->SendRemovePvPBuddy(
        static_cast<uint32_t>(user->get_acc_id()),
        static_cast<uint32_t>(user->getCurCharacNo()),
        static_cast<char>(remove_kind),
        target_name,
        target_name_size);
    return 0;
}

// IDA_EA: 0x82340FA
// IDA_NAME: _ZN25Dispatcher_RemovePvPBuddy4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RemovePvPBuddy::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RemovePvPBuddy::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234104
// IDA_NAME: _ZN25Dispatcher_RemovePvPBuddy7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RemovePvPBuddy::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RemovePvPBuddy::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597014
// IDA_NAME: _ZN27Dispatcher_PvPBuddyConnListC2Ev
// DEMANGLED: Dispatcher_PvPBuddyConnList::Dispatcher_PvPBuddyConnList(void)
Dispatcher_PvPBuddyConnList::Dispatcher_PvPBuddyConnList() {}

// IDA_EA: 0x82408D6
// IDA_NAME: _ZN27Dispatcher_PvPBuddyConnListD2Ev
// DEMANGLED: Dispatcher_PvPBuddyConnList::~Dispatcher_PvPBuddyConnList()
// IDA_EA: 0x8240906
// IDA_NAME: _ZN27Dispatcher_PvPBuddyConnListD0Ev
// DEMANGLED: Dispatcher_PvPBuddyConnList::~Dispatcher_PvPBuddyConnList()
Dispatcher_PvPBuddyConnList::~Dispatcher_PvPBuddyConnList() {}

// IDA_EA: 0x8224E08
// IDA_NAME: _ZN27Dispatcher_PvPBuddyConnList12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_PvPBuddyConnList::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_PvPBuddyConnList::dispatch_sig(CUser* user,
                                               PacketBuf& packet) {
    (void)packet;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        GlobalData::s_community_proxy->SendPvPBuddyList(
            static_cast<uint32_t>(user->get_acc_id()),
            static_cast<uint32_t>(user->getCurCharacNo()));
    }
    return 0;
}

// IDA_EA: 0x823410E
// IDA_NAME: _ZN27Dispatcher_PvPBuddyConnList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PvPBuddyConnList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PvPBuddyConnList::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234118
// IDA_NAME: _ZN27Dispatcher_PvPBuddyConnList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PvPBuddyConnList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PvPBuddyConnList::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

namespace {

// Every offset below is the original database-reply byte offset.  The
// InterDispatcher route has already handed this receiver the raw reply body;
// the old functions perform no length validation here.
int ReadPvPBuddyReplyInt(const char* payload, int offset) {
    int value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84EE688
// IDA_NAME: _ZN18Inter_LoadPvPBuddyC2Ev
Inter_LoadPvPBuddy::Inter_LoadPvPBuddy() : ISigDispatcher() {}

// IDA_EA: 0x84F6FA2 / 0x84F6FD2
Inter_LoadPvPBuddy::~Inter_LoadPvPBuddy() {}

// IDA_EA: 0x84E16F4
// IDA_NAME: _ZN18Inter_LoadPvPBuddy12dispatch_sigEP5CUserPci
// The original leaf consumes nothing and returns zero.  It is deliberately a
// concrete no-op receiver, not a substitute stub for a missing ABI.
int Inter_LoadPvPBuddy::dispatch_sig(CUser* user,
                                     char* payload,
                                     int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;
    return 0;
}

// IDA_EA: 0x84EE6A4
// IDA_NAME: _ZN22Inter_PvPBuddyConnListC2Ev
Inter_PvPBuddyConnList::Inter_PvPBuddyConnList() : ISigDispatcher() {}

// IDA_EA: 0x84F6F54 / 0x84F6F84
Inter_PvPBuddyConnList::~Inter_PvPBuddyConnList() {}

// IDA_EA: 0x84E16FE
// IDA_NAME: _ZN22Inter_PvPBuddyConnList12dispatch_sigEP5CUserPci
int Inter_PvPBuddyConnList::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    (void)payload_size;
    if (user->get_state() > 2 &&
        ReadPvPBuddyReplyInt(payload, 10) == user->getCurCharacNo()) {
        PacketGuard packet;
        packet.put_header(0, 271);
        const int count = ReadPvPBuddyReplyInt(payload, 18);
        packet.put_int(count);
        for (int index = 0; index < count; ++index) {
            packet.put_binary(payload + 22 + 41 * index, 41);
        }
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE6C0
// IDA_NAME: _ZN20Inter_ReqAddPvPBuddyC2Ev
Inter_ReqAddPvPBuddy::Inter_ReqAddPvPBuddy() : ISigDispatcher() {}

// IDA_EA: 0x84F6F06 / 0x84F6F36
Inter_ReqAddPvPBuddy::~Inter_ReqAddPvPBuddy() {}

// IDA_EA: 0x84E1832
// IDA_NAME: _ZN20Inter_ReqAddPvPBuddy12dispatch_sigEP5CUserPci
int Inter_ReqAddPvPBuddy::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)payload_size;
    if (user->get_state() > 2 && user->get_state() != 7 &&
        ReadPvPBuddyReplyInt(payload, 10) == user->getCurCharacNo()) {
        PacketGuard packet;
        packet.put_header(0, 268);
        packet.put_byte(static_cast<unsigned char>(payload[18]));
        const int name_size = static_cast<int>(std::strlen(payload + 19));
        packet.put_int(name_size);
        packet.put_str(payload + 19, name_size);
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE6DC
// IDA_NAME: _ZN23Inter_AddPvPBuddyResultC2Ev
Inter_AddPvPBuddyResult::Inter_AddPvPBuddyResult() : ISigDispatcher() {}

// IDA_EA: 0x84F6EB8 / 0x84F6EE8
Inter_AddPvPBuddyResult::~Inter_AddPvPBuddyResult() {}

// IDA_EA: 0x84E1978
// IDA_NAME: _ZN23Inter_AddPvPBuddyResult12dispatch_sigEP5CUserPci
int Inter_AddPvPBuddyResult::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)payload_size;
    if (user->get_state() <= 2 ||
        ReadPvPBuddyReplyInt(payload, 10) != user->getCurCharacNo()) {
        return 0;
    }

    const ENUM_CMDPACKET command =
        payload[18] ? ENUM_CMDPACKET_RESPONSE_ADD_PVP_BUDDY
                    : ENUM_CMDPACKET_REQUEST_ADD_PVP_BUDDY;
    const unsigned char error = static_cast<unsigned char>(payload[50]);
    if (error) {
        user->SendCmdErrorPacket(command, error);
        return 0;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(1, command);
    packet.put_byte(1);
    packet.put_byte(static_cast<unsigned char>(payload[19]));
    const int name_size = static_cast<int>(std::strlen(payload + 20));
    packet.put_int(name_size);
    packet.put_str(payload + 20, name_size);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE6F8
// IDA_NAME: _ZN26Inter_RemovePvPBuddyResultC2Ev
Inter_RemovePvPBuddyResult::Inter_RemovePvPBuddyResult()
    : ISigDispatcher() {}

// IDA_EA: 0x84F6E6A / 0x84F6E9A
Inter_RemovePvPBuddyResult::~Inter_RemovePvPBuddyResult() {}

// IDA_EA: 0x84E1C1E
// IDA_NAME: _ZN26Inter_RemovePvPBuddyResult12dispatch_sigEP5CUserPci
int Inter_RemovePvPBuddyResult::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)payload_size;
    if (user->get_state() <= 2 ||
        ReadPvPBuddyReplyInt(payload, 10) != user->getCurCharacNo()) {
        return 0;
    }

    const unsigned char error = static_cast<unsigned char>(payload[49]);
    if (error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REMOVE_PVP_BUDDY, error);
        return 0;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_REMOVE_PVP_BUDDY);
    packet.put_byte(1);
    packet.put_byte(static_cast<unsigned char>(payload[18]));
    const int name_size = static_cast<int>(std::strlen(payload + 19));
    packet.put_int(name_size);
    packet.put_str(payload + 19, name_size);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE714
// IDA_NAME: _ZN25Inter_NoticePvPBuddyInOutC2Ev
Inter_NoticePvPBuddyInOut::Inter_NoticePvPBuddyInOut()
    : ISigDispatcher() {}

// IDA_EA: 0x84F6E1C / 0x84F6E4C
Inter_NoticePvPBuddyInOut::~Inter_NoticePvPBuddyInOut() {}

// IDA_EA: 0x84E1D96
// IDA_NAME: _ZN25Inter_NoticePvPBuddyInOut12dispatch_sigEP5CUserPci
int Inter_NoticePvPBuddyInOut::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)payload_size;
    if (user->get_state() > 2 &&
        ReadPvPBuddyReplyInt(payload, 10) == user->getCurCharacNo()) {
        PacketGuard packet;
        if (payload[18]) {
            packet.put_header(0, 270);
        } else {
            packet.put_header(0, 269);
            packet.put_byte(static_cast<unsigned char>(payload[19]));
        }
        packet.put_byte(static_cast<unsigned char>(payload[20]));
        const int name_size = static_cast<int>(std::strlen(payload + 21));
        packet.put_int(name_size);
        packet.put_str(payload + 21, name_size);
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}
