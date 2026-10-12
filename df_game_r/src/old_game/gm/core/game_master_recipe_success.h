#ifndef OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_RECIPE_SUCCESS_H_
#define OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_RECIPE_SUCCESS_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA_EA: C2 0x82A8972, parse 0x84B399A, execute 0x84B3A34.
// CGameMasterMgr registers this object under the string command "recipeprob".
class CRecipeSuccess : public CCommand {
public:
    CRecipeSuccess();

    virtual void execute();
    virtual void parse();

private:
    int probability_mode_;
};

#if defined(__i386__)
static_assert(sizeof(CRecipeSuccess) == 0x0c,
              "CRecipeSuccess must remain the old 12-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_RECIPE_SUCCESS_H_
