#include "infra/network/packet/stream_guard.h"
#include "old_game/economy/mail/mail_trade_params.h"

// IDA_EA: 0x823af86
// IDA_NAME: _ZN6Stream11GetInBufferI21MSG_BUY_CERASHOP_ITEMEEPT_v
// DEMANGLED: MSG_BUY_CERASHOP_ITEM * Stream::GetInBuffer<MSG_BUY_CERASHOP_ITEM>(void)
template <>
MSG_BUY_CERASHOP_ITEM* Stream::GetInBuffer<MSG_BUY_CERASHOP_ITEM>() {
    const int bytes = 16260;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    MSG_BUY_CERASHOP_ITEM* result =
        reinterpret_cast<MSG_BUY_CERASHOP_ITEM*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823791C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21MSG_BUY_CERASHOP_ITEMEEPT_v
// DEMANGLED: MSG_BUY_CERASHOP_ITEM * CStreamGuard::GetInBuffer<MSG_BUY_CERASHOP_ITEM>(void)
template <>
MSG_BUY_CERASHOP_ITEM* CStreamGuard::GetInBuffer<MSG_BUY_CERASHOP_ITEM>() {
    return stream_->GetInBuffer<MSG_BUY_CERASHOP_ITEM>();
}
