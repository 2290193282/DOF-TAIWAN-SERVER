#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_delete_charac.h"

// IDA_EA: 0x82395e8
// IDA_NAME: _ZN6Stream11GetInBufferI17SIG_DELETE_CHARACEEPT_v
// DEMANGLED: SIG_DELETE_CHARAC * Stream::GetInBuffer<SIG_DELETE_CHARAC>(void)
template <>
SIG_DELETE_CHARAC* Stream::GetInBuffer<SIG_DELETE_CHARAC>() {
    const int bytes = 20;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_DELETE_CHARAC* result =
        reinterpret_cast<SIG_DELETE_CHARAC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236CE2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_DELETE_CHARACEEPT_v
// DEMANGLED: SIG_DELETE_CHARAC * CStreamGuard::GetInBuffer<SIG_DELETE_CHARAC>(void)
template <>
SIG_DELETE_CHARAC* CStreamGuard::GetInBuffer<SIG_DELETE_CHARAC>() {
    return stream_->GetInBuffer<SIG_DELETE_CHARAC>();
}
