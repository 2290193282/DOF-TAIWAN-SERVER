#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"

#include "common/manager/in_game_advertisement_abi.h"


// IDA_EA: 0x80FED5A
// IDA_NAME: _ZN6Stream11GetInBufferI32SIG_RELOAD_IN_GAME_ADVERTISEMENTEEPT_v
// DEMANGLED: SIG_RELOAD_IN_GAME_ADVERTISEMENT * Stream::GetInBuffer<SIG_RELOAD_IN_GAME_ADVERTISEMENT>(void)
template <>
SIG_RELOAD_IN_GAME_ADVERTISEMENT*
Stream::GetInBuffer<SIG_RELOAD_IN_GAME_ADVERTISEMENT>() {
    const int bytes = sizeof(SIG_RELOAD_IN_GAME_ADVERTISEMENT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_RELOAD_IN_GAME_ADVERTISEMENT* result =
        reinterpret_cast<SIG_RELOAD_IN_GAME_ADVERTISEMENT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x80FE5BE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI32SIG_RELOAD_IN_GAME_ADVERTISEMENTEEPT_v
// DEMANGLED: SIG_RELOAD_IN_GAME_ADVERTISEMENT * CStreamGuard::GetInBuffer<SIG_RELOAD_IN_GAME_ADVERTISEMENT>(void)
template <>
SIG_RELOAD_IN_GAME_ADVERTISEMENT*
CStreamGuard::GetInBuffer<SIG_RELOAD_IN_GAME_ADVERTISEMENT>() {
    return stream_->GetInBuffer<SIG_RELOAD_IN_GAME_ADVERTISEMENT>();
}

// IDA_EA: 0x80FEDC4
// IDA_NAME: _ZN6Stream11GetInBufferI29SIG_IN_GAME_ADVERTISEMENT_LOGEEPT_v
// DEMANGLED: SIG_IN_GAME_ADVERTISEMENT_LOG * Stream::GetInBuffer<SIG_IN_GAME_ADVERTISEMENT_LOG>(void)
template <>
SIG_IN_GAME_ADVERTISEMENT_LOG*
Stream::GetInBuffer<SIG_IN_GAME_ADVERTISEMENT_LOG>() {
    const int bytes = sizeof(SIG_IN_GAME_ADVERTISEMENT_LOG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_IN_GAME_ADVERTISEMENT_LOG* result =
        reinterpret_cast<SIG_IN_GAME_ADVERTISEMENT_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x80FE5D4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_IN_GAME_ADVERTISEMENT_LOGEEPT_v
// DEMANGLED: SIG_IN_GAME_ADVERTISEMENT_LOG * CStreamGuard::GetInBuffer<SIG_IN_GAME_ADVERTISEMENT_LOG>(void)
template <>
SIG_IN_GAME_ADVERTISEMENT_LOG*
CStreamGuard::GetInBuffer<SIG_IN_GAME_ADVERTISEMENT_LOG>() {
    return stream_->GetInBuffer<SIG_IN_GAME_ADVERTISEMENT_LOG>();
}
