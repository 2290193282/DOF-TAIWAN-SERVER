#ifndef OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_LEVEL_UP_H_
#define OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_LEVEL_UP_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA unit: 99_misc_global/game_master/CLevelUp.  CGameMasterMgr registers
// this command under the string key "lv".
class CLevelUp : public CCommand {
public:
    CLevelUp();

    virtual void parse();
    virtual void execute();

    CLevelUp* setLevel(int level);
    int getLevel();

private:
    int level_;
};

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_LEVEL_UP_H_
