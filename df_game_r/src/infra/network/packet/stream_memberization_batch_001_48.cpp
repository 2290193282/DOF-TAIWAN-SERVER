#include "infra/network/packet/stream_guard.h"
#include "old_data/db/inventory/load_inventory_payload.h"

// IDA_EA: 0x8239732
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_LOAD_INVENTORYEEPT_v
// DEMANGLED: SIG_LOAD_INVENTORY * Stream::GetInBuffer<SIG_LOAD_INVENTORY>(void)
template <>
SIG_LOAD_INVENTORY* Stream::GetInBuffer<SIG_LOAD_INVENTORY>() {
    const int bytes = sizeof(SIG_LOAD_INVENTORY);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_INVENTORY* result =
        reinterpret_cast<SIG_LOAD_INVENTORY*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236D30
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_LOAD_INVENTORYEEPT_v
// DEMANGLED: SIG_LOAD_INVENTORY * CStreamGuard::GetInBuffer<SIG_LOAD_INVENTORY>(void)
template <>
SIG_LOAD_INVENTORY* CStreamGuard::GetInBuffer<SIG_LOAD_INVENTORY>() {
    return stream_->GetInBuffer<SIG_LOAD_INVENTORY>();
}
