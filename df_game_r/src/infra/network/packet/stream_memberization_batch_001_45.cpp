#include "infra/network/packet/stream_guard.h"
#include "old_data/db/inventory/db_load_ticket.h"

// IDA_EA: 0x8239870
// IDA_NAME: _ZN6Stream11GetInBufferI15SIG_LOAD_TICKETEEPT_v
// DEMANGLED: SIG_LOAD_TICKET * Stream::GetInBuffer<SIG_LOAD_TICKET>(void)
template <>
SIG_LOAD_TICKET* Stream::GetInBuffer<SIG_LOAD_TICKET>() {
    const int bytes = 412;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_TICKET* result =
        reinterpret_cast<SIG_LOAD_TICKET*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236D72
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI15SIG_LOAD_TICKETEEPT_v
// DEMANGLED: SIG_LOAD_TICKET * CStreamGuard::GetInBuffer<SIG_LOAD_TICKET>(void)
template <>
SIG_LOAD_TICKET* CStreamGuard::GetInBuffer<SIG_LOAD_TICKET>() {
    return stream_->GetInBuffer<SIG_LOAD_TICKET>();
}
