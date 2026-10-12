#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_get_user_regdate.h"

// IDA_EA: 0x845791a
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_GET_USER_REGDATEEEPT_v
// DEMANGLED: SIG_GET_USER_REGDATE * Stream::GetInBuffer<SIG_GET_USER_REGDATE>(void)
template <>
SIG_GET_USER_REGDATE* Stream::GetInBuffer<SIG_GET_USER_REGDATE>() {
    const int bytes = 12;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_GET_USER_REGDATE* result =
        reinterpret_cast<SIG_GET_USER_REGDATE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453E72
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_GET_USER_REGDATEEEPT_v
// DEMANGLED: SIG_GET_USER_REGDATE * CStreamGuard::GetInBuffer<SIG_GET_USER_REGDATE>(void)
template <>
SIG_GET_USER_REGDATE* CStreamGuard::GetInBuffer<SIG_GET_USER_REGDATE>() {
    return stream_->GetInBuffer<SIG_GET_USER_REGDATE>();
}
