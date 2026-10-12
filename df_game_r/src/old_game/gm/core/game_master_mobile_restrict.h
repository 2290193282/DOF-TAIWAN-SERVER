#ifndef OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_MOBILE_RESTRICT_H_
#define OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_MOBILE_RESTRICT_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

#include <string>

namespace game_master {

// IDA_EA: C2 0x82A89B8, parse 0x84B3BDC, execute 0x84B3C60.
// CGameMasterMgr registers this object under the string command "restrict".
class CMobileRestrictCommand : public CCommand {
public:
    CMobileRestrictCommand();

    virtual void execute();
    virtual void parse();

private:
    std::string target_;
    bool restricted_;
};

#if defined(__i386__)
static_assert(sizeof(CMobileRestrictCommand) == 0x10,
              "CMobileRestrictCommand must remain the old 16-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_USERINFO_GAME_MASTER_MOBILE_RESTRICT_H_
