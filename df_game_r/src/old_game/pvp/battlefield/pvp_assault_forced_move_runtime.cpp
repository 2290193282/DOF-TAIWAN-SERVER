#include "old_game/pvp/battlefield/pvp_assault.h"

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"

namespace pvp_assault {

static_assert(sizeof(CForcedMoveCharacter) == 0x20,
              "CForcedMoveCharacter must retain the original i386 extent");
static_assert(offsetof(CForcedMoveCharacter, user) == 0x08,
              "CForcedMoveCharacter::user offset");
static_assert(offsetof(CForcedMoveCharacter, village) == 0x0c,
              "CForcedMoveCharacter::village offset");
static_assert(offsetof(CForcedMoveCharacter, direction) == 0x1c,
              "CForcedMoveCharacter::direction offset");

// IDA_EA: 0x82E567C
// IDA_NAME: _ZN11pvp_assault20CForcedMoveCharacter10ForcedMoveEv
// DEMANGLED: pvp_assault::CForcedMoveCharacter::ForcedMove(void)
void CForcedMoveCharacter::ForcedMove() {
    if (!user) {
        return;
    }

    if (static_cast<char>(user->getCurCharacVill()) != village ||
        user->get_area(false) != area) {
        G_GameWorld()->move_area(user,
                                 village,
                                 area,
                                 pos_x,
                                 pos_y,
                                 direction,
                                 false,
                                 0,
                                 0,
                                 0);
    } else {
        user->set_position(static_cast<uint16_t>(pos_x),
                           static_cast<uint16_t>(pos_y),
                           static_cast<char>(direction));
    }

    PacketGuard packet;
    packet.put_header(0, 138);
    packet.put_short(user->get_unique_id());
    packet.put_short(pos_x);
    packet.put_short(pos_y);
    packet.put_byte(direction);
    packet.finalize(true);

    if (village || area) {
        G_GameWorld()->send_to_area(village, area, packet);
    }
}

// IDA_EA: 0x82ED6A6
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace25ForcedMoveAtPowerWarPlaceEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::ForcedMoveAtPowerWarPlace(CUser *)
void CAssaultPlace::ForcedMoveAtPowerWarPlace(CUser* user) {
    if (!user || !CPowerManager::IsPowerWarEventOn() ||
        static_cast<unsigned char>(user->getCurCharacVill()) != 7u) {
        return;
    }

    CForcedMoveCharacter character = {};
    character.user = user;
    character.village = 7;
    character.direction = 0;

    switch (user->getPowerSide()) {
    case 1:
        character.area = 0;
        character.pos_x = 597;
        character.pos_y = 300;
        break;
    case 2:
        character.area = 4;
        character.pos_x = 313;
        character.pos_y = 314;
        break;
    default:
        return;
    }

    character.ForcedMove();
}

// IDA_EA: 0x82ED672
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr25ForcedMoveAtPowerWarPlaceEP5CUser
// DEMANGLED: pvp_assault::CAssaultMgr::ForcedMoveAtPowerWarPlace(CUser *)
int CAssaultMgr::ForcedMoveAtPowerWarPlace(CUser* user) {
    CAssaultPlace::ForcedMoveAtPowerWarPlace(user);
    return 0;
}

}  // namespace pvp_assault
