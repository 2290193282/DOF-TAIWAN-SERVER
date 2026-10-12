#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_select_charac_mid.h"

// IDA_EA: 0x8457db2
// IDA_NAME: _ZN6Stream11GetInBufferI25SIG_CHECK_CHAR_MID_RESULTEEPT_v
// DEMANGLED: SIG_CHECK_CHAR_MID_RESULT * Stream::GetInBuffer<SIG_CHECK_CHAR_MID_RESULT>(void)
template <>
SIG_CHECK_CHAR_MID_RESULT* Stream::GetInBuffer<SIG_CHECK_CHAR_MID_RESULT>() {
    const int bytes = 20;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_CHAR_MID_RESULT* result =
        reinterpret_cast<SIG_CHECK_CHAR_MID_RESULT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84542F0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_CHECK_CHAR_MID_RESULTEEPT_v
// DEMANGLED: SIG_CHECK_CHAR_MID_RESULT * CStreamGuard::GetInBuffer<SIG_CHECK_CHAR_MID_RESULT>(void)
template <>
SIG_CHECK_CHAR_MID_RESULT* CStreamGuard::GetInBuffer<SIG_CHECK_CHAR_MID_RESULT>() {
    return stream_->GetInBuffer<SIG_CHECK_CHAR_MID_RESULT>();
}

// IDA_EA: 0x8457e1c
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_CHECK_CHAR_MIDEEPT_v
// DEMANGLED: SIG_CHECK_CHAR_MID * Stream::GetInBuffer<SIG_CHECK_CHAR_MID>(void)
template <>
SIG_CHECK_CHAR_MID* Stream::GetInBuffer<SIG_CHECK_CHAR_MID>() {
    const int bytes = 48;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_CHAR_MID* result =
        reinterpret_cast<SIG_CHECK_CHAR_MID*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454306
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_CHECK_CHAR_MIDEEPT_v
// DEMANGLED: SIG_CHECK_CHAR_MID * CStreamGuard::GetInBuffer<SIG_CHECK_CHAR_MID>(void)
template <>
SIG_CHECK_CHAR_MID* CStreamGuard::GetInBuffer<SIG_CHECK_CHAR_MID>() {
    return stream_->GetInBuffer<SIG_CHECK_CHAR_MID>();
}
