#include "common/manager/reliable_person.h"

#include <stdint.h>

// IDA_DATA_EA: 0x9370F5C
int DEFAULT_MAX_FATIGUE = 156;

// IDA_EA: 0x84EBD74
// IDA_NAME: _ZN15CReliablePerson17SetTradeGoldDailyEj
// DEMANGLED: CReliablePerson::SetTradeGoldDaily(unsigned int)
CReliablePerson* CReliablePerson::SetTradeGoldDaily(
    uint32_t trade_gold_daily_value) {
    trade_gold_daily = trade_gold_daily_value;
    return this;
}

// IDA_EA: 0x86453B8
// IDA_NAME: _ZN15CReliablePerson13InitTradeGoldEjjj
// DEMANGLED: CReliablePerson::InitTradeGold(unsigned int,unsigned int,unsigned int)
CReliablePerson* CReliablePerson::InitTradeGold(uint32_t total,
                                                uint32_t total_billion,
                                                uint32_t daily) {
    trade_gold_total = total;
    trade_gold_total_billion = total_billion;
    trade_gold_daily = daily;
    return this;
}

// IDA_EA: 0x86453D8
// IDA_NAME: _ZN15CReliablePerson18IncrementTradeGoldEj
// DEMANGLED: CReliablePerson::IncrementTradeGold(unsigned int)
CReliablePerson* CReliablePerson::IncrementTradeGold(uint32_t amount) {
    static const uint32_t kTradeGoldBillionUnit = 1000000000u;

    trade_gold_daily += amount;
    trade_gold_total += amount;

    CReliablePerson* result = reinterpret_cast<CReliablePerson*>(
        static_cast<uintptr_t>(trade_gold_total));
    if (trade_gold_total > 0x3B9AC9FFu) {
        const uint32_t total = trade_gold_total;
        trade_gold_total = total % kTradeGoldBillionUnit;
        result = this;
        trade_gold_total_billion += total / kTradeGoldBillionUnit;
    }
    return result;
}

// IDA_EA: 0x8695986
// IDA_NAME: _ZNK15CReliablePerson17GetTradeGoldTotalEv
// DEMANGLED: CReliablePerson::GetTradeGoldTotal(void)const
int CReliablePerson::GetTradeGoldTotal() const {
    return static_cast<int32_t>(trade_gold_total);
}

// IDA_EA: 0x8695990
// IDA_NAME: _ZNK15CReliablePerson24GetTradeGoldTotalBillionEv
// DEMANGLED: CReliablePerson::GetTradeGoldTotalBillion(void)const
int CReliablePerson::GetTradeGoldTotalBillion() const {
    return static_cast<int32_t>(trade_gold_total_billion);
}

// IDA_EA: 0x869599C
// IDA_NAME: _ZNK15CReliablePerson17GetTradeGoldDailyEv
// DEMANGLED: CReliablePerson::GetTradeGoldDaily(void)const
int CReliablePerson::GetTradeGoldDaily() const {
    return static_cast<int32_t>(trade_gold_daily);
}
