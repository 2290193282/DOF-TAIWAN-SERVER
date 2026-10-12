#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_GAME_MASTER_BILLING_RECHARGE_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_GAME_MASTER_BILLING_RECHARGE_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

#include <string>

namespace game_master {

// IDA_EA: C2 0x82A898E, parse 0x84B3A60, execute 0x84B3AD4.
// CGameMasterMgr registers this object under the string command "cash".
class CBillingRechargeCommand : public CCommand {
public:
    CBillingRechargeCommand();

    virtual void execute();
    virtual void parse();

private:
    std::string currency_;
    unsigned int value_;
};

#if defined(__i386__)
static_assert(sizeof(CBillingRechargeCommand) == 0x10,
              "CBillingRechargeCommand must remain the old 16-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_GAME_MASTER_BILLING_RECHARGE_H_
