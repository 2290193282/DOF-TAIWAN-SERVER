#include "old_game/economy/mail/mail_trade_runtime.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

namespace private_store {

// IDA_EA: 0x85D0F90
// IDA_NAME: _ZN13private_store27CCharacterControllerPrivateC2Ev
CCharacterControllerPrivate::CCharacterControllerPrivate()
    : start_position() {}

// IDA_EA: 0x85CC894
// IDA_NAME: _ZN13private_store27CCharacterControllerPrivate13GetStartPointEP5CUserRNS_15PrivateStorePosE
PrivateStorePos* CCharacterControllerPrivate::GetStartPoint(
    CUser* user, PrivateStorePos& position) {
    position.x = 370;
    position.y = 190;
    position.zone.village = 0;
    position.zone.area = 0;

    if (!user) {
        cMyTrace trace(
            "void private_store::CCharacterControllerPrivate::GetStartPoint(CUser*, private_store::PrivateStorePos&)",
            387,
            5);
        trace("pUser == NULL");
        return 0;
    }

    position.zone.village = user->getCurCharacVill();
    GameWorld* const world = G_GameWorld();
    if (!world) {
        cMyTrace trace(
            "void private_store::CCharacterControllerPrivate::GetStartPoint(CUser*, private_store::PrivateStorePos&)",
            395,
            5);
        trace("G_GameWorld call failed");
        return 0;
    }

    Village* const village = world->getUserVillage(user);
    if (!village) {
        cMyTrace trace(
            "void private_store::CCharacterControllerPrivate::GetStartPoint(CUser*, private_store::PrivateStorePos&)",
            400,
            5);
        trace("G_GameWorld call failed");
        return 0;
    }

    position.zone.area = village->get_gate_area();
    return &position;
}

// IDA_EA: 0x85CC9D2
// IDA_NAME: _ZN13private_store27CCharacterControllerPrivate23MoveCharacterStartPointEP5CUser
int CCharacterControllerPrivate::MoveCharacterStartPoint(CUser* user) {
    GetStartPoint(user, start_position);
    return G_GameWorld()->move_area(user,
                                    start_position.zone.village,
                                    start_position.zone.area,
                                    start_position.x,
                                    start_position.y,
                                    0,
                                    false,
                                    0,
                                    0,
                                    0);
}

// IDA data: _ZN13private_store21g_CharacterControllerE @ 0x9472DE0.
CCharacterControllerPrivate g_CharacterController;

// IDA_EA: 0x85CCA73
// IDA_NAME: _ZN13private_store17CBroadcastPrivate4SendEiiR11PacketGuard
void CBroadcastPrivate::Send(int village_index,
                             int area_index,
                             PacketGuard& packet) {
    if (village_index || area_index)
        G_GameWorld()->send_to_area(village_index, area_index, packet);
}

}  // namespace private_store
