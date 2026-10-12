#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_STATISTIC_MANAGER_RUNTIME_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_STATISTIC_MANAGER_RUNTIME_H_

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

class CParty;
class CUser;

// IDA unit: CStatisticMgr @0x82AA89E and its two contiguous children.  The
// two map-backed child types are canonically declared by the dungeon/PvP
// value header; this header only establishes their owning aggregate.

class CStatisticMgr {
public:
    // IDA_EA: 0x82AA89E.
    CStatisticMgr();

private:
    CLevelDungeonPlayStatistic level_dungeon_play_;  // +0x00
    CDungeonExpStatistic dungeon_exp_;               // +0x18

    friend class CParty;
    friend class CUser;
};

#if defined(__i386__)
static_assert(sizeof(CLevelDungeonPlayStatistic) == 0x18u,
              "level dungeon statistic must remain map-sized");
static_assert(sizeof(CDungeonExpStatistic) == 0x18u,
              "dungeon EXP statistic must remain map-sized");
static_assert(sizeof(CStatisticMgr) == 0x30u,
              "CStatisticMgr must preserve its old 0x30-byte layout");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_STATISTIC_MANAGER_RUNTIME_H_
