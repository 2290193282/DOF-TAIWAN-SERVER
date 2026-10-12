#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/load_charac_payload.h"

// IDA_EA: 0x82396c8
// IDA_NAME: _ZN6Stream11GetInBufferI15SIG_LOAD_CHARACEEPT_v
// DEMANGLED: SIG_LOAD_CHARAC * Stream::GetInBuffer<SIG_LOAD_CHARAC>(void)
template <>
SIG_LOAD_CHARAC* Stream::GetInBuffer<SIG_LOAD_CHARAC>() {
    const int bytes = sizeof(SIG_LOAD_CHARAC);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_CHARAC* result =
        reinterpret_cast<SIG_LOAD_CHARAC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236D1A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI15SIG_LOAD_CHARACEEPT_v
// DEMANGLED: SIG_LOAD_CHARAC * CStreamGuard::GetInBuffer<SIG_LOAD_CHARAC>(void)
template <>
SIG_LOAD_CHARAC* CStreamGuard::GetInBuffer<SIG_LOAD_CHARAC>() {
    return stream_->GetInBuffer<SIG_LOAD_CHARAC>();
}
