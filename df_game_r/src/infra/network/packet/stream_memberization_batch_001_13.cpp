#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auction_cashshop/purchase_cash_item_bonus_payload.h"

// IDA_EA: 0x8457846
// IDA_NAME: _ZN6Stream11GetInBufferI27SIG_PURCHASE_CASHITEM_BONUSEEPT_v
// DEMANGLED: SIG_PURCHASE_CASHITEM_BONUS * Stream::GetInBuffer<SIG_PURCHASE_CASHITEM_BONUS>(void)
template <>
SIG_PURCHASE_CASHITEM_BONUS* Stream::GetInBuffer<SIG_PURCHASE_CASHITEM_BONUS>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_PURCHASE_CASHITEM_BONUS* result =
        reinterpret_cast<SIG_PURCHASE_CASHITEM_BONUS*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453BA6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI27SIG_PURCHASE_CASHITEM_BONUSEEPT_v
// DEMANGLED: SIG_PURCHASE_CASHITEM_BONUS * CStreamGuard::GetInBuffer<SIG_PURCHASE_CASHITEM_BONUS>(void)
template <>
SIG_PURCHASE_CASHITEM_BONUS* CStreamGuard::GetInBuffer<SIG_PURCHASE_CASHITEM_BONUS>() {
    return stream_->GetInBuffer<SIG_PURCHASE_CASHITEM_BONUS>();
}
