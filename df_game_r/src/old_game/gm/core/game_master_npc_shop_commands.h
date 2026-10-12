#ifndef OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_NPC_SHOP_COMMANDS_H_
#define OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_NPC_SHOP_COMMANDS_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA unit: 07_npc_shop_dialog/game_master/CChangeNPCMaxDayCmd.
class CChangeNPCMaxDayCmd : public CCommand {
public:
    CChangeNPCMaxDayCmd();

    virtual void execute();
    virtual void parse();

private:
    int npc_id_;
    int max_day_;
};

// IDA unit: 07_npc_shop_dialog/game_master/CChangeNPCFavorCmd.
class CChangeNPCFavorCmd : public CCommand {
public:
    CChangeNPCFavorCmd();

    virtual void execute();
    virtual void parse();

private:
    int npc_id_;
    int favor_;
};

// IDA unit: 07_npc_shop_dialog/game_master/COneADayItemShop.
class COneADayItemShop : public CCommand {
public:
    COneADayItemShop();

    virtual void execute();
};

// IDA unit: 07_npc_shop_dialog/game_master/NpcLimitBuyItem.
class NpcLimitBuyItem : public CCommand {
public:
    NpcLimitBuyItem();

    virtual void execute();
    virtual void parse();
};

// IDA unit: 07_npc_shop_dialog/game_master/CSecretShopNpc.
class CSecretShopNpc : public CCommand {
public:
    CSecretShopNpc();

    virtual void execute();
    virtual void parse();

private:
    int npc_id_;
};

// IDA unit: 07_npc_shop_dialog/game_master/CSecretShopCheckLottery.
class CSecretShopCheckLottery : public CCommand {
public:
    CSecretShopCheckLottery();

    virtual void execute();
};

#if defined(__i386__)
static_assert(sizeof(CChangeNPCMaxDayCmd) == 0x10u,
              "CChangeNPCMaxDayCmd must retain the old 16-byte ABI");
static_assert(sizeof(CChangeNPCFavorCmd) == 0x10u,
              "CChangeNPCFavorCmd must retain the old 16-byte ABI");
static_assert(sizeof(COneADayItemShop) == 0x08u,
              "COneADayItemShop must retain the old 8-byte ABI");
static_assert(sizeof(NpcLimitBuyItem) == 0x08u,
              "NpcLimitBuyItem must retain the old 8-byte ABI");
static_assert(sizeof(CSecretShopNpc) == 0x0cu,
              "CSecretShopNpc must retain the old 12-byte ABI");
static_assert(sizeof(CSecretShopCheckLottery) == 0x08u,
              "CSecretShopCheckLottery must retain the old 8-byte ABI");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_NPC_SHOP_COMMANDS_H_
