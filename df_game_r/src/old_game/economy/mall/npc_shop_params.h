#ifndef OLDSRV_OLD_GAME_NPC_NPC_SHOP_PARAMS_H_
#define OLDSRV_OLD_GAME_NPC_NPC_SHOP_PARAMS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8231AC0 in the companion source.


#include <stdint.h>

#include "infra/network/packet/message_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

struct CItemLimitEdition;
struct Inven_Item;


class CItem;
class CUser;
class PacketBuf;
struct MSG_BUY_CERASHOP_ITEM;
struct stNPCRelationship_t;

class CNPCDynamicInfo;
enum ENUM_NPC_FAVOR_CHANGE_CONST : int;

// IDA unit: 07_npc_shop_dialog/LimitNpcBuyItem.
struct LimitNpcBuyItem : public ParamBase {
    LimitNpcBuyItem();
    virtual ~LimitNpcBuyItem();

    int result;
};

void LimitNpcBuyItem_deleting_destructor(LimitNpcBuyItem* self);

// IDA unit: 07_npc_shop_dialog/STQuestShopParam.
struct STQuestShopParam : public ParamBase {
    STQuestShopParam();
    virtual ~STQuestShopParam();

    int result;
};

void STQuestShopParam_deleting_destructor(STQuestShopParam* self);

// IDA unit: 07_npc_shop_dialog/STQuestShopBuyParam.
struct STQuestShopBuyParam : public ParamBase {
    STQuestShopBuyParam();
    virtual ~STQuestShopBuyParam();

    uint8_t status_id;       // IDA +0x04
    uint8_t reserved_05;     // i386 alignment byte; C2 leaves it untouched
    uint16_t buying_count;   // IDA +0x06
    int result;              // IDA +0x08
};

#if defined(__i386__)
static_assert(sizeof(STQuestShopBuyParam) == 0x0cu &&
                  offsetof(STQuestShopBuyParam, status_id) == 0x04u &&
                  offsetof(STQuestShopBuyParam, buying_count) == 0x06u &&
                  offsetof(STQuestShopBuyParam, result) == 0x08u,
              "quest-shop buy result ABI drift");
#endif

void STQuestShopBuyParam_deleting_destructor(STQuestShopBuyParam* self);

// IDA unit: 07_npc_shop_dialog/stSecretShopOpenCloseParam.
struct stSecretShopOpenCloseParam : public ParamBase {
    stSecretShopOpenCloseParam();
    virtual ~stSecretShopOpenCloseParam();

    int result;
};

void stSecretShopOpenCloseParam_deleting_destructor(
    stSecretShopOpenCloseParam* self);

// IDA unit: 07_npc_shop_dialog/stSecretShopBuyItemParam.
struct stSecretShopBuyItemParam : public ParamBase {
    stSecretShopBuyItemParam();
    virtual ~stSecretShopBuyItemParam();

    int result;
};

void stSecretShopBuyItemParam_deleting_destructor(
    stSecretShopBuyItemParam* self);

// IDA unit: 07_npc_shop_dialog/MsgLimitNpcBuyItem.
#pragma pack(push, 1)
struct MsgLimitNpcBuyItem : public MSG_BASE {
    uint32_t item_id;  // +0x0d
    uint32_t npc_id;   // +0x11
};

// IDA unit: 07_npc_shop_dialog/MSG_SECRET_SHOP_BUY_ITEM.
struct MSG_SECRET_SHOP_BUY_ITEM : public MSG_BASE {
    uint32_t shop_item_index;  // +0x0d
    int32_t buy_count;         // +0x11
};

// IDA unit: 07_npc_shop_dialog/MSG_SECRET_SHOP_OPEN_CLOSE.
struct MSG_SECRET_SHOP_OPEN_CLOSE : public MSG_BASE {
    uint8_t buying;  // +0x0d
};
#pragma pack(pop)

static_assert(sizeof(MsgLimitNpcBuyItem) == 0x15u &&
                  offsetof(MsgLimitNpcBuyItem, item_id) == 0x0du &&
                  offsetof(MsgLimitNpcBuyItem, npc_id) == 0x11u,
              "limit-NPC buy request ABI drift");
static_assert(sizeof(MSG_SECRET_SHOP_BUY_ITEM) == 0x15u &&
                  offsetof(MSG_SECRET_SHOP_BUY_ITEM, shop_item_index) ==
                      0x0du &&
                  offsetof(MSG_SECRET_SHOP_BUY_ITEM, buy_count) == 0x11u,
              "secret-shop buy request ABI drift");
static_assert(sizeof(MSG_SECRET_SHOP_OPEN_CLOSE) == 0x0eu &&
                  offsetof(MSG_SECRET_SHOP_OPEN_CLOSE, buying) == 0x0du,
              "secret-shop open/close request ABI drift");

// IDA unit: 07_npc_shop_dialog/Dispatcher_SecretShopBuyItem.
class Dispatcher_SecretShopBuyItem
    : public IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                               stSecretShopBuyItemParam,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_SecretShopBuyItem();
    virtual ~Dispatcher_SecretShopBuyItem();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecretShopBuyItem) == 0x04u,
              "Dispatcher_SecretShopBuyItem must retain its i386 ABI size");
#endif

// IDA unit: 07_npc_shop_dialog/Dispatcher_SecretShopOpenClose.
class Dispatcher_SecretShopOpenClose
    : public IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                               stSecretShopOpenCloseParam,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_SecretShopOpenClose();
    virtual ~Dispatcher_SecretShopOpenClose();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecretShopOpenClose) == 0x04u,
              "Dispatcher_SecretShopOpenClose must retain its i386 ABI size");
#endif

// IDA unit: 07_npc_shop_dialog/Dispatcher_LimitNpcBuyItem.
class Dispatcher_LimitNpcBuyItem
    : public IPacketDispatcher<MsgLimitNpcBuyItem,
                               LimitNpcBuyItem,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_LimitNpcBuyItem();
    virtual ~Dispatcher_LimitNpcBuyItem();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_LimitNpcBuyItem) == 0x04u,
              "LimitNpcBuyItem dispatcher must retain i386 base-only ABI");
#endif

// IDA unit: 07_npc_shop_dialog/Dispatcher_Shop_Coin_Event.
class Dispatcher_Shop_Coin_Event
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Shop_Coin_Event();
    virtual ~Dispatcher_Shop_Coin_Event();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Shop_Coin_Event) == 0x04u,
              "ShopCoinEvent dispatcher must retain i386 base-only ABI");
#endif

// IDA unit: 07_npc_shop_dialog/Dispatcher_DungeonNPCBuffInfo.
class Dispatcher_DungeonNPCBuffInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DungeonNPCBuffInfo();
    virtual ~Dispatcher_DungeonNPCBuffInfo();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DungeonNPCBuffInfo) == 0x04u,
              "DungeonNPCBuffInfo dispatcher must retain i386 base-only ABI");
#endif

// IDA unit: 07_npc_shop_dialog/Dispatcher_BuyCeraShopItem.
class Dispatcher_BuyCeraShopItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_BuyCeraShopItem();
    virtual ~Dispatcher_BuyCeraShopItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    bool useCountDownCoinInFreeCoinDungeon(
        CUser* user,
        const MSG_BUY_CERASHOP_ITEM& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BuyCeraShopItem) == 0x04u,
              "BuyCeraShopItem dispatcher must retain i386 base-only ABI");
#endif

// IDA unit: 13_database_dispatch/Dispatcher_BuyAutomatItem.
class Dispatcher_BuyAutomatItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_BuyAutomatItem();
    virtual ~Dispatcher_BuyAutomatItem();

    // IDA_EA: 0x81FE578
    // IDA_NAME: _ZN25Dispatcher_BuyAutomatItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_BuyAutomatItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BuyAutomatItem) == 0x04u,
              "BuyAutomatItem dispatcher must retain i386 base-only ABI");
#endif

// IDA unit: 07_npc_shop_dialog/Dispatcher_BuyCeraShopLimitItem.
class Dispatcher_BuyCeraShopLimitItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_BuyCeraShopLimitItem();
    virtual ~Dispatcher_BuyCeraShopLimitItem();

    int _checkUserState(CUser* user) const;
    int _checkBuyCondition(CUser* user,
                           const CItemLimitEdition* item_info) const;
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BuyCeraShopLimitItem) == 0x04u,
              "BuyCeraShopLimitItem dispatcher must retain i386 base-only ABI");
#endif


#endif  // OLDSRV_OLD_GAME_NPC_NPC_SHOP_PARAMS_H_
