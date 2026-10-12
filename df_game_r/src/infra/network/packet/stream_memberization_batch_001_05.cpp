#include "infra/network/packet/stream_guard.h"
#include "old_data/db/dungeon/best_clear_time_payload.h"

// IDA_EA: 0x80c9032
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_BEST_CLEAR_TIMEEEPT_v
// DEMANGLED: SIG_BEST_CLEAR_TIME * Stream::GetInBuffer<SIG_BEST_CLEAR_TIME>(void)
template <>
SIG_BEST_CLEAR_TIME* Stream::GetInBuffer<SIG_BEST_CLEAR_TIME>() {
    const int bytes = 24008;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_BEST_CLEAR_TIME* result =
        reinterpret_cast<SIG_BEST_CLEAR_TIME*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x80C8D6A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_BEST_CLEAR_TIMEEEPT_v
// DEMANGLED: SIG_BEST_CLEAR_TIME * CStreamGuard::GetInBuffer<SIG_BEST_CLEAR_TIME>(void)
template <>
SIG_BEST_CLEAR_TIME* CStreamGuard::GetInBuffer<SIG_BEST_CLEAR_TIME>() {
    return stream_->GetInBuffer<SIG_BEST_CLEAR_TIME>();
}
