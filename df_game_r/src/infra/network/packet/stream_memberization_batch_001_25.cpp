#include "infra/network/packet/stream_guard.h"
#include "old_data/db/avatar_aura/avatar_item_payload.h"

// IDA_EA: 0x8454d5e
// IDA_NAME: _ZN6Stream11GetInBufferI23SIG_AVATAR_CHANGE_OWNEREEPT_v
// DEMANGLED: SIG_AVATAR_CHANGE_OWNER * Stream::GetInBuffer<SIG_AVATAR_CHANGE_OWNER>(void)
template <>
SIG_AVATAR_CHANGE_OWNER* Stream::GetInBuffer<SIG_AVATAR_CHANGE_OWNER>() {
    const int bytes = 13;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AVATAR_CHANGE_OWNER* result =
        reinterpret_cast<SIG_AVATAR_CHANGE_OWNER*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845046C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_AVATAR_CHANGE_OWNEREEPT_v
// DEMANGLED: SIG_AVATAR_CHANGE_OWNER * CStreamGuard::GetInBuffer<SIG_AVATAR_CHANGE_OWNER>(void)
template <>
SIG_AVATAR_CHANGE_OWNER* CStreamGuard::GetInBuffer<SIG_AVATAR_CHANGE_OWNER>() {
    return stream_->GetInBuffer<SIG_AVATAR_CHANGE_OWNER>();
}

// IDA_EA: 0x8454dc8
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_AVATAR_CHANGE_STATEEPT_v
// DEMANGLED: SIG_AVATAR_CHANGE_STAT * Stream::GetInBuffer<SIG_AVATAR_CHANGE_STAT>(void)
template <>
SIG_AVATAR_CHANGE_STAT* Stream::GetInBuffer<SIG_AVATAR_CHANGE_STAT>() {
    const int bytes = 81;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AVATAR_CHANGE_STAT* result =
        reinterpret_cast<SIG_AVATAR_CHANGE_STAT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84504D6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_AVATAR_CHANGE_STATEEPT_v
// DEMANGLED: SIG_AVATAR_CHANGE_STAT * CStreamGuard::GetInBuffer<SIG_AVATAR_CHANGE_STAT>(void)
template <>
SIG_AVATAR_CHANGE_STAT* CStreamGuard::GetInBuffer<SIG_AVATAR_CHANGE_STAT>() {
    return stream_->GetInBuffer<SIG_AVATAR_CHANGE_STAT>();
}

// IDA_EA: 0x845695e
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_UPDATE_AVATAR_JEWEL_SOCKETEEPT_v
// DEMANGLED: SIG_UPDATE_AVATAR_JEWEL_SOCKET * Stream::GetInBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>(void)
template <>
SIG_UPDATE_AVATAR_JEWEL_SOCKET* Stream::GetInBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>() {
    const int bytes = 40;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_AVATAR_JEWEL_SOCKET* result =
        reinterpret_cast<SIG_UPDATE_AVATAR_JEWEL_SOCKET*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452E0E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_UPDATE_AVATAR_JEWEL_SOCKETEEPT_v
// DEMANGLED: SIG_UPDATE_AVATAR_JEWEL_SOCKET * CStreamGuard::GetInBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>(void)
template <>
SIG_UPDATE_AVATAR_JEWEL_SOCKET* CStreamGuard::GetInBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>() {
    return stream_->GetInBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>();
}

// IDA_EA: 0x850d60c
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_AVATAR_OPTION_CHANGEEEPT_v
// DEMANGLED: SIG_AVATAR_OPTION_CHANGE * Stream::GetInBuffer<SIG_AVATAR_OPTION_CHANGE>(void)
template <>
SIG_AVATAR_OPTION_CHANGE* Stream::GetInBuffer<SIG_AVATAR_OPTION_CHANGE>() {
    const int bytes = 12;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AVATAR_OPTION_CHANGE* result =
        reinterpret_cast<SIG_AVATAR_OPTION_CHANGE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x850D5B2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_AVATAR_OPTION_CHANGEEEPT_v
// DEMANGLED: SIG_AVATAR_OPTION_CHANGE * CStreamGuard::GetInBuffer<SIG_AVATAR_OPTION_CHANGE>(void)
template <>
SIG_AVATAR_OPTION_CHANGE* CStreamGuard::GetInBuffer<SIG_AVATAR_OPTION_CHANGE>() {
    return stream_->GetInBuffer<SIG_AVATAR_OPTION_CHANGE>();
}

// IDA_EA: 0x850d676
// IDA_NAME: _ZN6Stream11GetInBufferI23SIG_AVATAR_COLOR_CHANGEEEPT_v
// DEMANGLED: SIG_AVATAR_COLOR_CHANGE * Stream::GetInBuffer<SIG_AVATAR_COLOR_CHANGE>(void)
template <>
SIG_AVATAR_COLOR_CHANGE* Stream::GetInBuffer<SIG_AVATAR_COLOR_CHANGE>() {
    const int bytes = 12;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AVATAR_COLOR_CHANGE* result =
        reinterpret_cast<SIG_AVATAR_COLOR_CHANGE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x850D5C8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_AVATAR_COLOR_CHANGEEEPT_v
// DEMANGLED: SIG_AVATAR_COLOR_CHANGE * CStreamGuard::GetInBuffer<SIG_AVATAR_COLOR_CHANGE>(void)
template <>
SIG_AVATAR_COLOR_CHANGE* CStreamGuard::GetInBuffer<SIG_AVATAR_COLOR_CHANGE>() {
    return stream_->GetInBuffer<SIG_AVATAR_COLOR_CHANGE>();
}
