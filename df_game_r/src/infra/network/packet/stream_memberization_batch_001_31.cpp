#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_req_change_charac_name.h"

// IDA_EA: 0x8455b98
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_CHANGE_CHARAC_NAMEEEPT_v
// DEMANGLED: SIG_CHANGE_CHARAC_NAME * Stream::GetInBuffer<SIG_CHANGE_CHARAC_NAME>(void)
template <>
SIG_CHANGE_CHARAC_NAME* Stream::GetInBuffer<SIG_CHANGE_CHARAC_NAME>() {
    const int bytes = 48;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHANGE_CHARAC_NAME* result =
        reinterpret_cast<SIG_CHANGE_CHARAC_NAME*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84520BE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_CHANGE_CHARAC_NAMEEEPT_v
// DEMANGLED: SIG_CHANGE_CHARAC_NAME * CStreamGuard::GetInBuffer<SIG_CHANGE_CHARAC_NAME>(void)
template <>
SIG_CHANGE_CHARAC_NAME* CStreamGuard::GetInBuffer<SIG_CHANGE_CHARAC_NAME>() {
    return stream_->GetInBuffer<SIG_CHANGE_CHARAC_NAME>();
}
