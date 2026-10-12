#include "infra/network/packet/stream_guard.h"
#include "old_data/db/statistics_log/db_insert_unlimit_support_log.h"

// IDA_EA: 0x84566e2
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_INSERT_UNLIMIT_SUPPORT_LOGEEPT_v
// DEMANGLED: SIG_INSERT_UNLIMIT_SUPPORT_LOG * Stream::GetInBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>(void)
template <>
SIG_INSERT_UNLIMIT_SUPPORT_LOG* Stream::GetInBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>() {
    const int bytes = 44;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_INSERT_UNLIMIT_SUPPORT_LOG* result =
        reinterpret_cast<SIG_INSERT_UNLIMIT_SUPPORT_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84529EE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_INSERT_UNLIMIT_SUPPORT_LOGEEPT_v
// DEMANGLED: SIG_INSERT_UNLIMIT_SUPPORT_LOG * CStreamGuard::GetInBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>(void)
template <>
SIG_INSERT_UNLIMIT_SUPPORT_LOG* CStreamGuard::GetInBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>() {
    return stream_->GetInBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>();
}
