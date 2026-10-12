#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auth_security/db_check_secu_reward.h"

// IDA_EA: 0x8457a58
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_SEND_USER_REWARDEEPT_v
// DEMANGLED: SIG_SEND_USER_REWARD * Stream::GetInBuffer<SIG_SEND_USER_REWARD>(void)
template <>
SIG_SEND_USER_REWARD* Stream::GetInBuffer<SIG_SEND_USER_REWARD>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SEND_USER_REWARD* result =
        reinterpret_cast<SIG_SEND_USER_REWARD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454058
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_SEND_USER_REWARDEEPT_v
// DEMANGLED: SIG_SEND_USER_REWARD * CStreamGuard::GetInBuffer<SIG_SEND_USER_REWARD>(void)
template <>
SIG_SEND_USER_REWARD* CStreamGuard::GetInBuffer<SIG_SEND_USER_REWARD>() {
    return stream_->GetInBuffer<SIG_SEND_USER_REWARD>();
}

// IDA_EA: 0x8457ac2
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_CHECK_USER_SECU_REWARDEEPT_v
// DEMANGLED: SIG_CHECK_USER_SECU_REWARD * Stream::GetInBuffer<SIG_CHECK_USER_SECU_REWARD>(void)
template <>
SIG_CHECK_USER_SECU_REWARD* Stream::GetInBuffer<SIG_CHECK_USER_SECU_REWARD>() {
    const int bytes = 8;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_USER_SECU_REWARD* result =
        reinterpret_cast<SIG_CHECK_USER_SECU_REWARD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845406E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_CHECK_USER_SECU_REWARDEEPT_v
// DEMANGLED: SIG_CHECK_USER_SECU_REWARD * CStreamGuard::GetInBuffer<SIG_CHECK_USER_SECU_REWARD>(void)
template <>
SIG_CHECK_USER_SECU_REWARD* CStreamGuard::GetInBuffer<SIG_CHECK_USER_SECU_REWARD>() {
    return stream_->GetInBuffer<SIG_CHECK_USER_SECU_REWARD>();
}
