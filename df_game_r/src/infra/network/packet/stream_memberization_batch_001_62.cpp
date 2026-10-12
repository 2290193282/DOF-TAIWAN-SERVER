#include "infra/network/packet/stream_guard.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x847e0aa
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_AUCTION_LOG_MESSAGE_AGEEPT_v
// DEMANGLED: SIG_AUCTION_LOG_MESSAGE_AG * Stream::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_AG>(void)
template <>
SIG_AUCTION_LOG_MESSAGE_AG* Stream::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_AG>() {
    const int bytes = 33;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AUCTION_LOG_MESSAGE_AG* result =
        reinterpret_cast<SIG_AUCTION_LOG_MESSAGE_AG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x847DDD6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_AUCTION_LOG_MESSAGE_AGEEPT_v
// DEMANGLED: SIG_AUCTION_LOG_MESSAGE_AG * CStreamGuard::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_AG>(void)
template <>
SIG_AUCTION_LOG_MESSAGE_AG* CStreamGuard::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_AG>() {
    return stream_->GetInBuffer<SIG_AUCTION_LOG_MESSAGE_AG>();
}

// IDA_EA: 0x847e17e
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_AUCTION_LOG_MESSAGE_PGEEPT_v
// DEMANGLED: SIG_AUCTION_LOG_MESSAGE_PG * Stream::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_PG>(void)
template <>
SIG_AUCTION_LOG_MESSAGE_PG* Stream::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_PG>() {
    const int bytes = 32;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AUCTION_LOG_MESSAGE_PG* result =
        reinterpret_cast<SIG_AUCTION_LOG_MESSAGE_PG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x847DE02
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_AUCTION_LOG_MESSAGE_PGEEPT_v
// DEMANGLED: SIG_AUCTION_LOG_MESSAGE_PG * CStreamGuard::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_PG>(void)
template <>
SIG_AUCTION_LOG_MESSAGE_PG* CStreamGuard::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_PG>() {
    return stream_->GetInBuffer<SIG_AUCTION_LOG_MESSAGE_PG>();
}
