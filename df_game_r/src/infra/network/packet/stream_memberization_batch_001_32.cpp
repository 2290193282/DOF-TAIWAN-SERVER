#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_save_charac.h"

// IDA_EA: 0x869f498
// IDA_NAME: _ZN6Stream11GetInBufferI15SIG_SAVE_CHARACEEPT_v
// DEMANGLED: SIG_SAVE_CHARAC * Stream::GetInBuffer<SIG_SAVE_CHARAC>(void)
template <>
SIG_SAVE_CHARAC* Stream::GetInBuffer<SIG_SAVE_CHARAC>() {
    const int bytes = sizeof(SIG_SAVE_CHARAC);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_CHARAC* result =
        reinterpret_cast<SIG_SAVE_CHARAC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869925C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI15SIG_SAVE_CHARACEEPT_v
// DEMANGLED: SIG_SAVE_CHARAC * CStreamGuard::GetInBuffer<SIG_SAVE_CHARAC>(void)
template <>
SIG_SAVE_CHARAC* CStreamGuard::GetInBuffer<SIG_SAVE_CHARAC>() {
    return stream_->GetInBuffer<SIG_SAVE_CHARAC>();
}
