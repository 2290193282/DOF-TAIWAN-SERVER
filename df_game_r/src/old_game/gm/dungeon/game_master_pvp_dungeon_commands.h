#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PVP_DUNGEON_COMMANDS_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PVP_DUNGEON_COMMANDS_H_

#include <stdint.h>

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA unit: 09_dungeon_pvp_party/game_master/CPvPStartCmd.
// CGameMasterMgr::C2 registers this command under numeric key 2.
class CPvPStartCmd : public CCommand {
public:
    // IDA_EA: 0x82A8028
    // IDA_NAME: _ZN11game_master12CPvPStartCmdC2Ev
    CPvPStartCmd();

    // IDA_EA: 0x84A8F6A
    // IDA_NAME: _ZN11game_master12CPvPStartCmd7executeEv
    virtual void execute();
};

// IDA unit: 09_dungeon_pvp_party/game_master/CItemDropInDungeon.
// CGameMasterMgr::C2 registers this command under numeric key 52.
class CItemDropInDungeon : public CCommand {
public:
    // IDA_EA: 0x82A84B2
    // IDA_NAME: _ZN11game_master18CItemDropInDungeonC2Ev
    CItemDropInDungeon();

    // IDA_EA: 0x84ABCE8
    // IDA_NAME: _ZN11game_master18CItemDropInDungeon7executeEv
    virtual void execute();

    // IDA_EA: 0x84ABCB0
    // IDA_NAME: _ZN11game_master18CItemDropInDungeon5parseEv
    virtual void parse();

private:
    int32_t item_id_;  // +0x08
};

// IDA unit: 09_dungeon_pvp_party/game_master/DungeonDifficulty.
// CGameMasterMgr::C2 registers this command under numeric key 120.
class DungeonDifficulty : public CCommand {
public:
    // IDA_EA: 0x82A8892
    // IDA_NAME: _ZN11game_master17DungeonDifficultyC2Ev
    DungeonDifficulty();

    // IDA_EA: 0x84B2152
    // IDA_NAME: _ZN11game_master17DungeonDifficulty7executeEv
    virtual void execute();

    // IDA_EA: 0x84B2110
    // IDA_NAME: _ZN11game_master17DungeonDifficulty5parseEv
    virtual void parse();

private:
    uint8_t difficulty_;  // +0x08
};

#if defined(__i386__)
static_assert(sizeof(CPvPStartCmd) == 0x08u,
              "CPvPStartCmd must remain the original 8-byte command");
static_assert(sizeof(CItemDropInDungeon) == 0x0cu,
              "CItemDropInDungeon must remain the original 12-byte command");
static_assert(sizeof(DungeonDifficulty) == 0x0cu,
              "DungeonDifficulty must remain the original 12-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PVP_DUNGEON_COMMANDS_H_
