#include "infra/network/packet/stream_charac_inform_notice.h"

// IDA_EA: 0x80E4DBE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_SAVE_CHARAC_INFORM_NOTICEEEPT_v
// DEMANGLED: SIG_SAVE_CHARAC_INFORM_NOTICE * CStreamGuard::GetInBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>(void)
template <>
SIG_SAVE_CHARAC_INFORM_NOTICE*
CStreamGuard::GetInBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>() {
    return stream_->GetInBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>();
}

// IDA_EA: 0x80E4F2C
// IDA_NAME: _ZN6Stream11GetInBufferI29SIG_SAVE_CHARAC_INFORM_NOTICEEEPT_v
// DEMANGLED: SIG_SAVE_CHARAC_INFORM_NOTICE * Stream::GetInBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>(void)
template <>
SIG_SAVE_CHARAC_INFORM_NOTICE*
Stream::GetInBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>() {
    const int bytes = sizeof(SIG_SAVE_CHARAC_INFORM_NOTICE);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_SAVE_CHARAC_INFORM_NOTICE* const result =
        reinterpret_cast<SIG_SAVE_CHARAC_INFORM_NOTICE*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454876
// IDA_NAME: _ZN6Stream12GetOutBufferI29SIG_SAVE_CHARAC_INFORM_NOTICEEEPT_v
// DEMANGLED: SIG_SAVE_CHARAC_INFORM_NOTICE * Stream::GetOutBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>(void)
template <>
SIG_SAVE_CHARAC_INFORM_NOTICE*
Stream::GetOutBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>() {
    const int bytes = sizeof(SIG_SAVE_CHARAC_INFORM_NOTICE);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_SAVE_CHARAC_INFORM_NOTICE* const result =
        reinterpret_cast<SIG_SAVE_CHARAC_INFORM_NOTICE*>(
            buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
