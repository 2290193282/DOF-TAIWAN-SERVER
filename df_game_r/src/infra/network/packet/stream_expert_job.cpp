#include "infra/network/packet/stream_expert_job.h"

// IDA_EA: 0x832B87E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI14SIG_EXPERT_JOBEEPT_v
// DEMANGLED: SIG_EXPERT_JOB * CStreamGuard::GetInBuffer<SIG_EXPERT_JOB>(void)
template <>
SIG_EXPERT_JOB* CStreamGuard::GetInBuffer<SIG_EXPERT_JOB>() {
    return stream_->GetInBuffer<SIG_EXPERT_JOB>();
}

// IDA_EA: 0x832BA74
// IDA_NAME: _ZN6Stream11GetInBufferI14SIG_EXPERT_JOBEEPT_v
// DEMANGLED: SIG_EXPERT_JOB * Stream::GetInBuffer<SIG_EXPERT_JOB>(void)
template <>
SIG_EXPERT_JOB* Stream::GetInBuffer<SIG_EXPERT_JOB>() {
    const int bytes = sizeof(SIG_EXPERT_JOB);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_EXPERT_JOB* const payload =
        reinterpret_cast<SIG_EXPERT_JOB*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return payload;
}

// IDA_EA: 0x8453344
// IDA_NAME: _ZN6Stream12GetOutBufferI14SIG_EXPERT_JOBEEPT_v
// DEMANGLED: SIG_EXPERT_JOB * Stream::GetOutBuffer<SIG_EXPERT_JOB>(void)
template <>
SIG_EXPERT_JOB* Stream::GetOutBuffer<SIG_EXPERT_JOB>() {
    const int bytes = sizeof(SIG_EXPERT_JOB);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_EXPERT_JOB* const payload =
        reinterpret_cast<SIG_EXPERT_JOB*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return payload;
}
