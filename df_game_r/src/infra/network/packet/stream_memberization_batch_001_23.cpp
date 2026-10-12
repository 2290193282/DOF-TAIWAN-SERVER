#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auth_security/db_mouse_register.h"

// IDA_EA: 0x82654d6
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_MOUSE_REGISTEREEPT_v
// DEMANGLED: SIG_MOUSE_REGISTER * Stream::GetInBuffer<SIG_MOUSE_REGISTER>(void)
template <>
SIG_MOUSE_REGISTER* Stream::GetInBuffer<SIG_MOUSE_REGISTER>() {
    const int bytes = 89;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_MOUSE_REGISTER* result =
        reinterpret_cast<SIG_MOUSE_REGISTER*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x82654C0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_MOUSE_REGISTEREEPT_v
// DEMANGLED: SIG_MOUSE_REGISTER * CStreamGuard::GetInBuffer<SIG_MOUSE_REGISTER>(void)
template <>
SIG_MOUSE_REGISTER* CStreamGuard::GetInBuffer<SIG_MOUSE_REGISTER>() {
    return stream_->GetInBuffer<SIG_MOUSE_REGISTER>();
}

// IDA_EA: 0x8455986
// IDA_NAME: _ZN6Stream11GetInBufferI32SIG_GOBLIN_PAD_ADVICE_CHANGE_PWDEEPT_v
// DEMANGLED: SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD * Stream::GetInBuffer<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD>(void)
template <>
SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD* Stream::GetInBuffer<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD* result =
        reinterpret_cast<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451DB0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI32SIG_GOBLIN_PAD_ADVICE_CHANGE_PWDEEPT_v
// DEMANGLED: SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD * CStreamGuard::GetInBuffer<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD>(void)
template <>
SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD* CStreamGuard::GetInBuffer<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD>() {
    return stream_->GetInBuffer<SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD>();
}
