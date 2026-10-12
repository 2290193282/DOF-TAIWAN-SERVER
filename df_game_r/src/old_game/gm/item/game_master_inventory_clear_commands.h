#ifndef OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_INVENTORY_CLEAR_COMMANDS_H_
#define OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_INVENTORY_CLEAR_COMMANDS_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA C2: 0x82A8338.  CGameMasterMgr registers this command at numeric key 37.
class CClearInventoryCmd : public CCommand {
public:
    CClearInventoryCmd();

    virtual void execute();
};

// IDA C2: 0x82A8354.  CGameMasterMgr registers this command at numeric key 38.
class CClearAvatarInvenCmd : public CCommand {
public:
    CClearAvatarInvenCmd();

    virtual void execute();
};

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_INVENTORY_CLEAR_COMMANDS_H_
