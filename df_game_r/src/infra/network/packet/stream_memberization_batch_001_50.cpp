#include "infra/network/packet/stream_guard.h"
#include "old_data/db/pvp_party/db_save_pvp.h"

// IDA_EA: 0x869fbb4
// IDA_NAME: _ZN6Stream11GetInBufferI12SIG_SAVE_PVPEEPT_v
// DEMANGLED: SIG_SAVE_PVP * Stream::GetInBuffer<SIG_SAVE_PVP>(void)
template <>
SIG_SAVE_PVP* Stream::GetInBuffer<SIG_SAVE_PVP>() {
    const int bytes = 136;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_PVP* result =
        reinterpret_cast<SIG_SAVE_PVP*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869AA76
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI12SIG_SAVE_PVPEEPT_v
// DEMANGLED: SIG_SAVE_PVP * CStreamGuard::GetInBuffer<SIG_SAVE_PVP>(void)
template <>
SIG_SAVE_PVP* CStreamGuard::GetInBuffer<SIG_SAVE_PVP>() {
    return stream_->GetInBuffer<SIG_SAVE_PVP>();
}
