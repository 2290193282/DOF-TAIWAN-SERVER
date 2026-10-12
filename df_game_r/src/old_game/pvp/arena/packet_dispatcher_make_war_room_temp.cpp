#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kMakeWarRoomTempSignature[] =
    "virtual int DisPatcher_MakeWarRoomTemp::dispatch_sig(CUser*, PacketBuf&)";
char kEmptyWarRoomName[] = "";

}  // namespace

// IDA_EA: 0x82431AA / 0x82431DA
// IDA_NAME: _ZN26DisPatcher_MakeWarRoomTempD2Ev / D0Ev
// DEMANGLED: DisPatcher_MakeWarRoomTemp::~DisPatcher_MakeWarRoomTemp()
DisPatcher_MakeWarRoomTemp::~DisPatcher_MakeWarRoomTemp() {}

// IDA_EA: 0x8201B32
// IDA_NAME: _ZN26DisPatcher_MakeWarRoomTemp12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_MakeWarRoomTemp::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_MakeWarRoomTemp::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;

    if (G_GameWorld()->IsPVPChannel() || user->get_state() != 3) {
        return 0;
    }

    PacketGuard response;
    WarRoom* const room = G_CGameManager()->GetWarRoom();
    response.put_header(1, ENUM_CMDPACKET_MAKE_WARROOM_TEMP);
    if (!room) {
        response.put_byte(0);
        response.put_byte(4);
        response.finalize(true);
        user->Send(response);
        return 0;
    }

    if (user->CheckInParty()) {
        CParty* const party = user->GetParty();
        if (party) {
            party->send_invite_warroom(user->get_unique_id());
            G_CGameManager()->CheckOutParty(user, false);
        }
    }

    const int create_result = room->Create(user, kEmptyWarRoomName, 6000);
    if (create_result > 0) {
        response.put_byte(0);
        response.put_byte(create_result);
        response.finalize(true);
        user->Send(response);
        LogManager::logFormat(1,
                              "PacketDispatcher_Impl_1.cpp",
                              kMakeWarRoomTempSignature,
                              0x976D,
                              "create_room fail: ",
                              room->GetIndex());
        G_CGameManager()->PutWarRoom(room);
        return 0;
    }

    response.clear();
    response.put_header(0, 2);
    response.put_byte(1);
    response.put_short(1);
    user->make_basic_info(response, 1);
    response.finalize(true);
    user->Send(response);

    response.clear();
    response.put_header(0, 87);
    response.put_short(1);
    room->MakeRoomInfo(reinterpret_cast<char*>(&response));
    response.finalize(true);
    G_GameWorld()->send_all(response);
    G_GameWorld()->goto_warroom(user);
    return 0;
}

// IDA_EA: 0x82335F8
// IDA_NAME: _ZN26DisPatcher_MakeWarRoomTemp4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_MakeWarRoomTemp::read(PacketBuf &,MSG_BASE &)
int DisPatcher_MakeWarRoomTemp::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233602
// IDA_NAME: _ZN26DisPatcher_MakeWarRoomTemp7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_MakeWarRoomTemp::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_MakeWarRoomTemp::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
