#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_COMMERCE_AUCTION_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_COMMERCE_AUCTION_H_

#include "old_game/economy/trade/vending_machine_public_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

class CUser;
class CMTRand;

// IDA unit: 13_database_dispatch/Dispatcher_DepositMoney.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_DepositMoney
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597164
    // IDA_NAME: _ZN23Dispatcher_DepositMoneyC2Ev
    // DEMANGLED: Dispatcher_DepositMoney::Dispatcher_DepositMoney(void)
    Dispatcher_DepositMoney();

    // IDA_EA: 0x8240750
    // IDA_NAME: _ZN23Dispatcher_DepositMoneyD2Ev
    // DEMANGLED: Dispatcher_DepositMoney::~Dispatcher_DepositMoney()
    // IDA_EA: 0x8240780
    // IDA_NAME: _ZN23Dispatcher_DepositMoneyD0Ev
    // DEMANGLED: Dispatcher_DepositMoney::~Dispatcher_DepositMoney()
    virtual ~Dispatcher_DepositMoney();

    // IDA_EA: 0x8224FE4
    // IDA_NAME: _ZN23Dispatcher_DepositMoney12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_DepositMoney::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82341D6
    // IDA_NAME: _ZN23Dispatcher_DepositMoney4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_DepositMoney::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82341E0
    // IDA_NAME: _ZN23Dispatcher_DepositMoney7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_DepositMoney::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_WithdrawMoney.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_WithdrawMoney
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597180
    // IDA_NAME: _ZN24Dispatcher_WithdrawMoneyC2Ev
    // DEMANGLED: Dispatcher_WithdrawMoney::Dispatcher_WithdrawMoney(void)
    Dispatcher_WithdrawMoney();

    // IDA_EA: 0x8240702
    // IDA_NAME: _ZN24Dispatcher_WithdrawMoneyD2Ev
    // DEMANGLED: Dispatcher_WithdrawMoney::~Dispatcher_WithdrawMoney()
    // IDA_EA: 0x8240732
    // IDA_NAME: _ZN24Dispatcher_WithdrawMoneyD0Ev
    // DEMANGLED: Dispatcher_WithdrawMoney::~Dispatcher_WithdrawMoney()
    virtual ~Dispatcher_WithdrawMoney();

    // IDA_EA: 0x82250EC
    // IDA_NAME: _ZN24Dispatcher_WithdrawMoney12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_WithdrawMoney::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82341EA
    // IDA_NAME: _ZN24Dispatcher_WithdrawMoney4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_WithdrawMoney::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82341F4
    // IDA_NAME: _ZN24Dispatcher_WithdrawMoney7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_WithdrawMoney::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_BiddingRoutingItem.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_BiddingRoutingItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597110
    // IDA_NAME: _ZN29Dispatcher_BiddingRoutingItemC2Ev
    // DEMANGLED: Dispatcher_BiddingRoutingItem::Dispatcher_BiddingRoutingItem(void)
    Dispatcher_BiddingRoutingItem();

    // IDA_EA: 0x82405CA
    // IDA_NAME: _ZN29Dispatcher_BiddingRoutingItemD2Ev
    // DEMANGLED: Dispatcher_BiddingRoutingItem::~Dispatcher_BiddingRoutingItem()
    // IDA_EA: 0x82405FA
    // IDA_NAME: _ZN29Dispatcher_BiddingRoutingItemD0Ev
    // DEMANGLED: Dispatcher_BiddingRoutingItem::~Dispatcher_BiddingRoutingItem()
    virtual ~Dispatcher_BiddingRoutingItem();

    // IDA_EA: 0x82255CA
    // IDA_NAME: _ZN29Dispatcher_BiddingRoutingItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_BiddingRoutingItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8234186
    // IDA_NAME: _ZN29Dispatcher_BiddingRoutingItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_BiddingRoutingItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234190
    // IDA_NAME: _ZN29Dispatcher_BiddingRoutingItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_BiddingRoutingItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_BuyPrivateStoreItem.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_BuyPrivateStoreItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595DB4
    // IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItemC2Ev
    // DEMANGLED: Dispatcher_BuyPrivateStoreItem::Dispatcher_BuyPrivateStoreItem(void)
    Dispatcher_BuyPrivateStoreItem();

    // IDA_EA: 0x8243072
    // IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItemD2Ev
    // DEMANGLED: Dispatcher_BuyPrivateStoreItem::~Dispatcher_BuyPrivateStoreItem()
    // IDA_EA: 0x82430A2
    // IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItemD0Ev
    // DEMANGLED: Dispatcher_BuyPrivateStoreItem::~Dispatcher_BuyPrivateStoreItem()
    virtual ~Dispatcher_BuyPrivateStoreItem();

    // IDA_EA: 0x8202322
    // IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_BuyPrivateStoreItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233648
    // IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_BuyPrivateStoreItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233652
    // IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_BuyPrivateStoreItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_EnterPrivateStore.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_EnterPrivateStore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595DD0
    // IDA_NAME: _ZN28Dispatcher_EnterPrivateStoreC2Ev
    // DEMANGLED: Dispatcher_EnterPrivateStore::Dispatcher_EnterPrivateStore(void)
    Dispatcher_EnterPrivateStore();

    // IDA_EA: 0x8243024
    // IDA_NAME: _ZN28Dispatcher_EnterPrivateStoreD2Ev
    // DEMANGLED: Dispatcher_EnterPrivateStore::~Dispatcher_EnterPrivateStore()
    // IDA_EA: 0x8243054
    // IDA_NAME: _ZN28Dispatcher_EnterPrivateStoreD0Ev
    // DEMANGLED: Dispatcher_EnterPrivateStore::~Dispatcher_EnterPrivateStore()
    virtual ~Dispatcher_EnterPrivateStore();

    // IDA_EA: 0x82025AE
    // IDA_NAME: _ZN28Dispatcher_EnterPrivateStore12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_EnterPrivateStore::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823365C
    // IDA_NAME: _ZN28Dispatcher_EnterPrivateStore4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_EnterPrivateStore::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233666
    // IDA_NAME: _ZN28Dispatcher_EnterPrivateStore7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_EnterPrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_ExitPrivateStore.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_ExitPrivateStore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595DEC
    // IDA_NAME: _ZN27Dispatcher_ExitPrivateStoreC2Ev
    // DEMANGLED: Dispatcher_ExitPrivateStore::Dispatcher_ExitPrivateStore(void)
    Dispatcher_ExitPrivateStore();

    // IDA_EA: 0x8242FD6
    // IDA_NAME: _ZN27Dispatcher_ExitPrivateStoreD2Ev
    // DEMANGLED: Dispatcher_ExitPrivateStore::~Dispatcher_ExitPrivateStore()
    // IDA_EA: 0x8243006
    // IDA_NAME: _ZN27Dispatcher_ExitPrivateStoreD0Ev
    // DEMANGLED: Dispatcher_ExitPrivateStore::~Dispatcher_ExitPrivateStore()
    virtual ~Dispatcher_ExitPrivateStore();

    // IDA_EA: 0x820269C
    // IDA_NAME: _ZN27Dispatcher_ExitPrivateStore12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ExitPrivateStore::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233670
    // IDA_NAME: _ZN27Dispatcher_ExitPrivateStore4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ExitPrivateStore::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823367A
    // IDA_NAME: _ZN27Dispatcher_ExitPrivateStore7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ExitPrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_CreatePrivateStore.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_CreatePrivateStore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595E08
    // IDA_NAME: _ZN29Dispatcher_CreatePrivateStoreC2Ev
    // DEMANGLED: Dispatcher_CreatePrivateStore::Dispatcher_CreatePrivateStore(void)
    Dispatcher_CreatePrivateStore();

    // IDA_EA: 0x8242F88
    // IDA_NAME: _ZN29Dispatcher_CreatePrivateStoreD2Ev
    // DEMANGLED: Dispatcher_CreatePrivateStore::~Dispatcher_CreatePrivateStore()
    // IDA_EA: 0x8242FB8
    // IDA_NAME: _ZN29Dispatcher_CreatePrivateStoreD0Ev
    // DEMANGLED: Dispatcher_CreatePrivateStore::~Dispatcher_CreatePrivateStore()
    virtual ~Dispatcher_CreatePrivateStore();

    // IDA_EA: 0x820271C
    // IDA_NAME: _ZN29Dispatcher_CreatePrivateStore12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CreatePrivateStore::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233684
    // IDA_NAME: _ZN29Dispatcher_CreatePrivateStore4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CreatePrivateStore::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823368E
    // IDA_NAME: _ZN29Dispatcher_CreatePrivateStore7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CreatePrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_RemovePrivateStore.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_RemovePrivateStore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595E24
    // IDA_NAME: _ZN29Dispatcher_RemovePrivateStoreC2Ev
    // DEMANGLED: Dispatcher_RemovePrivateStore::Dispatcher_RemovePrivateStore(void)
    Dispatcher_RemovePrivateStore();

    // IDA_EA: 0x8242F3A
    // IDA_NAME: _ZN29Dispatcher_RemovePrivateStoreD2Ev
    // DEMANGLED: Dispatcher_RemovePrivateStore::~Dispatcher_RemovePrivateStore()
    // IDA_EA: 0x8242F6A
    // IDA_NAME: _ZN29Dispatcher_RemovePrivateStoreD0Ev
    // DEMANGLED: Dispatcher_RemovePrivateStore::~Dispatcher_RemovePrivateStore()
    virtual ~Dispatcher_RemovePrivateStore();

    // IDA_EA: 0x8202BB6
    // IDA_NAME: _ZN29Dispatcher_RemovePrivateStore12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RemovePrivateStore::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233698
    // IDA_NAME: _ZN29Dispatcher_RemovePrivateStore4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RemovePrivateStore::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82336A2
    // IDA_NAME: _ZN29Dispatcher_RemovePrivateStore7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RemovePrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_CompleteDisplay.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_CompleteDisplay
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595E40
    // IDA_NAME: _ZN26Dispatcher_CompleteDisplayC2Ev
    // DEMANGLED: Dispatcher_CompleteDisplay::Dispatcher_CompleteDisplay(void)
    Dispatcher_CompleteDisplay();

    // IDA_EA: 0x8242EEC
    // IDA_NAME: _ZN26Dispatcher_CompleteDisplayD2Ev
    // DEMANGLED: Dispatcher_CompleteDisplay::~Dispatcher_CompleteDisplay()
    // IDA_EA: 0x8242F1C
    // IDA_NAME: _ZN26Dispatcher_CompleteDisplayD0Ev
    // DEMANGLED: Dispatcher_CompleteDisplay::~Dispatcher_CompleteDisplay()
    virtual ~Dispatcher_CompleteDisplay();

    // IDA_EA: 0x8202CDA
    // IDA_NAME: _ZN26Dispatcher_CompleteDisplay12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CompleteDisplay::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82336AC
    // IDA_NAME: _ZN26Dispatcher_CompleteDisplay4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CompleteDisplay::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82336B6
    // IDA_NAME: _ZN26Dispatcher_CompleteDisplay7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CompleteDisplay::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_VerifyPrivateStoreItem.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_VerifyPrivateStoreItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85977A6
    // IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItemC2Ev
    // DEMANGLED: Dispatcher_VerifyPrivateStoreItem::Dispatcher_VerifyPrivateStoreItem(void)
    Dispatcher_VerifyPrivateStoreItem();

    // IDA_EA: 0x824004E
    // IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItemD2Ev
    // DEMANGLED: Dispatcher_VerifyPrivateStoreItem::~Dispatcher_VerifyPrivateStoreItem()
    // IDA_EA: 0x824007E
    // IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItemD0Ev
    // DEMANGLED: Dispatcher_VerifyPrivateStoreItem::~Dispatcher_VerifyPrivateStoreItem()
    virtual ~Dispatcher_VerifyPrivateStoreItem();

    // IDA_EA: 0x8229768
    // IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_VerifyPrivateStoreItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823433E
    // IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_VerifyPrivateStoreItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234348
    // IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_VerifyPrivateStoreItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionRegistItem.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionRegistItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85966C8
    // IDA_NAME: _ZN28Dispatcher_AuctionRegistItemC2Ev
    // DEMANGLED: Dispatcher_AuctionRegistItem::Dispatcher_AuctionRegistItem(void)
    Dispatcher_AuctionRegistItem();

    // IDA_EA: 0x8241D40
    // IDA_NAME: _ZN28Dispatcher_AuctionRegistItemD2Ev
    // DEMANGLED: Dispatcher_AuctionRegistItem::~Dispatcher_AuctionRegistItem()
    // IDA_EA: 0x8241D70
    // IDA_NAME: _ZN28Dispatcher_AuctionRegistItemD0Ev
    // DEMANGLED: Dispatcher_AuctionRegistItem::~Dispatcher_AuctionRegistItem()
    virtual ~Dispatcher_AuctionRegistItem();

    // IDA_EA: 0x8213E8A
    // IDA_NAME: _ZN28Dispatcher_AuctionRegistItem12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionRegistItem::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C0E
    // IDA_NAME: _ZN28Dispatcher_AuctionRegistItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionRegistItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C18
    // IDA_NAME: _ZN28Dispatcher_AuctionRegistItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionRegistItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionRegistCancel.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionRegistCancel
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85966E4
    // IDA_NAME: _ZN30Dispatcher_AuctionRegistCancelC2Ev
    // DEMANGLED: Dispatcher_AuctionRegistCancel::Dispatcher_AuctionRegistCancel(void)
    Dispatcher_AuctionRegistCancel();

    // IDA_EA: 0x8241CF2
    // IDA_NAME: _ZN30Dispatcher_AuctionRegistCancelD2Ev
    // DEMANGLED: Dispatcher_AuctionRegistCancel::~Dispatcher_AuctionRegistCancel()
    // IDA_EA: 0x8241D22
    // IDA_NAME: _ZN30Dispatcher_AuctionRegistCancelD0Ev
    // DEMANGLED: Dispatcher_AuctionRegistCancel::~Dispatcher_AuctionRegistCancel()
    virtual ~Dispatcher_AuctionRegistCancel();

    // IDA_EA: 0x8214B96
    // IDA_NAME: _ZN30Dispatcher_AuctionRegistCancel12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionRegistCancel::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C22
    // IDA_NAME: _ZN30Dispatcher_AuctionRegistCancel4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionRegistCancel::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C2C
    // IDA_NAME: _ZN30Dispatcher_AuctionRegistCancel7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionRegistCancel::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionBidding.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionBidding
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596700
    // IDA_NAME: _ZN25Dispatcher_AuctionBiddingC2Ev
    // DEMANGLED: Dispatcher_AuctionBidding::Dispatcher_AuctionBidding(void)
    Dispatcher_AuctionBidding();

    // IDA_EA: 0x8241C56
    // IDA_NAME: _ZN25Dispatcher_AuctionBiddingD2Ev
    // DEMANGLED: Dispatcher_AuctionBidding::~Dispatcher_AuctionBidding()
    // IDA_EA: 0x8241C86
    // IDA_NAME: _ZN25Dispatcher_AuctionBiddingD0Ev
    // DEMANGLED: Dispatcher_AuctionBidding::~Dispatcher_AuctionBidding()
    virtual ~Dispatcher_AuctionBidding();

    // IDA_EA: 0x821522E
    // IDA_NAME: _ZN25Dispatcher_AuctionBidding12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionBidding::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C36
    // IDA_NAME: _ZN25Dispatcher_AuctionBidding4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionBidding::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C40
    // IDA_NAME: _ZN25Dispatcher_AuctionBidding7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionBidding::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 05_world_enter_village/Dispatcher_AuctionBuyItemApiece.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionBuyItemApiece
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597346
    // IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApieceC2Ev
    // DEMANGLED: Dispatcher_AuctionBuyItemApiece::Dispatcher_AuctionBuyItemApiece(void)
    Dispatcher_AuctionBuyItemApiece();

    // IDA_EA: 0x8241CA4
    // IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApieceD2Ev
    // DEMANGLED: Dispatcher_AuctionBuyItemApiece::~Dispatcher_AuctionBuyItemApiece()
    // IDA_EA: 0x8241CD4
    // IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApieceD0Ev
    // DEMANGLED: Dispatcher_AuctionBuyItemApiece::~Dispatcher_AuctionBuyItemApiece()
    virtual ~Dispatcher_AuctionBuyItemApiece();

    // IDA_EA: 0x8214E44
    // IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApiece12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionBuyItemApiece::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233BE6
    // IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApiece4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionBuyItemApiece::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233BF0
    // IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApiece7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionBuyItemApiece::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionAskAveragePrice.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionAskAveragePrice
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85966AC
    // IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePriceC2Ev
    // DEMANGLED: Dispatcher_AuctionAskAveragePrice::Dispatcher_AuctionAskAveragePrice(void)
    Dispatcher_AuctionAskAveragePrice();

    // IDA_EA: 0x8241D8E
    // IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePriceD2Ev
    // DEMANGLED: Dispatcher_AuctionAskAveragePrice::~Dispatcher_AuctionAskAveragePrice()
    // IDA_EA: 0x8241DBE
    // IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePriceD0Ev
    // DEMANGLED: Dispatcher_AuctionAskAveragePrice::~Dispatcher_AuctionAskAveragePrice()
    virtual ~Dispatcher_AuctionAskAveragePrice();

    // IDA_EA: 0x8213940
    // IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePrice12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionAskAveragePrice::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233BFA
    // IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePrice4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionAskAveragePrice::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C04
    // IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePrice7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionAskAveragePrice::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionSearchByNoItmeKey.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionSearchByNoItmeKey
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596738
    // IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKeyC2Ev
    // DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::Dispatcher_AuctionSearchByNoItmeKey(void)
    Dispatcher_AuctionSearchByNoItmeKey();

    // IDA_EA: 0x8241BBA
    // IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKeyD2Ev
    // DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::~Dispatcher_AuctionSearchByNoItmeKey()
    // IDA_EA: 0x8241BEA
    // IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKeyD0Ev
    // DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::~Dispatcher_AuctionSearchByNoItmeKey()
    virtual ~Dispatcher_AuctionSearchByNoItmeKey();

    // IDA_EA: 0x82161E4
    // IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKey12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C5E
    // IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKey4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C68
    // IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKey7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionSearchByItemKey.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionSearchByItemKey
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859671C
    // IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKeyC2Ev
    // DEMANGLED: Dispatcher_AuctionSearchByItemKey::Dispatcher_AuctionSearchByItemKey(void)
    Dispatcher_AuctionSearchByItemKey();

    // IDA_EA: 0x8241C08
    // IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKeyD2Ev
    // DEMANGLED: Dispatcher_AuctionSearchByItemKey::~Dispatcher_AuctionSearchByItemKey()
    // IDA_EA: 0x8241C38
    // IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKeyD0Ev
    // DEMANGLED: Dispatcher_AuctionSearchByItemKey::~Dispatcher_AuctionSearchByItemKey()
    virtual ~Dispatcher_AuctionSearchByItemKey();

    // IDA_EA: 0x82159FC
    // IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKey12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionSearchByItemKey::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C4A
    // IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKey4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionSearchByItemKey::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C54
    // IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKey7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionSearchByItemKey::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionMyRegistedItemInfo.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionMyRegistedItemInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596754
    // IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfoC2Ev
    // DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::Dispatcher_AuctionMyRegistedItemInfo(void)
    Dispatcher_AuctionMyRegistedItemInfo();

    // IDA_EA: 0x8241B6C
    // IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfoD2Ev
    // DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::~Dispatcher_AuctionMyRegistedItemInfo()
    // IDA_EA: 0x8241B9C
    // IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfoD0Ev
    // DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::~Dispatcher_AuctionMyRegistedItemInfo()
    virtual ~Dispatcher_AuctionMyRegistedItemInfo();

    // IDA_EA: 0x8216966
    // IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfo12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C72
    // IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfo4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C7C
    // IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfo7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionMyBiddingInfo.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionMyBiddingInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596770
    // IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfoC2Ev
    // DEMANGLED: Dispatcher_AuctionMyBiddingInfo::Dispatcher_AuctionMyBiddingInfo(void)
    Dispatcher_AuctionMyBiddingInfo();

    // IDA_EA: 0x8241B1E
    // IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfoD2Ev
    // DEMANGLED: Dispatcher_AuctionMyBiddingInfo::~Dispatcher_AuctionMyBiddingInfo()
    // IDA_EA: 0x8241B4E
    // IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfoD0Ev
    // DEMANGLED: Dispatcher_AuctionMyBiddingInfo::~Dispatcher_AuctionMyBiddingInfo()
    virtual ~Dispatcher_AuctionMyBiddingInfo();

    // IDA_EA: 0x8216B9A
    // IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfo12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionMyBiddingInfo::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C86
    // IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfo4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionMyBiddingInfo::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233C90
    // IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfo7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionMyBiddingInfo::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_AuctionMyAuctionHistory.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_AuctionMyAuctionHistory
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859678C
    // IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistoryC2Ev
    // DEMANGLED: Dispatcher_AuctionMyAuctionHistory::Dispatcher_AuctionMyAuctionHistory(void)
    Dispatcher_AuctionMyAuctionHistory();

    // IDA_EA: 0x8241AD0
    // IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistoryD2Ev
    // DEMANGLED: Dispatcher_AuctionMyAuctionHistory::~Dispatcher_AuctionMyAuctionHistory()
    // IDA_EA: 0x8241B00
    // IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistoryD0Ev
    // DEMANGLED: Dispatcher_AuctionMyAuctionHistory::~Dispatcher_AuctionMyAuctionHistory()
    virtual ~Dispatcher_AuctionMyAuctionHistory();

    // IDA_EA: 0x8216D02
    // IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistory12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AuctionMyAuctionHistory::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233C9A
    // IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistory4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AuctionMyAuctionHistory::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233CA4
    // IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistory7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AuctionMyAuctionHistory::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_UseCraneStart.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_UseCraneStart
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85969D8
    // IDA_NAME: _ZN24Dispatcher_UseCraneStartC2Ev
    // DEMANGLED: Dispatcher_UseCraneStart::Dispatcher_UseCraneStart(void)
    Dispatcher_UseCraneStart();

    // IDA_EA: 0x82415A2
    // IDA_NAME: _ZN24Dispatcher_UseCraneStartD2Ev
    // DEMANGLED: Dispatcher_UseCraneStart::~Dispatcher_UseCraneStart()
    // IDA_EA: 0x82415D2
    // IDA_NAME: _ZN24Dispatcher_UseCraneStartD0Ev
    // DEMANGLED: Dispatcher_UseCraneStart::~Dispatcher_UseCraneStart()
    virtual ~Dispatcher_UseCraneStart();

    // IDA_EA: 0x821D2C4
    // IDA_NAME: _ZN24Dispatcher_UseCraneStart12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_UseCraneStart::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233DC6
    // IDA_NAME: _ZN24Dispatcher_UseCraneStart4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_UseCraneStart::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233DD0
    // IDA_NAME: _ZN24Dispatcher_UseCraneStart7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_UseCraneStart::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_CranePickUp.
// This native state-0 dispatcher has no fields beyond the verified base.
class Dispatcher_CranePickUp
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85969F4
    // IDA_NAME: _ZN22Dispatcher_CranePickUpC2Ev
    // DEMANGLED: Dispatcher_CranePickUp::Dispatcher_CranePickUp(void)
    Dispatcher_CranePickUp();

    // IDA_EA: 0x8241554
    // IDA_NAME: _ZN22Dispatcher_CranePickUpD2Ev
    // DEMANGLED: Dispatcher_CranePickUp::~Dispatcher_CranePickUp()
    // IDA_EA: 0x8241584
    // IDA_NAME: _ZN22Dispatcher_CranePickUpD0Ev
    // DEMANGLED: Dispatcher_CranePickUp::~Dispatcher_CranePickUp()
    virtual ~Dispatcher_CranePickUp();

    // IDA_EA: 0x821D508
    // IDA_NAME: _ZN22Dispatcher_CranePickUp12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CranePickUp::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233DDA
    // IDA_NAME: _ZN22Dispatcher_CranePickUp4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CranePickUp::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233DE4
    // IDA_NAME: _ZN22Dispatcher_CranePickUp7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CranePickUp::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_UseVendingMachine.  The C2
// allocation at PacketDispatcher slot 218 is eight bytes: the state-0
// dispatcher base followed by the native CMTRand pointer.
class Dispatcher_UseVendingMachine
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA unit: Dispatcher_UseVendingMachine::stPutItemResult_t.
    // The result is written at the exact 0x00/0x04/0x08/0x0c/0x0e offsets by
    // _putItemIntoUser and then serialized by dispatch_sig.
    struct stPutItemResult_t {
        int item_id;
        unsigned int item_count;
        int item_space;
        int16_t slot;
        uint8_t success;
        uint8_t reserved;
    };

    // IDA_EA: 0x821C25C
    // IDA_NAME: _ZN28Dispatcher_UseVendingMachineC2Ev
    // DEMANGLED: Dispatcher_UseVendingMachine::Dispatcher_UseVendingMachine(void)
    Dispatcher_UseVendingMachine();

    // IDA_EA: 0x82415F0
    // IDA_NAME: _ZN28Dispatcher_UseVendingMachineD2Ev
    // DEMANGLED: Dispatcher_UseVendingMachine::~Dispatcher_UseVendingMachine()
    // IDA_EA: 0x8241620
    // IDA_NAME: _ZN28Dispatcher_UseVendingMachineD0Ev
    // DEMANGLED: Dispatcher_UseVendingMachine::~Dispatcher_UseVendingMachine()
    virtual ~Dispatcher_UseVendingMachine();

    // IDA_EA: 0x821C2E6
    // IDA_NAME: _ZN28Dispatcher_UseVendingMachine12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_UseVendingMachine::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233DB2
    // IDA_NAME: _ZN28Dispatcher_UseVendingMachine4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_UseVendingMachine::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233DBC
    // IDA_NAME: _ZN28Dispatcher_UseVendingMachine7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_UseVendingMachine::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

private:
    // IDA_EA: 0x821B60A
    // IDA_NAME: _ZNK28Dispatcher_UseVendingMachine18_getVendingMachineEj
    // DEMANGLED: Dispatcher_UseVendingMachine::_getVendingMachine(unsigned int) const
    const stVendingMachine_t* _getVendingMachine(unsigned int script_index) const;

    // IDA_EA: 0x821B698
    // IDA_NAME: _ZNK28Dispatcher_UseVendingMachine22_getVendingMachineInfoEPK18stVendingMachine_tj
    // DEMANGLED: Dispatcher_UseVendingMachine::_getVendingMachineInfo(stVendingMachine_t const *,unsigned int) const
    const stVendingMachineInfo_t* _getVendingMachineInfo(
        const stVendingMachine_t* vending_machine,
        unsigned int group_number) const;

    // IDA_EA: 0x821B71C
    // IDA_NAME: _ZNK28Dispatcher_UseVendingMachine16_putItemIntoUserEP5CUsermjRNS_17stPutItemResult_tE
    // DEMANGLED: Dispatcher_UseVendingMachine::_putItemIntoUser(CUser *,unsigned long,unsigned int,Dispatcher_UseVendingMachine::stPutItemResult_t &)const
    int _putItemIntoUser(CUser* user,
                         unsigned long item_index,
                         unsigned int item_count,
                         stPutItemResult_t& result) const;

    // IDA_EA: 0x821BF82
    // IDA_NAME: _ZNK28Dispatcher_UseVendingMachine12_getGiveItemEPK22stVendingMachineInfo_t
    // DEMANGLED: Dispatcher_UseVendingMachine::_getGiveItem(stVendingMachineInfo_t const *) const
    Inven_Item _getGiveItem(
        const stVendingMachineInfo_t* vending_machine_info) const;

    // IDA_EA: 0x821C058
    // IDA_NAME: _ZNK28Dispatcher_UseVendingMachine13_getBonusItemEPK22stVendingMachineInfo_t
    // DEMANGLED: Dispatcher_UseVendingMachine::_getBonusItem(stVendingMachineInfo_t const *) const
    Inven_Item _getBonusItem(
        const stVendingMachineInfo_t* vending_machine_info) const;

    // IDA_EA: 0x821C17C
    // IDA_NAME: _ZNK28Dispatcher_UseVendingMachine15_notifyItemInfoEPK22stVendingMachineInfo_tP5CUserRKNS_17stPutItemResult_tE
    // DEMANGLED: Dispatcher_UseVendingMachine::_notifyItemInfo(stVendingMachineInfo_t const*,CUser *,Dispatcher_UseVendingMachine::stPutItemResult_t const&)const
    char _notifyItemInfo(const stVendingMachineInfo_t* vending_machine_info,
                         CUser* user,
                         const stPutItemResult_t& result) const;

    CMTRand* random_;
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DepositMoney) == 0x04u,
              "CMD308 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_WithdrawMoney) == 0x04u,
              "CMD309 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_BiddingRoutingItem) == 0x04u,
              "CMD301 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_BuyPrivateStoreItem) == 0x04u,
              "CMD85 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_EnterPrivateStore) == 0x04u,
              "CMD86 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_ExitPrivateStore) == 0x04u,
              "CMD87 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_CreatePrivateStore) == 0x04u,
              "CMD88 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_RemovePrivateStore) == 0x04u,
              "CMD89 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_CompleteDisplay) == 0x04u,
              "CMD90 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_VerifyPrivateStoreItem) == 0x04u,
              "CMD405 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionRegistItem) == 0x04u,
              "CMD186 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionRegistCancel) == 0x04u,
              "CMD187 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionBidding) == 0x04u,
              "CMD188 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionBuyItemApiece) == 0x04u,
              "CMD335 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionAskAveragePrice) == 0x04u,
              "CMD185 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionSearchByNoItmeKey) == 0x04u,
              "CMD190 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionSearchByItemKey) == 0x04u,
              "CMD189 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionMyRegistedItemInfo) == 0x04u,
              "CMD191 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionMyBiddingInfo) == 0x04u,
              "CMD192 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_AuctionMyAuctionHistory) == 0x04u,
              "CMD193 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_UseCraneStart) == 0x04u,
              "CMD486 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_CranePickUp) == 0x04u,
              "CMD487 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_UseVendingMachine::stPutItemResult_t) ==
                  0x10u,
              "CMD218 result must retain its exact 16-byte ABI");
static_assert(sizeof(Dispatcher_UseVendingMachine) == 0x08u,
              "CMD218 dispatcher must retain its CMTRand pointer ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_COMMERCE_AUCTION_H_
