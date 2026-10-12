#ifndef OLDSRV_GLOBAL_RELIABLE_PERSON_H_
#define OLDSRV_GLOBAL_RELIABLE_PERSON_H_

#include <stdint.h>

// IDA unit: 99_misc_global/CReliablePerson. The old symbols live in the
// IDA_EA: InitTradeGold @0x86453B8, IncrementTradeGold @0x86453D8;
// DEFAULT_MAX_FATIGUE data @0x9370F5C.
// global namespace; oldsrv imports them below for active module convenience.
extern int DEFAULT_MAX_FATIGUE __asm__("DEFAULT_MAX_FATIGUE");

struct CReliablePerson {
    CReliablePerson* SetTradeGoldDaily(uint32_t trade_gold_daily);
    CReliablePerson* InitTradeGold(uint32_t total,
                                   uint32_t total_billion,
                                   uint32_t daily);
    CReliablePerson* IncrementTradeGold(uint32_t amount);
    int GetTradeGoldTotal() const;
    int GetTradeGoldTotalBillion() const;
    int GetTradeGoldDaily() const;

    uint32_t trade_gold_total;
    uint32_t trade_gold_total_billion;
    uint32_t trade_gold_daily;
};


using ::CReliablePerson;
using ::DEFAULT_MAX_FATIGUE;


static_assert(sizeof(CReliablePerson) == 0x0Cu,
              "CReliablePerson IDA ABI size must stay 0x0c");

#endif  // OLDSRV_GLOBAL_RELIABLE_PERSON_H_
