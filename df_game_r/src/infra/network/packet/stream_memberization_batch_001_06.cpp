#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auction_cashshop/cera_shop_purchase_count_payload.h"

// IDA_EA: 0x8457fc4
// IDA_NAME: _ZN6Stream11GetInBufferI27SIG_CERASHOP_PURCHASE_COUNTEEPT_v
// DEMANGLED: SIG_CERASHOP_PURCHASE_COUNT * Stream::GetInBuffer<SIG_CERASHOP_PURCHASE_COUNT>(void)
template <>
SIG_CERASHOP_PURCHASE_COUNT* Stream::GetInBuffer<SIG_CERASHOP_PURCHASE_COUNT>() {
    const int bytes = 16;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CERASHOP_PURCHASE_COUNT* result =
        reinterpret_cast<SIG_CERASHOP_PURCHASE_COUNT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84547F6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI27SIG_CERASHOP_PURCHASE_COUNTEEPT_v
// DEMANGLED: SIG_CERASHOP_PURCHASE_COUNT * CStreamGuard::GetInBuffer<SIG_CERASHOP_PURCHASE_COUNT>(void)
template <>
SIG_CERASHOP_PURCHASE_COUNT* CStreamGuard::GetInBuffer<SIG_CERASHOP_PURCHASE_COUNT>() {
    return stream_->GetInBuffer<SIG_CERASHOP_PURCHASE_COUNT>();
}
