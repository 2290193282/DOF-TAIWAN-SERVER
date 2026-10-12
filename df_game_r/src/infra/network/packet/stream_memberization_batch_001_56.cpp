#include "infra/network/packet/stream_guard.h"
#include "old_data/db/statistics_log/statistics_log_payloads.h"

// IDA_EA: 0x860071e
// IDA_NAME: _ZN6Stream11GetInBufferI32SIG_INSERT_SERVER_LOAD_STATISTICEEPT_v
// DEMANGLED: SIG_INSERT_SERVER_LOAD_STATISTIC * Stream::GetInBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>(void)
template <>
SIG_INSERT_SERVER_LOAD_STATISTIC* Stream::GetInBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>() {
    const int bytes = 10;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_INSERT_SERVER_LOAD_STATISTIC* result =
        reinterpret_cast<SIG_INSERT_SERVER_LOAD_STATISTIC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8600708
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI32SIG_INSERT_SERVER_LOAD_STATISTICEEPT_v
// DEMANGLED: SIG_INSERT_SERVER_LOAD_STATISTIC * CStreamGuard::GetInBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>(void)
template <>
SIG_INSERT_SERVER_LOAD_STATISTIC* CStreamGuard::GetInBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>() {
    return stream_->GetInBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>();
}
