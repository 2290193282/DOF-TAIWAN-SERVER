#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_SPACE_POOL_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_SPACE_POOL_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mail/mail_trade_value_types.h"

// IDA unit: 10_mail_trade_auction_cash/StaticPool<CTradeSpace,300>.
// CTradeSpace remains declared by the existing trade unit while its own
// business closure is staged.  This explicit specialization is the one
// canonical provider of the old 300-entry pool ABI; it does not redeclare
// CTradeSpace or create a replacement type.
template <>
class StaticPool<CTradeSpace, 300> {
public:
    enum { kCapacity = 300 };

    class CNode : public CTradeSpace {
    public:
        CNode();
        ~CNode();

        uint32_t dummy;
        uint8_t reference_count;
    };

    StaticPool();
    ~StaticPool();

    int Size();
    int Remain();
    CTradeSpace* Acquire();
    int GetIndex(CTradeSpace* node);
    CTradeSpace* Get(int index);
    CTradeSpace* Free(CTradeSpace* node);

    CNode* nodes;
    std::queue<CNode*> free_nodes;
    int error_state;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
typedef StaticPool<CTradeSpace, 300> TradeSpacePool;
static_assert(sizeof(CTradeSpace) == 0xF70u,
              "CTradeSpace old i386 ABI size must stay 0xF70");
static_assert(sizeof(TradeSpacePool::CNode) == 0xF78u,
              "StaticPool<CTradeSpace,300>::CNode old i386 ABI size must stay 0xF78");
static_assert(sizeof(TradeSpacePool) == 0x30u,
              "StaticPool<CTradeSpace,300> old i386 ABI size must stay 0x30");
static_assert(offsetof(TradeSpacePool, nodes) == 0x00u,
              "StaticPool<CTradeSpace,300>::nodes must stay at +0x00");
static_assert(offsetof(TradeSpacePool, free_nodes) == 0x04u,
              "StaticPool<CTradeSpace,300>::free_nodes must stay at +0x04");
static_assert(offsetof(TradeSpacePool, error_state) == 0x2Cu,
              "StaticPool<CTradeSpace,300>::error_state must stay at +0x2C");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TRADE_SPACE_POOL_RUNTIME_H_
