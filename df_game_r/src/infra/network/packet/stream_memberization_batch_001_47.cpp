#include "infra/network/packet/stream_guard.h"
#include "old_data/db/inventory/db_save_inven.h"

// IDA_EA: 0x8181acc
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_SAVE_INVENTORYEEPT_v
// DEMANGLED: SIG_SAVE_INVENTORY * Stream::GetInBuffer<SIG_SAVE_INVENTORY>(void)
template <>
SIG_SAVE_INVENTORY* Stream::GetInBuffer<SIG_SAVE_INVENTORY>() {
    const int bytes = sizeof(SIG_SAVE_INVENTORY);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_INVENTORY* result =
        reinterpret_cast<SIG_SAVE_INVENTORY*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8180DAC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_SAVE_INVENTORYEEPT_v
// DEMANGLED: SIG_SAVE_INVENTORY * CStreamGuard::GetInBuffer<SIG_SAVE_INVENTORY>(void)
template <>
SIG_SAVE_INVENTORY* CStreamGuard::GetInBuffer<SIG_SAVE_INVENTORY>() {
    return stream_->GetInBuffer<SIG_SAVE_INVENTORY>();
}
