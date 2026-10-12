#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PCROOM_REWARD_RESET_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PCROOM_REWARD_RESET_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA_EA: C2 0x82A89E2, parse 0x84B3D12, execute 0x84B3D20.
// CGameMasterMgr registers this object under "pcroomrewardreset".
class CPcroomRewardReset : public CCommand {
public:
    CPcroomRewardReset();

    virtual void execute();
    virtual void parse();
};

#if defined(__i386__)
static_assert(sizeof(CPcroomRewardReset) == 0x08,
              "CPcroomRewardReset must remain the old 8-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_GAME_MASTER_PCROOM_REWARD_RESET_H_
