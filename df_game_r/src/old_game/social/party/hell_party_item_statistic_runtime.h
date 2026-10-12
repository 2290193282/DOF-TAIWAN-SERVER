#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_HELL_PARTY_ITEM_STATISTIC_RUNTIME_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_HELL_PARTY_ITEM_STATISTIC_RUNTIME_H_

#include <stdint.h>

// The old singleton object at 0x0947334c is a one-byte, fieldless instance.
// Its only emitted behavior is the statistic-packet sender below.
class CHellPartyItemStatistic {
public:
    // IDA_EA: 0x860FCC0
    // IDA_NAME: _ZN23CHellPartyItemStatistic26SendHellPartyItemStatisticEbiiicPi
    // DEMANGLED: CHellPartyItemStatistic::SendHellPartyItemStatistic(
    //     bool,int,int,int,char,int *)
    int SendHellPartyItemStatistic(bool active_flag,
                                   int dungeon_index,
                                   int dungeon_difficulty,
                                   int kill_data_value,
                                   char statistics_enabled,
                                   int* item_rarity_counts);
};

static_assert(sizeof(CHellPartyItemStatistic) == 1u,
              "Hell Party statistic singleton must retain the old one-byte ABI");

// IDA_EA: 0x860FCB6
// IDA_NAME: _Z33GetInstanceHellPartyItemStatisticv
// DEMANGLED: GetInstanceHellPartyItemStatistic(void)
CHellPartyItemStatistic* GetInstanceHellPartyItemStatistic();

#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_HELL_PARTY_ITEM_STATISTIC_RUNTIME_H_
