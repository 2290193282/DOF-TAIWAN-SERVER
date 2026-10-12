#include "infra/network/packet/stream_guard.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"

// IDA_EA: 0x823a91a
// IDA_NAME: _ZN6Stream11GetInBufferI29SIG_REQUEST_CHARAC_SKILL_INFOEEPT_v
// DEMANGLED: SIG_REQUEST_CHARAC_SKILL_INFO * Stream::GetInBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>(void)
template <>
SIG_REQUEST_CHARAC_SKILL_INFO* Stream::GetInBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>() {
    const int bytes = 416;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REQUEST_CHARAC_SKILL_INFO* result =
        reinterpret_cast<SIG_REQUEST_CHARAC_SKILL_INFO*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8237438
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_REQUEST_CHARAC_SKILL_INFOEEPT_v
// DEMANGLED: SIG_REQUEST_CHARAC_SKILL_INFO * CStreamGuard::GetInBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>(void)
template <>
SIG_REQUEST_CHARAC_SKILL_INFO* CStreamGuard::GetInBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>() {
    return stream_->GetInBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>();
}
