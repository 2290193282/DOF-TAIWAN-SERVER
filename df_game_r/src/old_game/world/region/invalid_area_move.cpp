#include "old_game/world/region/invalid_area_move.h"

#include "common/manager/global_tools.h"
#include "old_game/world/region/game_world.h"

#include <stdint.h>


// IDA_EA: 0x8285E14
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove14checkSeriaRoomEii
// DEMANGLED: CHackLog_InvalidAreaMove::checkSeriaRoom(int,int)
int CHackLog_InvalidAreaMove::checkSeriaRoom(int area, int unused) {
    return IsSeriaRoom(area, unused);
}

// IDA_EA: 0x8285E2E
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove20IsInvalidMovableAreaEiiii
// DEMANGLED: CHackLog_InvalidAreaMove::IsInvalidMovableArea(int,int,int,int)
bool CHackLog_InvalidAreaMove::IsInvalidMovableArea(int before_area,
                                                    int target_village,
                                                    int target_area,
                                                    int unused) {
    return !IsMovingBetweenWestCoastAndGent(before_area, target_village,
                                            target_area, unused) &&
           !IsMovingAboutPowerWar(target_village, unused) &&
           !IsMovingInGuildAgit(target_village, unused) &&
           !IsExceptionMove(before_area, target_village, target_area, unused) &&
           !IsMovingBetweenSeriaRoomAndGuildAgit(before_area, target_village,
                                                 target_area, unused);
}

// IDA_EA: 0x8285EFE
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove14isMovableRouteEiiiiii
// DEMANGLED: CHackLog_InvalidAreaMove::isMovableRoute(int,int,int,int,int,int)
int CHackLog_InvalidAreaMove::isMovableRoute(int before_area,
                                             int current_village,
                                             int current_area,
                                             int target_village,
                                             int target_area,
                                             int unused) {
    (void)unused;
    const int previous_village =
        static_cast<int>(reinterpret_cast<intptr_t>(this));
    if ((previous_village == 3 && before_area == 2 &&
         current_village == 3 && current_area == 0 &&
         target_village == 2 && target_area == 0) ||
        (previous_village == 3 && before_area == 1 &&
         current_village == 3 && current_area == 0 &&
         target_village == 2 && target_area == 0) ||
        (previous_village == 2 && before_area == 0 &&
         current_village == 3 && current_area == 0 &&
         target_village == 3 && target_area == 1) ||
        (previous_village == 2 && before_area == 0 &&
         current_village == 3 && current_area == 0 &&
         target_village == 3 && target_area == 2) ||
        (previous_village == 6 && before_area == 0 &&
         current_village == 3 && current_area == 6 &&
         target_village == 3 && target_area == 1) ||
        (previous_village == 4 && before_area == 1 &&
         current_village == 4 && current_area == 0 &&
         target_village == 4 && target_area == 3) ||
        (previous_village == 4 && before_area == 3 &&
         current_village == 4 && current_area == 0 &&
         target_village == 4 && target_area == 1) ||
        (previous_village == 1 && before_area == 3 &&
         current_village == 4 && current_area == 0 &&
         target_village == 4 && target_area == 4) ||
        (previous_village == 4 && before_area == 4 &&
         current_village == 4 && current_area == 0 &&
         target_village == 1 && target_area == 3) ||
        (previous_village == 2 && before_area == 2 &&
         current_village == 2 && current_area == 8 &&
         target_village == 5 && target_area == 0) ||
        (previous_village == 5 && before_area == 0 &&
         current_village == 2 && current_area == 8 &&
         target_village == 2 && target_area == 2) ||
        (previous_village == 2 && before_area == 4 &&
         current_village == 2 && current_area == 0 &&
         target_village == 3 && target_area == 0) ||
        (previous_village == 4 && before_area == 1 &&
         current_village == 4 && current_area == 0 &&
         target_village == 1 && target_area == 3) ||
        (previous_village == 4 && before_area == 0 &&
         current_village == 1 && current_area == 3 &&
         target_village == 1 && target_area == 0) ||
        (previous_village == 3 && before_area == 6 &&
         current_village == 6 && current_area == 0 &&
         target_village == 6 && target_area == 1)) {
        return 1;
    }
    return 0;
}

// IDA_EA: 0x82861B6
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove31IsMovingBetweenWestCoastAndGentEiiii
// DEMANGLED: CHackLog_InvalidAreaMove::IsMovingBetweenWestCoastAndGent(int,int,int,int)
bool CHackLog_InvalidAreaMove::IsMovingBetweenWestCoastAndGent(
    int before_area, int target_village, int target_area, int unused) {
    CHackLog_InvalidAreaMove* target =
        reinterpret_cast<CHackLog_InvalidAreaMove*>(target_village);
    return (IsGentEntranceInWestCoast(before_area, unused) &&
            target->IsWestCostEntranceInGent(target_area, unused)) ||
           (IsWestCostEntranceInGent(before_area, unused) &&
            target->IsGentEntranceInWestCoast(target_area, unused));
}

// IDA_EA: 0x8286222
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove21IsMovingAboutPowerWarEii
// DEMANGLED: CHackLog_InvalidAreaMove::IsMovingAboutPowerWar(int,int)
bool CHackLog_InvalidAreaMove::IsMovingAboutPowerWar(int target_village,
                                                    int unused) {
    CHackLog_InvalidAreaMove* target =
        reinterpret_cast<CHackLog_InvalidAreaMove*>(target_village);
    return (IsHendonMyre(unused) && target->IsWarfare(unused)) ||
           (IsWarfare(unused) && target->IsHendonMyre(unused));
}

// IDA_EA: 0x8286272
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove36IsMovingBetweenSeriaRoomAndGuildAgitEiiii
// DEMANGLED: CHackLog_InvalidAreaMove::IsMovingBetweenSeriaRoomAndGuildAgit(int,int,int,int)
bool CHackLog_InvalidAreaMove::IsMovingBetweenSeriaRoomAndGuildAgit(
    int area, int target_village, int target_area, int unused) {
    CHackLog_InvalidAreaMove* target =
        reinterpret_cast<CHackLog_InvalidAreaMove*>(target_village);
    return (IsSeriaRoom(area, unused) && target->IsGuildAgit(unused)) ||
           (IsGuildAgit(unused) && target->IsSeriaRoom(target_area, unused));
}

// IDA_EA: 0x82862D0
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove25IsGentEntranceInWestCoastEii
// DEMANGLED: CHackLog_InvalidAreaMove::IsGentEntranceInWestCoast(int,int)
bool CHackLog_InvalidAreaMove::IsGentEntranceInWestCoast(int area,
                                                        int unused) {
    (void)unused;
    return static_cast<int>(reinterpret_cast<intptr_t>(this)) == 3 &&
           area == 6;
}

// IDA_EA: 0x82862EE
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove24IsWestCostEntranceInGentEii
// DEMANGLED: CHackLog_InvalidAreaMove::IsWestCostEntranceInGent(int,int)
bool CHackLog_InvalidAreaMove::IsWestCostEntranceInGent(int area, int unused) {
    (void)unused;
    return static_cast<int>(reinterpret_cast<intptr_t>(this)) == 6 &&
           area == 0;
}

// IDA_EA: 0x828630C
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove12IsHendonMyreEi
// DEMANGLED: CHackLog_InvalidAreaMove::IsHendonMyre(int)
bool CHackLog_InvalidAreaMove::IsHendonMyre(int unused) {
    (void)unused;
    return static_cast<int>(reinterpret_cast<intptr_t>(this)) == 2;
}

// IDA_EA: 0x8286318
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove9IsWarfareEi
// DEMANGLED: CHackLog_InvalidAreaMove::IsWarfare(int)
bool CHackLog_InvalidAreaMove::IsWarfare(int unused) {
    (void)unused;
    return static_cast<int>(reinterpret_cast<intptr_t>(this)) == 7;
}

// IDA_EA: 0x8286324
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove11IsGuildAgitEi
// DEMANGLED: CHackLog_InvalidAreaMove::IsGuildAgit(int)
bool CHackLog_InvalidAreaMove::IsGuildAgit(int unused) {
    (void)unused;
    return static_cast<int>(reinterpret_cast<intptr_t>(this)) == 8;
}

// IDA_EA: 0x8286330
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove19IsMovingInGuildAgitEii
// DEMANGLED: CHackLog_InvalidAreaMove::IsMovingInGuildAgit(int,int)
bool CHackLog_InvalidAreaMove::IsMovingInGuildAgit(int target_village,
                                                  int unused) {
    (void)unused;
    return static_cast<int>(reinterpret_cast<intptr_t>(this)) == 8 &&
           target_village == 8;
}

// IDA_EA: 0x8286350
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove15IsExceptionMoveEiiii
// DEMANGLED: CHackLog_InvalidAreaMove::IsExceptionMove(int,int,int,int)
bool CHackLog_InvalidAreaMove::IsExceptionMove(int before_area,
                                               int target_village,
                                               int target_area,
                                               int unused) {
    (void)unused;
    const int current_village =
        static_cast<int>(reinterpret_cast<intptr_t>(this));
    return (current_village == 6 && before_area == 0 &&
            target_village == 6 && target_area == 4) ||
           (current_village == 6 && before_area == 4 &&
            target_village == 6 && target_area == 0) ||
           (current_village == 3 && before_area == 8 &&
            target_village == 3 && target_area == 0) ||
           (current_village == 3 && before_area == 0 &&
            target_village == 3 && target_area == 8) ||
           (current_village == 3 && before_area == 1 &&
            target_village == 2 && target_area == 0) ||
           (current_village == 2 && before_area == 0 &&
            target_village == 3 && target_area == 1) ||
           (current_village == 2 && before_area == 2 &&
            target_village == 2 && target_area == 5) ||
           (current_village == 2 && before_area == 5 &&
            target_village == 2 && target_area == 2) ||
           (current_village == 2 && before_area == 1 &&
            target_village == 2 && target_area == 5) ||
           (current_village == 2 && before_area == 5 &&
            target_village == 2 && target_area == 1);
}

// IDA_EA: 0x8286462
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove11IsSeriaRoomEii
// DEMANGLED: CHackLog_InvalidAreaMove::IsSeriaRoom(int,int)
bool CHackLog_InvalidAreaMove::IsSeriaRoom(int area, int unused) {
    (void)unused;
    const int village = static_cast<int>(reinterpret_cast<intptr_t>(this));
    if (village == 1 && area == 1) {
        return true;
    }
    const bool legacy_seria_room =
        (village == 1 && area == 1) ||
        (village == 2 && area == 5) ||
        (village == 3 && area == 2) ||
        (village == 4 && area == 1) ||
        (village == 5 && area == 1) ||
        (village == 6 && area == 4) ||
        (village == 9 && area == 2) ||
        (village == 11 && area == 3);
    if (legacy_seria_room || village < 12) {
        return legacy_seria_room;
    }

    GameWorld* const world = G_GameWorld();
    Village* const village_data = world ? world->GetVillage(village) : 0;
    return village_data && village_data->area_count > 0 &&
           village_data->get_gate_area() == area;
}

// IDA_EA: 0x86D272C
// IDA_NAME: _ZN24CHackLog_InvalidAreaMove11GetHackTypeEv
// DEMANGLED: CHackLog_InvalidAreaMove::GetHackType(void)
int CHackLog_InvalidAreaMove::GetHackType() {
    return 501;
}
