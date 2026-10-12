#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <utility>
#include <vector>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/activity/reward/event_base.h"
#include "infra/concurrent/timer/timer_sig.h"

class CUser;
struct SIG_AUTO_MARKET_CONDITIONS_CONTROL;
struct CAutoMarketConditionsControlLayoutVerifier;

// IDA unit: 10_mail_trade_auction_cash/CAuctionAveragePrice.
// The complete old object is one i386 std::map at offset zero.  Each item ID
// owns the ordered entries loaded from auction_average_price, and each entry
// carries its upgrade byte and server-provided average price.
class CAuctionAveragePrice {
public:
    // IDA_EA: 0x82F5A94.
    // IDA_NAME: _ZN20CAuctionAveragePriceC2Ev
    // DEMANGLED: CAuctionAveragePrice::CAuctionAveragePrice(void)
    CAuctionAveragePrice();

    // IDA_EA: 0x82AA812.
    // IDA_NAME: _ZN20CAuctionAveragePriceD2Ev
    // DEMANGLED: CAuctionAveragePrice::~CAuctionAveragePrice()
    ~CAuctionAveragePrice();

    // IDA_EA: 0x82F5AA8.
    // IDA_NAME: _ZN20CAuctionAveragePrice16LoadAveragePriceEv
    // DEMANGLED: CAuctionAveragePrice::LoadAveragePrice(void)
    int LoadAveragePrice();

    // IDA_EA: 0x82F5C24.
    // IDA_NAME: _ZN20CAuctionAveragePrice15SetAveragePriceEihi
    // DEMANGLED: CAuctionAveragePrice::SetAveragePrice(int,unsigned char,int)
    void SetAveragePrice(int item_id, unsigned char upgrade, int average_price);

    // IDA_EA: 0x82F5DA8.
    // IDA_NAME: _ZNK20CAuctionAveragePrice15GetAveragePriceEih
    // DEMANGLED: CAuctionAveragePrice::GetAveragePrice(int,unsigned char)const
    int GetAveragePrice(int item_id, unsigned char upgrade) const;

private:
    std::map<int, std::vector<std::pair<unsigned char, int> > >
        average_prices_;  // +0x00
};

// IDA unit: 10_mail_trade_auction_cash/stAuctionLimitItemParam_t.
// The old parameter is exactly the item identifier followed by its inclusive
// begin/end timestamps.  It is consumed by the auction-registration limiter.
struct stAuctionLimitItemParam_t {
    // IDA_EA: 0x8A8CC42.
    stAuctionLimitItemParam_t();
    // IDA_EA: 0x8A934EE.
    ~stAuctionLimitItemParam_t();

    int item_id;     // +0x00
    int begin_time;  // +0x04
    int end_time;    // +0x08
};

// IDA_EA: Inter_AuctionNotifyAuctionService::dispatch_sig @0x84D7AC8.
// The old dispatcher receives this packed five-byte server notification:
// service 0 selects the gold Auction proxy, service 1 selects Point Auction.
#pragma pack(push, 1)
struct SIG_AUCTION_NOTIFY_AUCTION_SERVICE {
    int32_t service_type;  // +0x00
    uint8_t running;       // +0x04
};
#pragma pack(pop)

static_assert(sizeof(SIG_AUCTION_NOTIFY_AUCTION_SERVICE) == 0x05u,
              "SIG_AUCTION_NOTIFY_AUCTION_SERVICE IDA ABI size must stay 0x05");

// IDA_EA: Inter_GenCeraTicket::dispatch_sig @0x84CEEA2.
// The monitor response contains a fixed 32-byte C string.  It is a named
// fixed-text field rather than an opaque packet byte array; the old handler
// still copies it with strncpy before calculating its length.
class CeraTicketResponseText {
public:
    enum { kCapacity = 0x20 };

    char* data() { return characters_; }
    const char* data() const { return characters_; }

private:
    char characters_[kCapacity];
};

#pragma pack(push, 1)
struct SIG_GEN_CERA_TICKET {
    int32_t response_value;                 // +0x00
    CeraTicketResponseText response_text;   // +0x04
};

// IDA_EA: Inter_AuctionSubMoney::dispatch_sig @0x84E8004.
// Both values are handed directly to the established hack-analyzer boundary.
struct SIG_AUCTION_SUB_MONEY {
    uint32_t hack_argument_0;  // +0x00
    uint32_t hack_argument_1;  // +0x04
};
#pragma pack(pop)

static_assert(sizeof(CeraTicketResponseText) == 0x20u,
              "CeraTicketResponseText IDA ABI size must stay 0x20");
static_assert(sizeof(SIG_GEN_CERA_TICKET) == 0x24u,
              "SIG_GEN_CERA_TICKET IDA ABI size must stay 0x24");
static_assert(sizeof(SIG_AUCTION_SUB_MONEY) == 0x08u,
              "SIG_AUCTION_SUB_MONEY IDA ABI size must stay 0x08");

// IDA_EA: Inter_AuctionResultBuyItemApiece::dispatch_sig @0x84D7C8E.
// The auction service supplies the target character, a refunded gold amount,
// the result byte forwarded to the client, and (on a successful refund) the
// response/detail byte.  This is a database-to-game-server record, not a
// client packet.
#pragma pack(push, 1)
struct SIG_AUCTION_RESULT_BUY_ITEM_APIECE {
    int32_t charac_no;          // +0x00
    uint32_t refund_money;      // +0x04
    uint8_t result;             // +0x08
    uint8_t response_detail;    // +0x09, present only when result == 0
    // The auction relay copies two additional source dwords into this tail.
    // Their business meaning is not recovered, but they are part of the
    // physical DB/Inter record and must not be truncated by typed readers.
    uint8_t relay_reserved_0a[8]; // +0x0a
};
#pragma pack(pop)

static_assert(sizeof(SIG_AUCTION_RESULT_BUY_ITEM_APIECE) == 0x12u,
              "auction apiece DB/Inter ABI size must stay 0x12");

// IDA_EA: CDispatchServerPacket::dispatchPointPacket @ 0x847DB04 and
// Inter_OwnerisVIP::dispatch_sig @ 0x84E7C90.  The point-auction service
// emits this 42-byte reply after the CMD188 Cera bidding request has been
// authenticated.  The unaligned policy value at +0x0d is intentional and is
// passed unchanged to ItemVendingMachine's auction-tax calculation.
#pragma pack(push, 1)
struct SIG_AUCTION_OWNER_IS_VIP {
    int32_t user_uid;                 // +0x00, routing UID
    uint32_t auction_key_low;         // +0x04
    uint32_t auction_key_high;        // +0x08
    uint8_t is_vip;                   // +0x0c
    int32_t tax_divisor;              // +0x0d, intentionally unaligned
    char character_name[13];          // +0x11, retained by the relay wire
    uint8_t relay_reserved_1e[4];     // +0x1e, not read by the game receiver
    int32_t bid_cera;                 // +0x22, split billing total
    int32_t billing_context;          // +0x26, preserved raw helper argument
};
#pragma pack(pop)

static_assert(sizeof(SIG_AUCTION_OWNER_IS_VIP) == 0x2au,
              "SIG_AUCTION_OWNER_IS_VIP IDA ABI size must stay 0x2A");
static_assert(offsetof(SIG_AUCTION_OWNER_IS_VIP, tax_divisor) == 0x0du,
              "SIG_AUCTION_OWNER_IS_VIP tax policy offset must stay +0x0D");
static_assert(offsetof(SIG_AUCTION_OWNER_IS_VIP, bid_cera) == 0x22u,
              "SIG_AUCTION_OWNER_IS_VIP bid amount offset must stay +0x22");

// IDA unit: 10_mail_trade_auction_cash/CAutoMarketConditionsControl.
// The table fields below are the named business values recovered from the
// original 0x444-byte i386 object.  They are not an opaque backing store.
class CAutoMarketConditionsControl {
public:
    // IDA_EA: 0x82F8108.
    CAutoMarketConditionsControl();
    // IDA_EA: 0x82F8144.
    ~CAutoMarketConditionsControl();

    // IDA_EA: 0x82F815C.
    int LoadScript(const char* filename);
    // IDA_EA: 0x82F8540.
    void SendNotiSetDurability(int durability_rate);
    // IDA_EA: 0x82F85A8.
    void SendNotiSetDurability();
    // IDA_EA: 0x82F85EA.
    void SendNotiSetDurability(CUser* user);
    // IDA_EA: 0x82F8684.
    int LoadDatabase(SIG_AUTO_MARKET_CONDITIONS_CONTROL* payload);
    // IDA_EA: 0x82F89BC.
    int GetGoldDropAmountRate(int charac_level, int base_gold);
    // IDA_EA: 0x82F8AE8.
    float GetItemDropRate(int level_range);
    // IDA_EA: 0x82F8C8C.
    bool ChangeItemRarity(int* item_rarity,
                          int* out_item_rarity,
                          int item_rarity_size,
                          double control_rarity);

private:
    friend struct CAutoMarketConditionsControlLayoutVerifier;

    int gold_control_state_;                         // +0x000
    double gold_drop_amount_rates_[10];              // +0x004
    double gold_current_rate_;                        // +0x054
    int gold_drop_amount_rate_per_level_[201];       // +0x05C
    int item_control_state_;                          // +0x380
    double equipment_drop_rates_[10];                // +0x384
    double item_current_rate_;                        // +0x3D4
    std::vector<int> equipment_drop_level_ranges_;   // +0x3DC
    int durability_control_state_;                    // +0x3E8
    double durability_decrease_rates_[10];           // +0x3EC
    double durability_current_rate_;                  // +0x43C
};

// IDA unit: 10_mail_trade_auction_cash/CAutoMarketConditionsControlEvent.
class CAutoMarketConditionsControlEvent : public CEventBase {
public:
    // IDA_EA: 0x8109C14.
    CAutoMarketConditionsControlEvent();
    // IDA_EA: 0x8109C30/0x8109C60.
    virtual ~CAutoMarketConditionsControlEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_LoadAutoMarketConditionsControl.
class Inter_LoadAutoMarketConditionsControl : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDFDC.
    Inter_LoadAutoMarketConditionsControl();
    // IDA_EA: 0x84F8322/0x84F8352.
    virtual ~Inter_LoadAutoMarketConditionsControl();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionNotifyAuctionService.
class Inter_AuctionNotifyAuctionService : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDE38.
    Inter_AuctionNotifyAuctionService();
    // IDA_EA: 0x84F8718/0x84F8748.
    virtual ~Inter_AuctionNotifyAuctionService();

    // IDA_EA: 0x84D7AC8.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_UpdatePurchaseCashItemBonus.
class Inter_UpdatePurchaseCashItemBonus : public ISigDispatcher {
public:
    // IDA_EA: 0x84EEAB0.
    Inter_UpdatePurchaseCashItemBonus();
    // IDA_EA: 0x84F6546/0x84F6576.
    virtual ~Inter_UpdatePurchaseCashItemBonus();

    // IDA_EA: 0x84E42E0.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionLogMessage.
class Inter_AuctionLogMessage : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDE1C.
    Inter_AuctionLogMessage();
    // IDA_EA: 0x84F8766/0x84F8796.
    virtual ~Inter_AuctionLogMessage();

    // IDA_EA: 0x84D7A90.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_PointAuctionLogMessage.
class Inter_PointAuctionLogMessage : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDE54.
    Inter_PointAuctionLogMessage();
    // IDA_EA: 0x84F86CA/0x84F86FA.
    virtual ~Inter_PointAuctionLogMessage();

    // IDA_EA: 0x84D7C4C.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_GenCeraTicket.
class Inter_GenCeraTicket : public ISigDispatcher {
public:
    // IDA_EA: 0x84ED4D0.
    Inter_GenCeraTicket();
    // IDA_EA: 0x84F9EDC/0x84F9F0C.
    virtual ~Inter_GenCeraTicket();

    // IDA_EA: 0x84CEEA2.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_AuctionSubMoney.
class Inter_AuctionSubMoney : public ISigDispatcher {
public:
    // IDA_EA: 0x84EEE4C.
    Inter_AuctionSubMoney();
    // IDA_EA: 0x84F5980/0x84F59B0.
    virtual ~Inter_AuctionSubMoney();

    // IDA_EA: 0x84E8004.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 05_world_enter_village/Inter_AuctionResultBuyItemApiece.
class Inter_AuctionResultBuyItemApiece : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDE8C.
    Inter_AuctionResultBuyItemApiece();
    // IDA_EA: 0x84F862E/0x84F865E.
    virtual ~Inter_AuctionResultBuyItemApiece();

    // IDA_EA: 0x84D7C8E.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Inter_PointAuctionChargeCeraPoint.
class Inter_PointAuctionChargeCeraPoint : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDE70.
    Inter_PointAuctionChargeCeraPoint();
    // IDA_EA: 0x84F867C/0x84F86AC.
    virtual ~Inter_PointAuctionChargeCeraPoint();

    // IDA_EA: 0x84D7C84.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 10_mail_trade_auction_cash/Timer_CheckAuctionReady.
// The old D2 body at 0x863E8CE installs vtable 0x8CE8748 and then invokes
// TimerSig::~TimerSig(), proving the direct public TimerSig base.  That
// vtable has exactly dispatch_sig, D2, and D0: RegistNextTimer is not virtual.
class Timer_CheckAuctionReady : public TimerSig {
public:
    // IDA_EA: 0x863E8CE.
    // IDA_NAME: _ZN23Timer_CheckAuctionReadyD2Ev
    // DEMANGLED: Timer_CheckAuctionReady::~Timer_CheckAuctionReady()
    // IDA_EA: 0x863E8FE.
    // IDA_NAME: _ZN23Timer_CheckAuctionReadyD0Ev
    // DEMANGLED: Timer_CheckAuctionReady::~Timer_CheckAuctionReady() deleting destructor
    virtual ~Timer_CheckAuctionReady();

    // IDA_EA: 0x863A808.
    // IDA_NAME: _ZN23Timer_CheckAuctionReady12dispatch_sigEiij
    // DEMANGLED: Timer_CheckAuctionReady::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x863A858.
    // IDA_NAME: _ZN23Timer_CheckAuctionReady15RegistNextTimerEv
    // DEMANGLED: Timer_CheckAuctionReady::RegistNextTimer(void)
    int RegistNextTimer();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct CAutoMarketConditionsControlLayoutVerifier {
    static_assert(offsetof(CAutoMarketConditionsControl,
                           gold_drop_amount_rates_) == 0x004u,
                  "CAutoMarketConditionsControl gold table offset must stay 0x004");
    static_assert(offsetof(CAutoMarketConditionsControl, gold_current_rate_) ==
                      0x054u,
                  "CAutoMarketConditionsControl gold rate offset must stay 0x054");
    static_assert(offsetof(CAutoMarketConditionsControl,
                           gold_drop_amount_rate_per_level_) == 0x05Cu,
                  "CAutoMarketConditionsControl gold-level table offset must stay 0x05C");
    static_assert(offsetof(CAutoMarketConditionsControl, item_control_state_) ==
                      0x380u,
                  "CAutoMarketConditionsControl item state offset must stay 0x380");
    static_assert(offsetof(CAutoMarketConditionsControl,
                           equipment_drop_rates_) == 0x384u,
                  "CAutoMarketConditionsControl equipment table offset must stay 0x384");
    static_assert(offsetof(CAutoMarketConditionsControl, item_current_rate_) ==
                      0x3D4u,
                  "CAutoMarketConditionsControl item rate offset must stay 0x3D4");
    static_assert(offsetof(CAutoMarketConditionsControl,
                           equipment_drop_level_ranges_) == 0x3DCu,
                  "CAutoMarketConditionsControl level-range vector offset must stay 0x3DC");
    static_assert(offsetof(CAutoMarketConditionsControl,
                           durability_control_state_) == 0x3E8u,
                  "CAutoMarketConditionsControl durability state offset must stay 0x3E8");
    static_assert(offsetof(CAutoMarketConditionsControl,
                           durability_decrease_rates_) == 0x3ECu,
                  "CAutoMarketConditionsControl durability table offset must stay 0x3EC");
    static_assert(offsetof(CAutoMarketConditionsControl,
                           durability_current_rate_) == 0x43Cu,
                  "CAutoMarketConditionsControl durability rate offset must stay 0x43C");
};
static_assert(sizeof(CAuctionAveragePrice) == 0x18u,
              "CAuctionAveragePrice IDA ABI size must stay 0x18");
static_assert(sizeof(stAuctionLimitItemParam_t) == 0x0Cu,
              "stAuctionLimitItemParam_t IDA ABI size must stay 0x0C");
static_assert(sizeof(CAutoMarketConditionsControl) == 0x444u,
              "CAutoMarketConditionsControl IDA ABI size must stay 0x444");
static_assert(sizeof(CAutoMarketConditionsControlEvent) == 0x0Cu,
              "CAutoMarketConditionsControlEvent IDA ABI size must stay 0x0C");
static_assert(sizeof(Inter_LoadAutoMarketConditionsControl) == 0x04u,
              "Inter_LoadAutoMarketConditionsControl IDA ABI size must stay 0x04");
static_assert(sizeof(Inter_AuctionNotifyAuctionService) == 0x04u,
              "Inter_AuctionNotifyAuctionService IDA ABI size must stay 0x04");
static_assert(sizeof(Inter_UpdatePurchaseCashItemBonus) == 0x04u,
              "Inter_UpdatePurchaseCashItemBonus IDA ABI size must stay 0x04");
static_assert(sizeof(Inter_AuctionLogMessage) == 0x04u,
              "Inter_AuctionLogMessage IDA ABI size must stay 0x04");
static_assert(sizeof(Inter_PointAuctionLogMessage) == 0x04u,
              "Inter_PointAuctionLogMessage IDA ABI size must stay 0x04");
static_assert(sizeof(Inter_GenCeraTicket) == 0x04u,
              "Inter_GenCeraTicket IDA ABI size must stay 0x04");
static_assert(sizeof(Inter_AuctionSubMoney) == 0x04u,
              "Inter_AuctionSubMoney IDA ABI size must stay 0x04");
static_assert(sizeof(Inter_AuctionResultBuyItemApiece) == 0x04u,
              "Inter_AuctionResultBuyItemApiece must remain base-only");
static_assert(sizeof(Inter_PointAuctionChargeCeraPoint) == 0x04u,
              "Inter_PointAuctionChargeCeraPoint IDA ABI size must stay 0x04");
static_assert(sizeof(Timer_CheckAuctionReady) == 0x04u,
              "Timer_CheckAuctionReady IDA ABI size must stay 0x04");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_AUCTION_RUNTIME_H_
