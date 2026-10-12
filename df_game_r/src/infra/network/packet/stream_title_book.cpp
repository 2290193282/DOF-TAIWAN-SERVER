#include "old_data/db/inventory/title_book_payload.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"

// IDA_EA: 0x84508CE
// IDA_NAME: _ZN6Stream12GetOutBufferI16stTitleBook_SaveEEPT_v
// DEMANGLED: stTitleBook_Save * Stream::GetOutBuffer<stTitleBook_Save>(void)
template <>
stTitleBook_Save* Stream::GetOutBuffer<stTitleBook_Save>() {
    const int bytes = sizeof(stTitleBook_Save);
    if (!enable_out(bytes)) {
        return 0;
    }
    stTitleBook_Save* const result =
        reinterpret_cast<stTitleBook_Save*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x86431D0
// IDA_NAME: _ZN6Stream11GetInBufferI16stTitleBook_SaveEEPT_v
// DEMANGLED: stTitleBook_Save * Stream::GetInBuffer<stTitleBook_Save>(void)
template <>
stTitleBook_Save* Stream::GetInBuffer<stTitleBook_Save>() {
    const int bytes = sizeof(stTitleBook_Save);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    stTitleBook_Save* const result =
        reinterpret_cast<stTitleBook_Save*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x864314C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI16stTitleBook_SaveEEPT_v
// DEMANGLED: stTitleBook_Save * CStreamGuard::GetInBuffer<stTitleBook_Save>(void)
template <>
stTitleBook_Save* CStreamGuard::GetInBuffer<stTitleBook_Save>() {
    return stream_->GetInBuffer<stTitleBook_Save>();
}
