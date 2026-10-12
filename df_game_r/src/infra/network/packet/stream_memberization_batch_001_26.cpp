#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_check_charac_name.h"

// IDA_EA: 0x8457984
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_GOLD_CONTROL_EXCEPTION_MIDEEPT_v
// DEMANGLED: SIG_GOLD_CONTROL_EXCEPTION_MID * Stream::GetInBuffer<SIG_GOLD_CONTROL_EXCEPTION_MID>(void)
template <>
SIG_GOLD_CONTROL_EXCEPTION_MID* Stream::GetInBuffer<SIG_GOLD_CONTROL_EXCEPTION_MID>() {
    const int bytes = 2;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_GOLD_CONTROL_EXCEPTION_MID* result =
        reinterpret_cast<SIG_GOLD_CONTROL_EXCEPTION_MID*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453EDC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_GOLD_CONTROL_EXCEPTION_MIDEEPT_v
// DEMANGLED: SIG_GOLD_CONTROL_EXCEPTION_MID * CStreamGuard::GetInBuffer<SIG_GOLD_CONTROL_EXCEPTION_MID>(void)
template <>
SIG_GOLD_CONTROL_EXCEPTION_MID* CStreamGuard::GetInBuffer<SIG_GOLD_CONTROL_EXCEPTION_MID>() {
    return stream_->GetInBuffer<SIG_GOLD_CONTROL_EXCEPTION_MID>();
}

// IDA_EA: 0x84579ee
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_CHECK_CHARAC_NAMEEEPT_v
// DEMANGLED: SIG_CHECK_CHARAC_NAME * Stream::GetInBuffer<SIG_CHECK_CHARAC_NAME>(void)
template <>
SIG_CHECK_CHARAC_NAME* Stream::GetInBuffer<SIG_CHECK_CHARAC_NAME>() {
    const int bytes = 36;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_CHARAC_NAME* result =
        reinterpret_cast<SIG_CHECK_CHARAC_NAME*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453EF2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_CHECK_CHARAC_NAMEEEPT_v
// DEMANGLED: SIG_CHECK_CHARAC_NAME * CStreamGuard::GetInBuffer<SIG_CHECK_CHARAC_NAME>(void)
template <>
SIG_CHECK_CHARAC_NAME* CStreamGuard::GetInBuffer<SIG_CHECK_CHARAC_NAME>() {
    return stream_->GetInBuffer<SIG_CHECK_CHARAC_NAME>();
}
