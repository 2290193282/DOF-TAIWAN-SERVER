#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auction_cashshop/zero_trade_payload.h"

// IDA_EA: 0x853242e
// IDA_NAME: _ZN6Stream11GetInBufferI28SIG_SAVE_GOLD_ZERO_TRADE_LOGEEPT_v
// DEMANGLED: SIG_SAVE_GOLD_ZERO_TRADE_LOG * Stream::GetInBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG>(void)
template <>
SIG_SAVE_GOLD_ZERO_TRADE_LOG* Stream::GetInBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG>() {
    const int bytes = 224;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_GOLD_ZERO_TRADE_LOG* result =
        reinterpret_cast<SIG_SAVE_GOLD_ZERO_TRADE_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x85321A0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI28SIG_SAVE_GOLD_ZERO_TRADE_LOGEEPT_v
// DEMANGLED: SIG_SAVE_GOLD_ZERO_TRADE_LOG * CStreamGuard::GetInBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG>(void)
template <>
SIG_SAVE_GOLD_ZERO_TRADE_LOG* CStreamGuard::GetInBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG>() {
    return stream_->GetInBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG>();
}
