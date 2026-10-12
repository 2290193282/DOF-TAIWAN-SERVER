#ifndef OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_DELETE_ACCOUNT_CARGO_H_
#define OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_DELETE_ACCOUNT_CARGO_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

class CDeleteAccountCargo : public CCommand {
public:
    // IDA_EA: 0x82A84CE
    // IDA_NAME: _ZN11game_master19CDeleteAccountCargoC2Ev
    // DEMANGLED: game_master::CDeleteAccountCargo::CDeleteAccountCargo(void)
    CDeleteAccountCargo();

    // IDA_EA: 0x84ABDB4
    // IDA_NAME: _ZN11game_master19CDeleteAccountCargo7executeEv
    // DEMANGLED: game_master::CDeleteAccountCargo::execute(void)
    virtual void execute();
};

#if defined(__i386__)
static_assert(sizeof(CDeleteAccountCargo) == 0x08u,
              "CDeleteAccountCargo must remain the CCommand-only i386 object");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_DELETE_ACCOUNT_CARGO_H_
