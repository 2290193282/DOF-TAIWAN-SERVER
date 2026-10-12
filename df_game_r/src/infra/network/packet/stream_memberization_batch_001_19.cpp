#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/logout_payload.h"

// IDA_EA: 0x86a0108
// IDA_NAME: _ZN6Stream11GetInBufferI17SIG_UPDATE_LOGOUTEEPT_v
// DEMANGLED: SIG_UPDATE_LOGOUT * Stream::GetInBuffer<SIG_UPDATE_LOGOUT>(void)
template <>
SIG_UPDATE_LOGOUT* Stream::GetInBuffer<SIG_UPDATE_LOGOUT>() {
    const int bytes = 139;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_LOGOUT* result =
        reinterpret_cast<SIG_UPDATE_LOGOUT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869AC86
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_UPDATE_LOGOUTEEPT_v
// DEMANGLED: SIG_UPDATE_LOGOUT * CStreamGuard::GetInBuffer<SIG_UPDATE_LOGOUT>(void)
template <>
SIG_UPDATE_LOGOUT* CStreamGuard::GetInBuffer<SIG_UPDATE_LOGOUT>() {
    return stream_->GetInBuffer<SIG_UPDATE_LOGOUT>();
}
