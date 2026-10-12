#include "infra/network/packet/stream_guard.h"
#include "old_data/db/creature/creature_item_payload.h"

// IDA_EA: 0x8343ADE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_CREATURE_ITEMEEPT_v
// DEMANGLED: SIG_CREATURE_ITEM * CStreamGuard::GetInBuffer<SIG_CREATURE_ITEM>(void)
template <>
SIG_CREATURE_ITEM* CStreamGuard::GetInBuffer<SIG_CREATURE_ITEM>() {
    return stream_->GetInBuffer<SIG_CREATURE_ITEM>();
}
