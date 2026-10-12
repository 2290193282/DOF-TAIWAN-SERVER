#include "infra/network/packet/stream_guard.h"
#include "old_data/db/account/break_away_payload.h"

// IDA_EA: 0x84570d2
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_BREAK_AWAY_UV_LISTEEPT_v
// DEMANGLED: SIG_BREAK_AWAY_UV_LIST * Stream::GetInBuffer<SIG_BREAK_AWAY_UV_LIST>(void)
template <>
SIG_BREAK_AWAY_UV_LIST* Stream::GetInBuffer<SIG_BREAK_AWAY_UV_LIST>() {
    const int bytes = 31;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_BREAK_AWAY_UV_LIST* result =
        reinterpret_cast<SIG_BREAK_AWAY_UV_LIST*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84536D2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_BREAK_AWAY_UV_LISTEEPT_v
// DEMANGLED: SIG_BREAK_AWAY_UV_LIST * CStreamGuard::GetInBuffer<SIG_BREAK_AWAY_UV_LIST>(void)
template <>
SIG_BREAK_AWAY_UV_LIST* CStreamGuard::GetInBuffer<SIG_BREAK_AWAY_UV_LIST>() {
    return stream_->GetInBuffer<SIG_BREAK_AWAY_UV_LIST>();
}

// IDA_EA: 0x84571a6
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_BREAK_AWAY_PREVENT_MESSAGEEEPT_v
// DEMANGLED: SIG_BREAK_AWAY_PREVENT_MESSAGE * Stream::GetInBuffer<SIG_BREAK_AWAY_PREVENT_MESSAGE>(void)
template <>
SIG_BREAK_AWAY_PREVENT_MESSAGE* Stream::GetInBuffer<SIG_BREAK_AWAY_PREVENT_MESSAGE>() {
    const int bytes = 8;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_BREAK_AWAY_PREVENT_MESSAGE* result =
        reinterpret_cast<SIG_BREAK_AWAY_PREVENT_MESSAGE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453752
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_BREAK_AWAY_PREVENT_MESSAGEEEPT_v
// DEMANGLED: SIG_BREAK_AWAY_PREVENT_MESSAGE * CStreamGuard::GetInBuffer<SIG_BREAK_AWAY_PREVENT_MESSAGE>(void)
template <>
SIG_BREAK_AWAY_PREVENT_MESSAGE* CStreamGuard::GetInBuffer<SIG_BREAK_AWAY_PREVENT_MESSAGE>() {
    return stream_->GetInBuffer<SIG_BREAK_AWAY_PREVENT_MESSAGE>();
}
