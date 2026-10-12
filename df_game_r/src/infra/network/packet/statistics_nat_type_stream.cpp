#include "old_data/db/core/nat_type_payload.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"


// IDA_EA: 0x8612986
// IDA_NAME: _ZN6Stream11GetInBufferI12SIG_NAT_TYPEEEPT_v
// DEMANGLED: SIG_NAT_TYPE * Stream::GetInBuffer<SIG_NAT_TYPE>(void)
template <>
SIG_NAT_TYPE* Stream::GetInBuffer<SIG_NAT_TYPE>() {
    const int bytes = sizeof(SIG_NAT_TYPE);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_NAT_TYPE* const result =
        reinterpret_cast<SIG_NAT_TYPE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8610F6E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI12SIG_NAT_TYPEEEPT_v
// DEMANGLED: SIG_NAT_TYPE * CStreamGuard::GetInBuffer<SIG_NAT_TYPE>(void)
template <>
SIG_NAT_TYPE* CStreamGuard::GetInBuffer<SIG_NAT_TYPE>() {
    return stream_->GetInBuffer<SIG_NAT_TYPE>();
}
