#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"

#include "old_data/db/auth_security/save_proxy_log_payload.h"

// IDA_EA: 0x823AEB2
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_SAVE_PROXY_LOGEEPT_v
// DEMANGLED: SIG_SAVE_PROXY_LOG * Stream::GetInBuffer<SIG_SAVE_PROXY_LOG>(void)
template <>
SIG_SAVE_PROXY_LOG* Stream::GetInBuffer<SIG_SAVE_PROXY_LOG>() {
    const int bytes = sizeof(SIG_SAVE_PROXY_LOG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_PROXY_LOG* result =
        reinterpret_cast<SIG_SAVE_PROXY_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x82378C2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_SAVE_PROXY_LOGEEPT_v
// DEMANGLED: SIG_SAVE_PROXY_LOG * CStreamGuard::GetInBuffer<SIG_SAVE_PROXY_LOG>(void)
template <>
SIG_SAVE_PROXY_LOG* CStreamGuard::GetInBuffer<SIG_SAVE_PROXY_LOG>() {
    return stream_->GetInBuffer<SIG_SAVE_PROXY_LOG>();
}
