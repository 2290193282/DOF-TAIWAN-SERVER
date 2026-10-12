#include "infra/network/packet/stream_guard.h"
#include "old_game/activity/schedule/event_runtime.h"

// IDA_EA: 0x817A50C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22stCeraShopIPGRequest_tEEPT_v
// DEMANGLED: stCeraShopIPGRequest_t * CStreamGuard::GetInBuffer<stCeraShopIPGRequest_t>(void)
template <>
stCeraShopIPGRequest_t* CStreamGuard::GetInBuffer<stCeraShopIPGRequest_t>() {
    return stream_->GetInBuffer<stCeraShopIPGRequest_t>();
}
