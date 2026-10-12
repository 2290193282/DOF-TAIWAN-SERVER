#ifndef OLDSRV_GLOBAL_ADVANCE_ALTAR_CONFIG_PARAMETER_H_
#define OLDSRV_GLOBAL_ADVANCE_ALTAR_CONFIG_PARAMETER_H_

#include <stddef.h>
#include <stdint.h>

namespace advancealtar {

// IDA unit: 99_misc_global/advancealtar/ConfigParameter.
// IDA_EA: ctor @0x889B662, dtor @0x889B6C6, import @0x889B6CC.
class ConfigParameter {
public:
    ConfigParameter();
    ~ConfigParameter();

    int importConfigParameter(const char* filename);

    int32_t daily_refill_ticket;          // +0x00
    float gage_upgrade_cost_percent;      // +0x04
    int32_t gage_up_rates[4];             // +0x08
    uint32_t gage_up_rates_ptr;           // +0x18, old i386 pointer to +0x08
    int32_t gage_upgrade_costs[4];        // +0x1c
};

static_assert(sizeof(ConfigParameter) == 0x2Cu,
              "advancealtar::ConfigParameter IDA size must stay 0x2C");
static_assert(offsetof(ConfigParameter, gage_up_rates) == 0x08u,
              "ConfigParameter gage_up_rates offset must stay 0x08");
static_assert(offsetof(ConfigParameter, gage_up_rates_ptr) == 0x18u,
              "ConfigParameter pointer offset must stay 0x18");
static_assert(offsetof(ConfigParameter, gage_upgrade_costs) == 0x1cu,
              "ConfigParameter gage_upgrade_costs offset must stay 0x1c");

}  // namespace advancealtar

#endif  // OLDSRV_GLOBAL_ADVANCE_ALTAR_CONFIG_PARAMETER_H_
