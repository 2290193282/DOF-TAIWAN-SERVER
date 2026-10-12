#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/query_charac_info_payload.h"

// IDA_EA: 0x8455c02
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_QUERY_CHARAC_INFOEEPT_v
// DEMANGLED: SIG_QUERY_CHARAC_INFO * Stream::GetInBuffer<SIG_QUERY_CHARAC_INFO>(void)
template <>
SIG_QUERY_CHARAC_INFO* Stream::GetInBuffer<SIG_QUERY_CHARAC_INFO>() {
    const int bytes = 48;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_QUERY_CHARAC_INFO* result =
        reinterpret_cast<SIG_QUERY_CHARAC_INFO*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452128
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_QUERY_CHARAC_INFOEEPT_v
// DEMANGLED: SIG_QUERY_CHARAC_INFO * CStreamGuard::GetInBuffer<SIG_QUERY_CHARAC_INFO>(void)
template <>
SIG_QUERY_CHARAC_INFO* CStreamGuard::GetInBuffer<SIG_QUERY_CHARAC_INFO>() {
    return stream_->GetInBuffer<SIG_QUERY_CHARAC_INFO>();
}
