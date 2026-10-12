#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PCROOM_PREMIUM_CMD_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PCROOM_PREMIUM_CMD_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA unit: 09_dungeon_pvp_party/game_master/CPcroomPremiumCmd.  The command
// has the CCommand parser pointer at +0x04 and the parsed duration at +0x08.
// It is deliberately not registered here: CGameMasterMgr owns registration.
class CPcroomPremiumCmd : public CCommand {
public:
    // IDA_EA: 0x82A822E
    // IDA_NAME: _ZN11game_master17CPcroomPremiumCmdC2Ev
    CPcroomPremiumCmd();

    // IDA_EA: 0x84A9DE8
    // IDA_NAME: _ZN11game_master17CPcroomPremiumCmd7executeEv
    virtual void execute();

    // IDA_EA: 0x84A9DB0
    // IDA_NAME: _ZN11game_master17CPcroomPremiumCmd5parseEv
    virtual void parse();

private:
    int premium_duration_second_;  // +0x08
};

#if defined(__i386__)
static_assert(sizeof(CPcroomPremiumCmd) == 0x0cu,
              "CPcroomPremiumCmd must remain the original 12-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PCROOM_PREMIUM_CMD_H_
