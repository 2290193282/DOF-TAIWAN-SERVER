#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/tag_charac_info_payload.h"

// IDA_EA: 0x823a984
// IDA_NAME: _ZN6Stream11GetInBufferI27SIG_REQUEST_TAG_CHARAC_INFOEEPT_v
// DEMANGLED: SIG_REQUEST_TAG_CHARAC_INFO * Stream::GetInBuffer<SIG_REQUEST_TAG_CHARAC_INFO>(void)
template <>
SIG_REQUEST_TAG_CHARAC_INFO* Stream::GetInBuffer<SIG_REQUEST_TAG_CHARAC_INFO>() {
    const int bytes = sizeof(SIG_REQUEST_TAG_CHARAC_INFO);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REQUEST_TAG_CHARAC_INFO* result =
        reinterpret_cast<SIG_REQUEST_TAG_CHARAC_INFO*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823744E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI27SIG_REQUEST_TAG_CHARAC_INFOEEPT_v
// DEMANGLED: SIG_REQUEST_TAG_CHARAC_INFO * CStreamGuard::GetInBuffer<SIG_REQUEST_TAG_CHARAC_INFO>(void)
template <>
SIG_REQUEST_TAG_CHARAC_INFO* CStreamGuard::GetInBuffer<SIG_REQUEST_TAG_CHARAC_INFO>() {
    return stream_->GetInBuffer<SIG_REQUEST_TAG_CHARAC_INFO>();
}
