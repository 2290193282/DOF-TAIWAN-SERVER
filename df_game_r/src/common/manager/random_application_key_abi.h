#ifndef OLDSRV_GLOBAL_RANDOM_APPLICATION_KEY_ABI_H_
#define OLDSRV_GLOBAL_RANDOM_APPLICATION_KEY_ABI_H_

#include <stdint.h>

// IDA unit: 05_world_enter_village/RandomApplicationKey.
// Bodies remain in the C05 world-value source.
struct RandomApplicationKey {
    RandomApplicationKey();
    RandomApplicationKey(uint8_t rarity_value, uint8_t grade_value);

    bool operator<(const RandomApplicationKey& rhs) const;

    uint8_t rarity;
    uint8_t grade;
};

static_assert(sizeof(RandomApplicationKey) == 0x02u,
              "RandomApplicationKey IDA ABI size must stay 0x02");

#endif  // OLDSRV_GLOBAL_RANDOM_APPLICATION_KEY_ABI_H_
