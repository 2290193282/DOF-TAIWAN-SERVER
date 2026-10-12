#include "infra/network/packet/stream_guard.h"
#include "old_data/db/skill/db_update_one_day_lethe_skill.h"

// IDA_EA: 0x8456bda
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_PREMIUM_LETHE_FLAGEEPT_v
// DEMANGLED: SIG_PREMIUM_LETHE_FLAG * Stream::GetInBuffer<SIG_PREMIUM_LETHE_FLAG>(void)
template <>
SIG_PREMIUM_LETHE_FLAG* Stream::GetInBuffer<SIG_PREMIUM_LETHE_FLAG>() {
    const int bytes = 416;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_PREMIUM_LETHE_FLAG* result =
        reinterpret_cast<SIG_PREMIUM_LETHE_FLAG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453132
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_PREMIUM_LETHE_FLAGEEPT_v
// DEMANGLED: SIG_PREMIUM_LETHE_FLAG * CStreamGuard::GetInBuffer<SIG_PREMIUM_LETHE_FLAG>(void)
template <>
SIG_PREMIUM_LETHE_FLAG* CStreamGuard::GetInBuffer<SIG_PREMIUM_LETHE_FLAG>() {
    return stream_->GetInBuffer<SIG_PREMIUM_LETHE_FLAG>();
}

// IDA_EA: 0x84f2db2
// IDA_NAME: _ZN6Stream11GetInBufferI35SIG_RESULT_PREMIUM_LETHE_SKILL_LOADEEPT_v
// DEMANGLED: SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD * Stream::GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>(void)
template <>
SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* Stream::GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>() {
    const int bytes = 424;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* result =
        reinterpret_cast<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84F165C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI35SIG_RESULT_PREMIUM_LETHE_SKILL_LOADEEPT_v
// DEMANGLED: SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD * CStreamGuard::GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>(void)
template <>
SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* CStreamGuard::GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>() {
    return stream_->GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>();
}
