#include "old_game/economy/equipment/packet_dispatcher_upgrade_room.h"

#include <stdint.h>

#include <vector>

#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/economy/equipment/upgrade_room.h"
#include "old_game/player/core/cuser.h"

namespace {

const unsigned int kMaximumMasterMessageBytes = 518u;

int NoRead(PacketBuf&, MSG_BASE&) { return 0; }
int NoProcess(CUser*, MSG_BASE&, ParamBase&) { return 0; }

int RejectMalformed(PacketBuf& packet, int command) {
    cMyTrace trace("UpgradeRoom0828Dispatcher", __LINE__, 5);
    trace("[0828 UpgradeRoom] reject malformed CMD%d cursor(%d)",
          command, packet.get_index());
    // A malformed request has no closed target-client error branch.  It is
    // deliberately dropped instead of manufacturing an ACK or error code.
    return 0;
}

int SendResult(CUser* user, ENUM_CMDPACKET command, int result) {
    if (!user) return 0;
    if (result == 0) {
        PacketGuard response;
        response.put_header(ENUM_PACKETCLASS_CMDPACKET, command);
        response.put_byte(1);
        response.finalize(true);
        user->Send(response);
        return 0;
    }
    if (result > 0 && result <= 0xff) {
        user->SendCmdErrorPacket(command, result);
        return 0;
    }

    cMyTrace trace("UpgradeRoom0828Dispatcher", __LINE__, 5);
    trace("[0828 UpgradeRoom] CMD%d failed without a framed client error (%d)",
          static_cast<int>(command), result);
    return 0;
}

bool IsCanonicalUtf16(const std::vector<uint16_t>& value) {
    for (size_t index = 0u; index < value.size(); ++index) {
        const uint16_t unit = value[index];
        if (unit == 0u) return false;
        if (unit >= 0xd800u && unit <= 0xdbffu) {
            if (++index >= value.size()) return false;
            const uint16_t low = value[index];
            if (low < 0xdc00u || low > 0xdfffu) return false;
        } else if (unit >= 0xdc00u && unit <= 0xdfffu) {
            return false;
        }
    }
    return true;
}

bool ReadMasterMessage(PacketBuf& packet,
                       std::vector<uint16_t>& message) {
    unsigned int byte_count = 0u;
    if (packet.get_int(byte_count) != 1 ||
        (byte_count & 1u) != 0u ||
        byte_count > kMaximumMasterMessageBytes ||
        packet.is_len_end(static_cast<int>(byte_count))) {
        return false;
    }

    std::vector<unsigned char> bytes(byte_count);
    if (byte_count != 0u &&
        packet.get_binary(reinterpret_cast<char*>(&bytes[0]),
                          static_cast<int>(byte_count)) != 1) {
        return false;
    }
    message.reserve(byte_count / 2u);
    for (size_t index = 0u; index < bytes.size(); index += 2u) {
        message.push_back(static_cast<uint16_t>(
            static_cast<uint16_t>(bytes[index]) |
            (static_cast<uint16_t>(bytes[index + 1u]) << 8u)));
    }
    return IsCanonicalUtf16(message);
}

}  // namespace

// 0828 sub_624DD0 writes exactly {u16 x,u16 y}.
int Dispatcher_CreateUpgradeRoom::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    uint16_t x = 0u;
    uint16_t y = 0u;
    if (!user || packet.get_short(x) != 1 || packet.get_short(y) != 1) {
        return RejectMalformed(packet, 570);
    }
    return SendResult(user, ENUM_CMDPACKET_CREATE_UPGRADE_ROOM,
                      upgrade_room::CreateRoom(user, x, y));
}

// 0828 sub_622AF0 writes an empty body.
int Dispatcher_DestroyUpgradeRoom::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    (void)packet;
    if (!user) {
        return RejectMalformed(packet, 571);
    }
    return SendResult(user, ENUM_CMDPACKET_DESTROY_UPGRADE_ROOM,
                      upgrade_room::DestroyRoom(user));
}

// 0828 sub_622B10 writes {u16 room_id,u16 owner_uid}.
int Dispatcher_JoinSpectatorUpgradeRoom::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    uint16_t room_id = 0u;
    uint16_t owner_uid = 0u;
    if (!user || packet.get_short(room_id) != 1 ||
        packet.get_short(owner_uid) != 1) {
        return RejectMalformed(packet, 572);
    }
    return SendResult(user, ENUM_CMDPACKET_JOIN_SPECTATOR_UPGRADE_ROOM,
                      upgrade_room::JoinRoom(user, room_id, owner_uid));
}

// 0828 sub_622EF0 writes an empty body.
int Dispatcher_LeaveSpectatorUpgradeRoom::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    (void)packet;
    if (!user) {
        return RejectMalformed(packet, 573);
    }
    return SendResult(user, ENUM_CMDPACKET_LEAVE_SPECTATOR_UPGRADE_ROOM,
                      upgrade_room::LeaveRoom(user));
}

// 0828 sub_605BC0/sub_623E40 emit only positive entries from the fixed
// manager keys 0 and 3037: {u8 count,repeat{u32 item_id,u32 amount}}.
int Dispatcher_UpgradeRoomGiveItemInfo::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    uint8_t count = 0u;
    if (!user || packet.get_byte(count) != 1 || count == 0u || count > 2u) {
        return RejectMalformed(packet, 574);
    }
    std::vector<upgrade_room::Donation> donations;
    donations.reserve(count);
    for (unsigned int index = 0u; index < count; ++index) {
        unsigned int item_id = 0u;
        unsigned int amount = 0u;
        if (packet.get_int(item_id) != 1 || packet.get_int(amount) != 1) {
            return RejectMalformed(packet, 574);
        }
        donations.push_back(upgrade_room::Donation(item_id, amount));
    }
    return SendResult(user, ENUM_CMDPACKET_UPGRADE_ROOM_GIVE_ITEM_INFO,
                      upgrade_room::Donate(user, donations));
}

// 0828 sub_622B50 writes {u32 item_id,u16 inventory_slot}.
int Dispatcher_UpgradeRoomPutUpItem::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    unsigned int item_id = 0u;
    uint16_t inventory_slot = 0u;
    if (!user || packet.get_int(item_id) != 1 ||
        packet.get_short(inventory_slot) != 1) {
        return RejectMalformed(packet, 582);
    }
    return SendResult(user, ENUM_CMDPACKET_UPGRADE_ROOM_PUT_UP_ITEM,
                      upgrade_room::PutUpItem(
                          user, item_id, inventory_slot));
}

// 0828 sub_622B90 writes an empty body.  All upgrade input is bound to the
// room by prior CMD574/CMD582 state.
int Dispatcher_UpgradeRoomUpgradeStart::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    (void)packet;
    if (!user) {
        return RejectMalformed(packet, 583);
    }
    return SendResult(user, ENUM_CMDPACKET_UPGRADE_ROOM_UPGRADE_START,
                      upgrade_room::StartUpgrade(user));
}

// 0828 sub_625870 writes an empty body.
int Dispatcher_UpgradeRoomUpgradeCancel::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    (void)packet;
    if (!user) {
        return RejectMalformed(packet, 584);
    }
    return SendResult(user, ENUM_CMDPACKET_UPGRADE_ROOM_UPGRADE_CANCEL,
                      upgrade_room::CancelUpgrade(user));
}

// 0828 sub_6231B0 writes {u32 utf16le_bytes,raw UTF-16LE}.
int Dispatcher_UpgradeRoomMasterMessage::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    std::vector<uint16_t> message;
    if (!user || !ReadMasterMessage(packet, message)) {
        return RejectMalformed(packet, 585);
    }
    return SendResult(user, ENUM_CMDPACKET_UPGRADE_ROOM_MASTER_MESSAGE,
                      upgrade_room::SetMasterMessage(user, message));
}

#define DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(Type)                    \
    Type::Type()                                                          \
        : IPacketDispatcher<MSG_BASE, ParamBase,                          \
                            static_cast<ch_state>(0)>() {}                \
    Type::~Type() {}                                                      \
    int Type::read(PacketBuf& packet, MSG_BASE& message) {                \
        return NoRead(packet, message);                                   \
    }                                                                     \
    int Type::process(CUser* user, MSG_BASE& message, ParamBase& param) { \
        return NoProcess(user, message, param);                           \
    }

DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_CreateUpgradeRoom)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_DestroyUpgradeRoom)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_JoinSpectatorUpgradeRoom)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_LeaveSpectatorUpgradeRoom)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_UpgradeRoomGiveItemInfo)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_UpgradeRoomPutUpItem)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_UpgradeRoomUpgradeStart)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_UpgradeRoomUpgradeCancel)
DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE(
    Dispatcher_UpgradeRoomMasterMessage)

#undef DEFINE_UPGRADE_ROOM_DISPATCHER_LIFECYCLE
