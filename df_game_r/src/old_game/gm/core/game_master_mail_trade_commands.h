#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_GAME_MASTER_MAIL_TRADE_COMMANDS_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_GAME_MASTER_MAIL_TRADE_COMMANDS_H_

// Canonical non-registering GameMaster command providers for the old
// mail/trade/cash domain.  CGameMasterMgr ownership remains elsewhere: this
// header deliberately supplies only the recovered object ABI and bodies.

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA_EA: C2 0x82A8060, execute 0x84A8FFC.
// The old object contains only the CCommand base.
class CAutoMarketConditionsControlCmd : public CCommand {
public:
    CAutoMarketConditionsControlCmd();

    virtual void execute();
};

// IDA_EA: C2 0x82A853E, parse 0x84AC2AA, execute 0x84AC2E2.
class CAddItemByMail : public CCommand {
public:
    CAddItemByMail();

    virtual void execute();
    virtual void parse();

private:
    // IDA +0x08.  C2 leaves this scalar uninitialized; parse assigns it.
    int item_id_;
};

// IDA_EA: C2 0x82A88CA, parse 0x84B263A, execute 0x84B2640.
// This command has no derived fields beyond CCommand.
class CTestTradeRestrict : public CCommand {
public:
    CTestTradeRestrict();

    virtual void execute();
    virtual void parse();
};

#if defined(__i386__)
static_assert(sizeof(CAutoMarketConditionsControlCmd) == 0x08u,
              "CAutoMarketConditionsControlCmd must remain the old 8-byte command");
static_assert(sizeof(CAddItemByMail) == 0x0cu,
              "CAddItemByMail must remain the old 12-byte command");
static_assert(sizeof(CTestTradeRestrict) == 0x08u,
              "CTestTradeRestrict must remain the old 8-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_GAME_MASTER_MAIL_TRADE_COMMANDS_H_
