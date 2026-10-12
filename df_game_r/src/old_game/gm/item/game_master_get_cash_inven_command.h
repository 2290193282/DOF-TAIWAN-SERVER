#ifndef OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_GET_CASH_INVEN_COMMAND_H_
#define OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_GET_CASH_INVEN_COMMAND_H_

#include <stdint.h>

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA C2: 0x82A85CA.  CGameMasterMgr registers this command at numeric key 70.
class CGetCashInven : public CCommand {
public:
    CGetCashInven();

    virtual void parse();
    virtual void execute();

private:
    int16_t item_space_;
};

#if defined(__i386__)
static_assert(sizeof(CGetCashInven) == 0x0c,
              "CGetCashInven must remain the old 12-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_GET_CASH_INVEN_COMMAND_H_
