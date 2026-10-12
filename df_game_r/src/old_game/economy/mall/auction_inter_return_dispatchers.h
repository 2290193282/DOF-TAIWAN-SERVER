#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_INTER_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_INTER_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 10_mail_trade_auction_cash/Inter_NotifyAuctionMail.
class Inter_NotifyAuctionMail : public ISigDispatcher {
public:
    Inter_NotifyAuctionMail();
    virtual ~Inter_NotifyAuctionMail();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultAskPrice.
class Inter_AuctionResultAskPrice : public ISigDispatcher {
public:
    Inter_AuctionResultAskPrice();
    virtual ~Inter_AuctionResultAskPrice();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultAskRegistedItemNum.
class Inter_AuctionResultAskRegistedItemNum : public ISigDispatcher {
public:
    Inter_AuctionResultAskRegistedItemNum();
    virtual ~Inter_AuctionResultAskRegistedItemNum();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultRegist.
class Inter_AuctionResultRegist : public ISigDispatcher {
public:
    Inter_AuctionResultRegist();
    virtual ~Inter_AuctionResultRegist();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultBidding.
class Inter_AuctionResultBidding : public ISigDispatcher {
public:
    Inter_AuctionResultBidding();
    virtual ~Inter_AuctionResultBidding();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84D6CDA.  The game is the Taiwan build, so the public
    // dispatcher always delegates to this billing-aware branch.
    int dispatch_sig_taiwan(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultRegistCancel.
class Inter_AuctionResultRegistCancel : public ISigDispatcher {
public:
    Inter_AuctionResultRegistCancel();
    virtual ~Inter_AuctionResultRegistCancel();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultItemList.
class Inter_AuctionResultItemList : public ISigDispatcher {
public:
    Inter_AuctionResultItemList();
    virtual ~Inter_AuctionResultItemList();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultMyRegistedItems.
class Inter_AuctionResultMyRegistedItems : public ISigDispatcher {
public:
    Inter_AuctionResultMyRegistedItems();
    virtual ~Inter_AuctionResultMyRegistedItems();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionResultMyBidding.
class Inter_AuctionResultMyBidding : public ISigDispatcher {
public:
    Inter_AuctionResultMyBidding();
    virtual ~Inter_AuctionResultMyBidding();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_NotifyAuctionMail) == 0x04u,
              "Inter_NotifyAuctionMail must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultAskPrice) == 0x04u,
              "Inter_AuctionResultAskPrice must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultAskRegistedItemNum) == 0x04u,
              "Inter_AuctionResultAskRegistedItemNum must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultRegist) == 0x04u,
              "Inter_AuctionResultRegist must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultBidding) == 0x04u,
              "Inter_AuctionResultBidding must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultRegistCancel) == 0x04u,
              "Inter_AuctionResultRegistCancel must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultItemList) == 0x04u,
              "Inter_AuctionResultItemList must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultMyRegistedItems) == 0x04u,
              "Inter_AuctionResultMyRegistedItems must retain the original base-only ABI");
static_assert(sizeof(Inter_AuctionResultMyBidding) == 0x04u,
              "Inter_AuctionResultMyBidding must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_INTER_RETURN_DISPATCHERS_H_
