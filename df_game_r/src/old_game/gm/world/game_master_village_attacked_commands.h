#ifndef OLDSRV_OLD_GAME_WORLD_GAME_MASTER_VILLAGE_ATTACKED_COMMANDS_H_
#define OLDSRV_OLD_GAME_WORLD_GAME_MASTER_VILLAGE_ATTACKED_COMMANDS_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA unit: 05_world_enter_village/game_master/CVillageAttackedStartCmd.
class CVillageAttackedStartCmd : public CCommand {
public:
    CVillageAttackedStartCmd();

    virtual void execute();
    virtual void parse();

private:
    int value_;
};

// IDA unit: 05_world_enter_village/game_master/CVillageAttackedStopCmd.
class CVillageAttackedStopCmd : public CCommand {
public:
    CVillageAttackedStopCmd();

    virtual void execute();
};

// IDA unit: 05_world_enter_village/game_master/CVillageAttackedFightCmd.
class CVillageAttackedFightCmd : public CCommand {
public:
    CVillageAttackedFightCmd();

    virtual void execute();
};

#if defined(__i386__)
static_assert(sizeof(CVillageAttackedStartCmd) == 0x0cu,
              "CVillageAttackedStartCmd must retain the old 12-byte ABI");
static_assert(sizeof(CVillageAttackedStopCmd) == 0x08u,
              "CVillageAttackedStopCmd must retain the old 8-byte ABI");
static_assert(sizeof(CVillageAttackedFightCmd) == 0x08u,
              "CVillageAttackedFightCmd must retain the old 8-byte ABI");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_WORLD_GAME_MASTER_VILLAGE_ATTACKED_COMMANDS_H_
