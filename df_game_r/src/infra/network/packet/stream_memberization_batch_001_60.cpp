#include "infra/network/packet/stream_guard.h"
#include "old_game/economy/mall/quest_shop_runtime.h"

// IDA_EA: 0x85f0302
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_QUEST_SHOP_DATAEEPT_v
// DEMANGLED: SIG_QUEST_SHOP_DATA * Stream::GetInBuffer<SIG_QUEST_SHOP_DATA>(void)
template <>
SIG_QUEST_SHOP_DATA* Stream::GetInBuffer<SIG_QUEST_SHOP_DATA>() {
    const int bytes = 341;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_QUEST_SHOP_DATA* result =
        reinterpret_cast<SIG_QUEST_SHOP_DATA*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x85F00C8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_QUEST_SHOP_DATAEEPT_v
// DEMANGLED: SIG_QUEST_SHOP_DATA * CStreamGuard::GetInBuffer<SIG_QUEST_SHOP_DATA>(void)
template <>
SIG_QUEST_SHOP_DATA* CStreamGuard::GetInBuffer<SIG_QUEST_SHOP_DATA>() {
    return stream_->GetInBuffer<SIG_QUEST_SHOP_DATA>();
}
