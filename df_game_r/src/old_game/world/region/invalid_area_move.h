#ifndef OLDSRV_OLD_GAME_WORLD_INVALID_AREA_MOVE_H_
#define OLDSRV_OLD_GAME_WORLD_INVALID_AREA_MOVE_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8285E14 in the companion source.



// IDA unit: 05_world_enter_village/CHackLog_InvalidAreaMove.
struct CHackLog_InvalidAreaMove {
    int checkSeriaRoom(int area, int unused);
    bool IsInvalidMovableArea(int before_area, int target_village,
                              int target_area, int unused);
    int isMovableRoute(int before_area, int current_village,
                       int current_area, int target_village,
                       int target_area, int unused);
    bool IsMovingBetweenWestCoastAndGent(int before_area, int target_village,
                                         int target_area, int unused);
    bool IsMovingAboutPowerWar(int target_village, int unused);
    bool IsMovingBetweenSeriaRoomAndGuildAgit(int area, int target_village,
                                              int target_area, int unused);
    bool IsGentEntranceInWestCoast(int area, int unused);
    bool IsWestCostEntranceInGent(int area, int unused);
    bool IsHendonMyre(int unused);
    bool IsWarfare(int unused);
    bool IsGuildAgit(int unused);
    bool IsMovingInGuildAgit(int target_village, int unused);
    bool IsExceptionMove(int before_area, int target_village, int target_area,
                         int unused);
    bool IsSeriaRoom(int area, int unused);

    int GetHackType();
};


#endif  // OLDSRV_OLD_GAME_WORLD_INVALID_AREA_MOVE_H_
