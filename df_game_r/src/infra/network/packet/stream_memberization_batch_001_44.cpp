#include "infra/network/packet/stream_guard.h"
#include "old_data/db/event/db_update_school_point.h"

// IDA_EA: 0x84567b6
// IDA_NAME: _ZN6Stream11GetInBufferI23SIG_UPDATE_SCHOOL_POINTEEPT_v
// DEMANGLED: SIG_UPDATE_SCHOOL_POINT * Stream::GetInBuffer<SIG_UPDATE_SCHOOL_POINT>(void)
template <>
SIG_UPDATE_SCHOOL_POINT* Stream::GetInBuffer<SIG_UPDATE_SCHOOL_POINT>() {
    const int bytes = 8;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_SCHOOL_POINT* result =
        reinterpret_cast<SIG_UPDATE_SCHOOL_POINT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452A6E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_UPDATE_SCHOOL_POINTEEPT_v
// DEMANGLED: SIG_UPDATE_SCHOOL_POINT * CStreamGuard::GetInBuffer<SIG_UPDATE_SCHOOL_POINT>(void)
template <>
SIG_UPDATE_SCHOOL_POINT* CStreamGuard::GetInBuffer<SIG_UPDATE_SCHOOL_POINT>() {
    return stream_->GetInBuffer<SIG_UPDATE_SCHOOL_POINT>();
}
