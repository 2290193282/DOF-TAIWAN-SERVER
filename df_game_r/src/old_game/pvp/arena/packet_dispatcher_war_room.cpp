#include "old_game/pvp/arena/packet_dispatcher_war_room.h"

#include <stdint.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/world/region/game_world.h"

namespace {

const char kEnterSignature[] =
    "virtual int Dispatcher_EnterWarRoom0828::dispatch_sig(CUser*, PacketBuf&)";
const char kSeatSignature[] =
    "virtual int Dispatcher_SetWarRoomSeatState0828::dispatch_sig(CUser*, PacketBuf&)";
const char kDieSignature[] =
    "virtual int Dispatcher_DieWarRoomCharacter0828::dispatch_sig(CUser*, PacketBuf&)";

int NoRead(PacketBuf&, MSG_BASE&) { return 0; }
int NoProcess(CUser*, MSG_BASE&, ParamBase&) { return 0; }

}  // namespace

// Client sub_118AB80 begins CMD90 at 0x118AC19, writes the selected WarRoom
// index as one u16 at 0x118AC2D, and finalizes at 0x118AC32.  WarRoom::Join
// owns the exact success tail {u32 current_money,u16 elapsed_seconds}.
int Dispatcher_EnterWarRoom::dispatch_sig(CUser* user,
                                               PacketBuf& packet) {
    uint16_t room_index = 0;
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        G_GameWorld()->IsPVPChannel() ||
        packet.get_short(room_index) != 1) {
        return LineFunc(14082890, kEnterSignature, 0, 0);
    }

    // A session already attached to a WarRoom must never be admitted through
    // a second room index, even if a forged client races the world transition.
    if (user->CheckInWarRoom() || user->GetWarRoom()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ENTER_WARROOM, 19);
        return 0;
    }

    WarRoom* const room = G_CGameManager()->GetWarRoom(room_index);
    if (!room) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ENTER_WARROOM, 19);
        return 0;
    }

    int slot = -1;
    int result = room->IsJoinable(user);
    if (result <= 0) {
        result = room->Join(user, slot);
    }
    if (result > 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ENTER_WARROOM, result);
        return 0;
    }

    G_CGameManager()->CheckOutParty(user, false);

    PacketGuard slot_packet;
    room->MakeSlotInfo(reinterpret_cast<char*>(&slot_packet), slot);
    G_GameWorld()->send_all(slot_packet);
    G_GameWorld()->goto_warroom(user);
    if (room->GetWaiterCount() > 2 && room->GetState() == 0) {
        room->Start();
    }
    return 0;
}

int Dispatcher_SetWarRoomSeatState::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    uint8_t seat = 0;
    uint8_t state = 0;
    if (!user || user->get_state() != 8 ||
        packet.get_byte(seat) != 1 || packet.get_byte(state) != 1) {
        return LineFunc(14082891, kSeatSignature, 0, 0);
    }

    // Native callers use seats 0..5 and states 0/1/2 for the occupant's
    // visible WarRoom role, plus 255 for leaving its own seat.  Values 3..254
    // are not produced by the 140828 client and remain fail-closed.
    if (seat >= 6 || (state > 2 && state != 255)) {
        return 0;
    }

    WarRoom* const room = user->GetWarRoom();
    if (!room || room->GetUserSlot(user) != static_cast<int>(seat)) {
        return 0;
    }

    room->HandleSetSeatState(
        user, static_cast<int>(seat), static_cast<ENUM_SEAT_STATE>(state));
    return 0;
}

// Client sub_A4D3C0 writes the authoritative scene killer seat (0..5), or
// 255 when no WarRoom killer exists.  WarRoom::HandleDieCharacer revalidates
// the victim's live flag and resolves the killer from the room table.
int Dispatcher_DieWarRoomCharacter::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    uint8_t killer_seat = 0;
    if (!user || user->get_state() != 8 ||
        packet.get_byte(killer_seat) != 1) {
        return LineFunc(14082892, kDieSignature, 0, 0);
    }
    if (killer_seat >= 6 && killer_seat != 255) {
        return 0;
    }

    WarRoom* const room = user->GetWarRoom();
    if (!room || room->GetUserSlot(user) < 0) {
        return 0;
    }
    room->HandleDieCharacer(user, static_cast<int>(killer_seat));
    return 0;
}

#define DEFINE_WARROOM_DISPATCHER_LIFECYCLE(Type)                           \
    Type::Type()                                                            \
        : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {} \
    Type::~Type() {}                                                        \
    int Type::read(PacketBuf& packet, MSG_BASE& message) {                  \
        return NoRead(packet, message);                                     \
    }                                                                       \
    int Type::process(CUser* user, MSG_BASE& message, ParamBase& param) {   \
        return NoProcess(user, message, param);                             \
    }

DEFINE_WARROOM_DISPATCHER_LIFECYCLE(Dispatcher_EnterWarRoom)
DEFINE_WARROOM_DISPATCHER_LIFECYCLE(Dispatcher_SetWarRoomSeatState)
DEFINE_WARROOM_DISPATCHER_LIFECYCLE(Dispatcher_DieWarRoomCharacter)

#undef DEFINE_WARROOM_DISPATCHER_LIFECYCLE
