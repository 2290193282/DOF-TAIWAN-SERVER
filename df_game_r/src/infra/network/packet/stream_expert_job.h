#ifndef OLDSRV_PROTOCOL_STREAM_EXPERT_JOB_H_
#define OLDSRV_PROTOCOL_STREAM_EXPERT_JOB_H_

#include "old_data/db/quest/expert_job_payload.h"
#include "infra/network/packet/stream_guard.h"

// IDA_EA: 0x832B87E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI14SIG_EXPERT_JOBEEPT_v
// DEMANGLED: SIG_EXPERT_JOB * CStreamGuard::GetInBuffer<SIG_EXPERT_JOB>(void)
template <>
SIG_EXPERT_JOB* CStreamGuard::GetInBuffer<SIG_EXPERT_JOB>();

// IDA_EA: 0x832BA74
// IDA_NAME: _ZN6Stream11GetInBufferI14SIG_EXPERT_JOBEEPT_v
// DEMANGLED: SIG_EXPERT_JOB * Stream::GetInBuffer<SIG_EXPERT_JOB>(void)
template <>
SIG_EXPERT_JOB* Stream::GetInBuffer<SIG_EXPERT_JOB>();

// IDA_EA: 0x8453344
// IDA_NAME: _ZN6Stream12GetOutBufferI14SIG_EXPERT_JOBEEPT_v
// DEMANGLED: SIG_EXPERT_JOB * Stream::GetOutBuffer<SIG_EXPERT_JOB>(void)
template <>
SIG_EXPERT_JOB* Stream::GetOutBuffer<SIG_EXPERT_JOB>();

#endif  // OLDSRV_PROTOCOL_STREAM_EXPERT_JOB_H_
