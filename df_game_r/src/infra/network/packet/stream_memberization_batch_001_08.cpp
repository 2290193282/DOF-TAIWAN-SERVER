#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auction_cashshop/db_mileage.h"

// IDA_EA: 0x869f42e
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_MILEAGE_STRUCTEEPT_v
// DEMANGLED: SIG_MILEAGE_STRUCT * Stream::GetInBuffer<SIG_MILEAGE_STRUCT>(void)
template <>
SIG_MILEAGE_STRUCT* Stream::GetInBuffer<SIG_MILEAGE_STRUCT>() {
    const int bytes = 12;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_MILEAGE_STRUCT* result =
        reinterpret_cast<SIG_MILEAGE_STRUCT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8699246
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_MILEAGE_STRUCTEEPT_v
// DEMANGLED: SIG_MILEAGE_STRUCT * CStreamGuard::GetInBuffer<SIG_MILEAGE_STRUCT>(void)
template <>
SIG_MILEAGE_STRUCT* CStreamGuard::GetInBuffer<SIG_MILEAGE_STRUCT>() {
    return stream_->GetInBuffer<SIG_MILEAGE_STRUCT>();
}
