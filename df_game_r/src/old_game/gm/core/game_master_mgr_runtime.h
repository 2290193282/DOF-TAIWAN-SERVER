#ifndef OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_MGR_RUNTIME_H_
#define OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_MGR_RUNTIME_H_

#include <string>
#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA 0x82a8a28.  This is the command-root object owned by GlobalData.  The
// map is intentionally populated only with commands whose full transactions
// are already restored; unported original keys remain on oldGmRoutine's path.
class CGameMasterMgr {
public:
    CGameMasterMgr();
    ~CGameMasterMgr();
    int HandleRequest(CCommand* command);
    CCommand* GetCommand(int command_id) const;
    CCommand* GetCommand(const std::string& command_name) const;

private:
    IGameMaster* senior_;
    IGameMaster* junior_;
    CCommandMap command_map_;
};

#if defined(__i386__)
static_assert(sizeof(CGameMasterMgr) == 0x40u,
              "CGameMasterMgr must retain old i386 root layout");
#endif
}  // namespace game_master
#endif
