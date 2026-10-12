#ifndef OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_PREMIUM_DELETE_COMMAND_H_
#define OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_PREMIUM_DELETE_COMMAND_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA C2: 0x82A8522.  CGameMasterMgr registers this command at numeric key 60.
class CUserPremiumDelete : public CCommand {
public:
    CUserPremiumDelete();

    virtual void execute();
};

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_PREMIUM_DELETE_COMMAND_H_
