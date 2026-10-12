#include "infra/network/packet/stream_guard.h"
#include "old_data/db/anti_cheat/db_load_clean_pad_point.h"

// IDA_EA: 0x845674c
// IDA_NAME: _ZN6Stream11GetInBufferI23SIG_LOAD_CLEANPAD_POINTEEPT_v
// DEMANGLED: SIG_LOAD_CLEANPAD_POINT * Stream::GetInBuffer<SIG_LOAD_CLEANPAD_POINT>(void)
template <>
SIG_LOAD_CLEANPAD_POINT* Stream::GetInBuffer<SIG_LOAD_CLEANPAD_POINT>() {
    const int bytes = 15600;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_CLEANPAD_POINT* result =
        reinterpret_cast<SIG_LOAD_CLEANPAD_POINT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452A04
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_LOAD_CLEANPAD_POINTEEPT_v
// DEMANGLED: SIG_LOAD_CLEANPAD_POINT * CStreamGuard::GetInBuffer<SIG_LOAD_CLEANPAD_POINT>(void)
template <>
SIG_LOAD_CLEANPAD_POINT* CStreamGuard::GetInBuffer<SIG_LOAD_CLEANPAD_POINT>() {
    return stream_->GetInBuffer<SIG_LOAD_CLEANPAD_POINT>();
}
