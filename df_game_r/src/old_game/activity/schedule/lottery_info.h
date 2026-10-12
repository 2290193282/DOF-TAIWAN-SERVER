#ifndef OLDSRV_OLD_GAME_EVENT_LOTTERY_INFO_H_
#define OLDSRV_OLD_GAME_EVENT_LOTTERY_INFO_H_

#include <stdint.h>

#include <utility>
#include <vector>

// IDA unit: 11_event_runtime/LotteryInfo.  Area owns this real value
// subobject at +0x24, while the event runtime owns its lifecycle bodies.
// Keeping the declaration independent of event_value_types.h prevents the
// world/private-store header cycle from turning that subobject into a facade.
struct LotteryInfo {
    LotteryInfo();
    ~LotteryInfo();

    LotteryInfo* clear();

    uint32_t first_dword;
    std::vector<std::pair<int, int> > pair_values;
    uint32_t second_dword;
    std::vector<int> int_values;
};

#if defined(__i386__)
static_assert(sizeof(LotteryInfo) == 0x20u,
              "LotteryInfo must retain its IDA i386 layout");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_LOTTERY_INFO_H_
