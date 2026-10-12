#include "old_game/world/region/game_world.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

namespace {

bool is_current_area_valid(GameWorld& world, CUser* user) {
    return user && world.check_valid_area(
                       static_cast<signed char>(user->getCurCharacVill()),
                       user->get_area(false));
}

Area* find_current_area(GameWorld& world, CUser* user) {
    Village* const village = world.getUserVillage(user);
    return village ? village->getArea(user->get_area(false)) : 0;
}

void send_enter_instance_packet(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 23);
    packet.put_short(user->get_unique_id());
    packet.put_byte(user->getCurCharacVill());
    packet.put_byte(254);
    packet.put_short(user->get_posX());
    packet.put_short(user->get_posY());
    packet.put_byte(user->get_direction());
    packet.put_byte(user->get_charac_visible_values());
    packet.finalize(true);
    user->Send(packet);
}

void send_world_state_packet(GameWorld& world, CUser* user, int state) {
    PacketGuard packet;
    packet.put_header(0, 3);
    packet.put_byte(1);
    packet.put_short(user->get_unique_id());
    packet.put_byte(state);
    packet.finalize(true);
    world.send_all(packet, user);
}

void disconnect_invalid_return(CUser* user, const char* member_name, int line) {
    cMyTrace trace(member_name, line, 0);
    trace("user %s - check_valid_area() fail", user->getCurCharacName());
    user->DisConnSig(static_cast<DISCONN_SIG>(3), true, 0);
}

}  // namespace

// IDA_EA: 0x86C5706
// IDA_NAME: _ZN9GameWorld13move_positionEP5CUseriiit
// DEMANGLED: GameWorld::move_position(CUser *,int,int,int,unsigned short)
void GameWorld::move_position(CUser* user,
                              int pos_x,
                              int pos_y,
                              int direction,
                              uint16_t position_context) {
    if (!user || static_cast<unsigned char>(user->IsCurCharacGhost())) {
        LogManager::logFormat(
            1, "world_position_diag.cpp",
            "void GameWorld::move_position(CUser*, int, int, int, short unsigned int)",
            0, "[DIAG0627][MOVE_POSITION_DROP] reason=null_or_ghost");
        return;
    }

    user->set_position(static_cast<uint16_t>(pos_x),
                       static_cast<uint16_t>(pos_y),
                       static_cast<char>(direction));

    PacketGuard packet;
    packet.put_header(0, 22);
    packet.put_short(user->get_unique_id());
    packet.put_short(pos_x);
    packet.put_short(pos_y);
    packet.put_byte(direction);
    packet.put_short(position_context);
    packet.finalize(true);

    const int village_index = static_cast<signed char>(user->getCurCharacVill());
    const int area_index = user->get_area(false);
    if (!check_valid_area(village_index, area_index)) {
        LogManager::logFormat(
            1, "world_position_diag.cpp",
            "void GameWorld::move_position(CUser*, int, int, int, short unsigned int)",
            0,
            "[DIAG0627][MOVE_POSITION_DROP] reason=invalid_area village=%d area=%d",
            village_index,
            area_index);
        return;
    }

    Village* const village = getUserVillage(user);
    if (!village) {
        LogManager::logFormat(
            1, "world_position_diag.cpp",
            "void GameWorld::move_position(CUser*, int, int, int, short unsigned int)",
            0,
            "[DIAG0627][MOVE_POSITION_DROP] reason=no_village village=%d area=%d",
            village_index,
            area_index);
        cMyTrace trace(
            "void GameWorld::move_position(CUser*, int, int, int, short unsigned int)",
            1516,
            5);
        trace("%d %d", village_index, area_index);
        return;
    }

    Area* const area = village->getArea(area_index);
    if (!area) {
        LogManager::logFormat(
            1, "world_position_diag.cpp",
            "void GameWorld::move_position(CUser*, int, int, int, short unsigned int)",
            0,
            "[DIAG0627][MOVE_POSITION_DROP] reason=no_area village=%d area=%d",
            village_index,
            area_index);
        return;
    }

    if (static_cast<unsigned char>(user->IsCurCharacVisible())) {
        area->send_to_all(packet);
    } else {
        user->Send(packet);
    }
}

// IDA_EA: 0x86C6CEE
// IDA_NAME: _ZN9GameWorld8goto_pvpEP5CUser
// DEMANGLED: GameWorld::goto_pvp(CUser *)
void GameWorld::goto_pvp(CUser* user) {
    if (!is_current_area_valid(*this, user)) {
        cMyTrace trace("void GameWorld::goto_pvp(CUser*)", 2456, 5);
        trace("user %s - check_valid_area() fail", user ? user->getCurCharacName() : "");
        return;
    }

    Area* const area = find_current_area(*this, user);
    if (area) {
        area->erase_user(user);
    }
    send_enter_instance_packet(user);
    user->set_state(6);
    send_world_state_packet(*this, user, 2);
}

// IDA_EA: 0x86C6FAE
// IDA_NAME: _ZN9GameWorld12out_from_pvpEP5CUser
// DEMANGLED: GameWorld::out_from_pvp(CUser *)
void GameWorld::out_from_pvp(CUser* user) {
    if (!is_current_area_valid(*this, user)) {
        if (user) {
            disconnect_invalid_return(user,
                                      "void GameWorld::out_from_pvp(CUser*)",
                                      2510);
        }
        return;
    }

    user->SetPvpIndex(-1);
    Area* const area = find_current_area(*this, user);
    if (area) {
        area->insert_user(user);
    }
    send_world_state_packet(*this, user, 0);
}

// IDA_EA: 0x86C7192
// IDA_NAME: _ZN9GameWorld12goto_warroomEP5CUser
// DEMANGLED: GameWorld::goto_warroom(CUser *)
void GameWorld::goto_warroom(CUser* user) {
    if (!is_current_area_valid(*this, user)) {
        LogManager::logFormat(1,
                              "world.cpp",
                              "void GameWorld::goto_warroom(CUser*)",
                              2547,
                              "user %s - check_valid_area() fail",
                              user->getCurCharacName());
        return;
    }

    Area* const area = find_current_area(*this, user);
    if (area) {
        area->erase_user(user);
    }
    send_enter_instance_packet(user);
    user->set_state(8);
    send_world_state_packet(*this, user, 3);
}

// IDA_EA: 0x86C7654
// IDA_NAME: _ZN9GameWorld14gotoBlueMarbleEP5CUser
// DEMANGLED: GameWorld::gotoBlueMarble(CUser *)
void GameWorld::gotoBlueMarble(CUser* user) {
    if (!is_current_area_valid(*this, user)) {
        cMyTrace trace("void GameWorld::gotoBlueMarble(CUser*)", 2644, 5);
        trace("user %s - check_valid_area() fail", user ? user->getCurCharacName() : "");
        return;
    }

    Area* const area = find_current_area(*this, user);
    if (area) {
        area->erase_user(user);
    }
    send_enter_instance_packet(user);
    user->set_state(11);
    send_world_state_packet(*this, user, 7);
}

// IDA_EA: 0x86C7914
// IDA_NAME: _ZN9GameWorld17outFromBlueMarbleEP5CUser
// DEMANGLED: GameWorld::outFromBlueMarble(CUser *)
void GameWorld::outFromBlueMarble(CUser* user) {
    if (!is_current_area_valid(*this, user)) {
        if (user) {
            disconnect_invalid_return(user,
                                      "void GameWorld::outFromBlueMarble(CUser*)",
                                      2687);
        }
        return;
    }

    user->setBlueMarbleIndex(-1);
    Area* const area = find_current_area(*this, user);
    if (area) {
        area->insert_user(user);
    }
    send_world_state_packet(*this, user, 0);
}
