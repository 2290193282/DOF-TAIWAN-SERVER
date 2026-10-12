#include "infra/network/packet/stream_guard.h"
#include "old_data/channel/legacy_runtime/pvp_channel.h"

// IDA_EA: 0x8237712
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI14SIG_LOGIN_DATAEEPT_v
// DEMANGLED: SIG_LOGIN_DATA * CStreamGuard::GetInBuffer<SIG_LOGIN_DATA>(void)
template <>
SIG_LOGIN_DATA* CStreamGuard::GetInBuffer<SIG_LOGIN_DATA>() {
    return stream_->GetInBuffer<SIG_LOGIN_DATA>();
}
