#include "infra/network/packet/blood_dungeon_stream.h"

// IDA_EA: 0x832A696
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_BLOOD_DUNGEONEEPT_v
// DEMANGLED: SIG_BLOOD_DUNGEON * CStreamGuard::GetInBuffer<SIG_BLOOD_DUNGEON>(void)
template <>
SIG_BLOOD_DUNGEON* CStreamGuard::GetInBuffer<SIG_BLOOD_DUNGEON>() {
    return stream_->GetInBuffer<SIG_BLOOD_DUNGEON>();
}

// IDA_EA: 0x832A6AC
// IDA_NAME: _ZN6Stream11GetInBufferI17SIG_BLOOD_DUNGEONEEPT_v
// DEMANGLED: SIG_BLOOD_DUNGEON * Stream::GetInBuffer<SIG_BLOOD_DUNGEON>(void)
template <>
SIG_BLOOD_DUNGEON* Stream::GetInBuffer<SIG_BLOOD_DUNGEON>() {
    const int bytes = sizeof(SIG_BLOOD_DUNGEON);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_BLOOD_DUNGEON* const result =
        reinterpret_cast<SIG_BLOOD_DUNGEON*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845352A
// IDA_NAME: _ZN6Stream12GetOutBufferI17SIG_BLOOD_DUNGEONEEPT_v
// DEMANGLED: SIG_BLOOD_DUNGEON * Stream::GetOutBuffer<SIG_BLOOD_DUNGEON>(void)
template <>
SIG_BLOOD_DUNGEON* Stream::GetOutBuffer<SIG_BLOOD_DUNGEON>() {
    const int bytes = sizeof(SIG_BLOOD_DUNGEON);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_BLOOD_DUNGEON* const result =
        reinterpret_cast<SIG_BLOOD_DUNGEON*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
