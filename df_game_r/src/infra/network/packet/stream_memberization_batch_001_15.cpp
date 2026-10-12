#include "infra/network/packet/stream_guard.h"
#include "infra/security/policy/service_restrict_manager_abi.h"

// IDA_EA: 0x815f198
// IDA_NAME: _ZN6Stream11GetInBufferIN6Taiwan21SigLoadRestrictPolicyEEEPT_v
// DEMANGLED: Taiwan::SigLoadRestrictPolicy * Stream::GetInBuffer<Taiwan::SigLoadRestrictPolicy>(void)
template <>
Taiwan::SigLoadRestrictPolicy* Stream::GetInBuffer<Taiwan::SigLoadRestrictPolicy>() {
    const int bytes = 4608;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    Taiwan::SigLoadRestrictPolicy* result =
        reinterpret_cast<Taiwan::SigLoadRestrictPolicy*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815E9A2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN6Taiwan21SigLoadRestrictPolicyEEEPT_v
// DEMANGLED: Taiwan::SigLoadRestrictPolicy * CStreamGuard::GetInBuffer<Taiwan::SigLoadRestrictPolicy>(void)
template <>
Taiwan::SigLoadRestrictPolicy* CStreamGuard::GetInBuffer<Taiwan::SigLoadRestrictPolicy>() {
    return stream_->GetInBuffer<Taiwan::SigLoadRestrictPolicy>();
}
