#ifndef OLDSRV_GLOBAL_EXCHANGE_SERVER_HIT_RATE_ABI_H_
#define OLDSRV_GLOBAL_EXCHANGE_SERVER_HIT_RATE_ABI_H_

#include <stdint.h>

namespace exchange_server {

// IDA unit: 99_misc_global/exchange_server/CExchangeServerHitRate.
// The exchange server session/cache bodies remain outside this public counter ABI.
class CExchangeServerHitRate {
public:
    CExchangeServerHitRate();

    char IncrementDBLoadCount();
    char IncrementCacheLoadCount();
    CExchangeServerHitRate* Reset();
    char IncrementCacheLoadFailCount();

    int GetDBLoadCount();
    int GetCacheLoadCount();
    int GetDBLoadOverflowCount();
    int GetCacheLoadOverflowCount();
    int GetCacheLoadFailCount();
    int GetCacheLoadFailOverflowCount();

private:
    uint32_t db_load_count_;
    uint32_t cache_load_count_;
    uint32_t db_load_overflow_count_;
    uint32_t cache_load_overflow_count_;
    uint32_t cache_load_fail_count_;
    uint32_t cache_load_fail_overflow_count_;
};

CExchangeServerHitRate* GetInstanceExchangeServerHitRate();

}  // namespace exchange_server

static_assert(sizeof(exchange_server::CExchangeServerHitRate) == 0x18u,
              "CExchangeServerHitRate IDA ABI size must stay 0x18");

#endif  // OLDSRV_GLOBAL_EXCHANGE_SERVER_HIT_RATE_ABI_H_