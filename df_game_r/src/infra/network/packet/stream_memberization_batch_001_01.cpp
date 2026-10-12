#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/create_charac_payload.h"

// IDA_EA: 0x823957e
// IDA_NAME: _ZN6Stream11GetInBufferI17SIG_CREATE_CHARACEEPT_v
// DEMANGLED: SIG_CREATE_CHARAC * Stream::GetInBuffer<SIG_CREATE_CHARAC>(void)
template <>
SIG_CREATE_CHARAC* Stream::GetInBuffer<SIG_CREATE_CHARAC>() {
    const int bytes = sizeof(SIG_CREATE_CHARAC);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CREATE_CHARAC* result =
        reinterpret_cast<SIG_CREATE_CHARAC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236CCC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_CREATE_CHARACEEPT_v
// DEMANGLED: SIG_CREATE_CHARAC * CStreamGuard::GetInBuffer<SIG_CREATE_CHARAC>(void)
template <>
SIG_CREATE_CHARAC* CStreamGuard::GetInBuffer<SIG_CREATE_CHARAC>() {
    return stream_->GetInBuffer<SIG_CREATE_CHARAC>();
}
