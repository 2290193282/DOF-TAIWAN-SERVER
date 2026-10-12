#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_load_no_cache_data.h"

// IDA_EA: 0x8457068
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_LOAD_NO_CACHE_DATAEEPT_v
// DEMANGLED: SIG_LOAD_NO_CACHE_DATA * Stream::GetInBuffer<SIG_LOAD_NO_CACHE_DATA>(void)
template <>
SIG_LOAD_NO_CACHE_DATA* Stream::GetInBuffer<SIG_LOAD_NO_CACHE_DATA>() {
    const int bytes = 44;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_NO_CACHE_DATA* result =
        reinterpret_cast<SIG_LOAD_NO_CACHE_DATA*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84536BC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_LOAD_NO_CACHE_DATAEEPT_v
// DEMANGLED: SIG_LOAD_NO_CACHE_DATA * CStreamGuard::GetInBuffer<SIG_LOAD_NO_CACHE_DATA>(void)
template <>
SIG_LOAD_NO_CACHE_DATA* CStreamGuard::GetInBuffer<SIG_LOAD_NO_CACHE_DATA>() {
    return stream_->GetInBuffer<SIG_LOAD_NO_CACHE_DATA>();
}
