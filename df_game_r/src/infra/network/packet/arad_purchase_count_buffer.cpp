#include "infra/network/packet/arad_purchase_count_buffer.h"


// IDA_EA: 0x818EDE8
// IDA_NAME: _ZN6Stream12GetOutBufferI21MSG_BUY_CERASHOP_ITEMEEPT_v
// DEMANGLED: MSG_BUY_CERASHOP_ITEM * Stream::GetOutBuffer<MSG_BUY_CERASHOP_ITEM>(void)
template <>
MSG_BUY_CERASHOP_ITEM* Stream::GetOutBuffer<MSG_BUY_CERASHOP_ITEM>() {
    const int bytes = sizeof(MSG_BUY_CERASHOP_ITEM);
    if (enable_out(bytes) != true) {
        return 0;
    }
    MSG_BUY_CERASHOP_ITEM* const result =
        reinterpret_cast<MSG_BUY_CERASHOP_ITEM*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x818EE52
// IDA_NAME: _ZN6Stream11GetInBufferIN4ARAD15INTERNAL_PACKET20PURCHASE_COUNT_CHECKEEEPT_v
// DEMANGLED: ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK * Stream::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>(void)
template <>
ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK*
Stream::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>() {
    const int bytes = sizeof(ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK* const result =
        reinterpret_cast<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x818EE3C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN4ARAD15INTERNAL_PACKET20PURCHASE_COUNT_CHECKEEEPT_v
// DEMANGLED: ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK * CStreamGuard::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>(void)
template <>
ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK*
CStreamGuard::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>() {
    return stream_->GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>();
}
