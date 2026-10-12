#include "infra/network/packet/stream_item_lock.h"

#include "old_game/economy/inventory/item_lock_payload.h"


// IDA_EA: 0x8453768
// IDA_NAME: _ZN6Stream12GetOutBufferI20SIG_ITEM_LOCK_UPDATEEEPT_v
// DEMANGLED: SIG_ITEM_LOCK_UPDATE * Stream::GetOutBuffer<SIG_ITEM_LOCK_UPDATE>(void)
template <>
SIG_ITEM_LOCK_UPDATE*
Stream::GetOutBuffer<SIG_ITEM_LOCK_UPDATE>() {
    const int bytes = sizeof(SIG_ITEM_LOCK_UPDATE);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_ITEM_LOCK_UPDATE* result =
        reinterpret_cast<SIG_ITEM_LOCK_UPDATE*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8543836
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_ITEM_LOCK_UPDATEEEPT_v
// DEMANGLED: SIG_ITEM_LOCK_UPDATE * Stream::GetInBuffer<SIG_ITEM_LOCK_UPDATE>(void)
template <>
SIG_ITEM_LOCK_UPDATE*
Stream::GetInBuffer<SIG_ITEM_LOCK_UPDATE>() {
    const int bytes = sizeof(SIG_ITEM_LOCK_UPDATE);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_ITEM_LOCK_UPDATE* result =
        reinterpret_cast<SIG_ITEM_LOCK_UPDATE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8543238
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_ITEM_LOCK_UPDATEEEPT_v
// DEMANGLED: SIG_ITEM_LOCK_UPDATE * CStreamGuard::GetInBuffer<SIG_ITEM_LOCK_UPDATE>(void)
template <>
SIG_ITEM_LOCK_UPDATE*
CStreamGuard::GetInBuffer<SIG_ITEM_LOCK_UPDATE>() {
    return stream_->GetInBuffer<SIG_ITEM_LOCK_UPDATE>();
}
