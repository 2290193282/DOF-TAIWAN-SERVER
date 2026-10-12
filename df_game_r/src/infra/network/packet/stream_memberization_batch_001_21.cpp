#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auth_security/db_cancel_resting_user_restrict.h"

// IDA_EA: 0x8456ec0
// IDA_NAME: _ZN6Stream11GetInBufferI25SIG_RESTING_USER_RESTRICTEEPT_v
// DEMANGLED: SIG_RESTING_USER_RESTRICT * Stream::GetInBuffer<SIG_RESTING_USER_RESTRICT>(void)
template <>
SIG_RESTING_USER_RESTRICT* Stream::GetInBuffer<SIG_RESTING_USER_RESTRICT>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_RESTING_USER_RESTRICT* result =
        reinterpret_cast<SIG_RESTING_USER_RESTRICT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453514
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_RESTING_USER_RESTRICTEEPT_v
// DEMANGLED: SIG_RESTING_USER_RESTRICT * CStreamGuard::GetInBuffer<SIG_RESTING_USER_RESTRICT>(void)
template <>
SIG_RESTING_USER_RESTRICT* CStreamGuard::GetInBuffer<SIG_RESTING_USER_RESTRICT>() {
    return stream_->GetInBuffer<SIG_RESTING_USER_RESTRICT>();
}
