#include "common/manager/exchange_server_hit_rate_abi.h"

namespace exchange_server {

// IDA_EA: 0x823465A
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate20IncrementDBLoadCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::IncrementDBLoadCount(void)
char CExchangeServerHitRate::IncrementDBLoadCount() {
    char result = (++db_load_count_ == 0u);
    if (!db_load_count_) {
        result = static_cast<char>(reinterpret_cast<uintptr_t>(this));
        ++db_load_overflow_count_;
    }
    return result;
}

// IDA_EA: 0x823468A
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate23IncrementCacheLoadCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::IncrementCacheLoadCount(void)
char CExchangeServerHitRate::IncrementCacheLoadCount() {
    char result = (++cache_load_count_ == 0u);
    if (!cache_load_count_) {
        result = static_cast<char>(reinterpret_cast<uintptr_t>(this));
        ++cache_load_overflow_count_;
    }
    return result;
}

// IDA_EA: 0x82346BC
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate5ResetEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::Reset(void)
CExchangeServerHitRate* CExchangeServerHitRate::Reset() {
    db_load_count_ = 0;
    cache_load_count_ = 0;
    db_load_overflow_count_ = 0;
    cache_load_overflow_count_ = 0;
    cache_load_fail_count_ = 0;
    cache_load_fail_overflow_count_ = 0;
    return this;
}

// IDA_EA: 0x82346FC
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRateC2Ev
// DEMANGLED: exchange_server::CExchangeServerHitRate::CExchangeServerHitRate(void)
CExchangeServerHitRate::CExchangeServerHitRate() {
    Reset();
}

// IDA_EA: 0x823470F
// IDA_NAME: _ZN15exchange_server32GetInstanceExchangeServerHitRateEv
// DEMANGLED: exchange_server::GetInstanceExchangeServerHitRate(void)
CExchangeServerHitRate* GetInstanceExchangeServerHitRate() {
    static CExchangeServerHitRate instance;
    return &instance;
}

// IDA_EA: 0x848F35E
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate27IncrementCacheLoadFailCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::IncrementCacheLoadFailCount(void)
char CExchangeServerHitRate::IncrementCacheLoadFailCount() {
    char result = (++cache_load_fail_count_ == 0u);
    if (!cache_load_fail_count_) {
        result = static_cast<char>(reinterpret_cast<uintptr_t>(this));
        ++cache_load_fail_overflow_count_;
    }
    return result;
}

// IDA_EA: 0x863BFB0
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate14GetDBLoadCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::GetDBLoadCount(void)
int CExchangeServerHitRate::GetDBLoadCount() {
    return static_cast<int>(db_load_count_);
}

// IDA_EA: 0x863BFBA
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate17GetCacheLoadCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::GetCacheLoadCount(void)
int CExchangeServerHitRate::GetCacheLoadCount() {
    return static_cast<int>(cache_load_count_);
}

// IDA_EA: 0x863BFC6
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate22GetDBLoadOverflowCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::GetDBLoadOverflowCount(void)
int CExchangeServerHitRate::GetDBLoadOverflowCount() {
    return static_cast<int>(db_load_overflow_count_);
}

// IDA_EA: 0x863BFD2
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate25GetCacheLoadOverflowCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::GetCacheLoadOverflowCount(void)
int CExchangeServerHitRate::GetCacheLoadOverflowCount() {
    return static_cast<int>(cache_load_overflow_count_);
}

// IDA_EA: 0x863BFDE
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate21GetCacheLoadFailCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::GetCacheLoadFailCount(void)
int CExchangeServerHitRate::GetCacheLoadFailCount() {
    return static_cast<int>(cache_load_fail_count_);
}

// IDA_EA: 0x863BFEA
// IDA_NAME: _ZN15exchange_server22CExchangeServerHitRate29GetCacheLoadFailOverflowCountEv
// DEMANGLED: exchange_server::CExchangeServerHitRate::GetCacheLoadFailOverflowCount(void)
int CExchangeServerHitRate::GetCacheLoadFailOverflowCount() {
    return static_cast<int>(cache_load_fail_overflow_count_);
}

}  // namespace exchange_server