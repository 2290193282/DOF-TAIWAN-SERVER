#include "old_data/db/inventory/expand_equipslot_payload.h"
#include "infra/network/packet/stream_expand_equipslot.h"

// IDA_EA: 0x849AB98
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_SAVE_EXPAND_CHARACEEPT_v
// DEMANGLED: SIG_SAVE_EXPAND_CHARAC * CStreamGuard::GetInBuffer<SIG_SAVE_EXPAND_CHARAC>(void)
template <>
SIG_SAVE_EXPAND_CHARAC*
CStreamGuard::GetInBuffer<SIG_SAVE_EXPAND_CHARAC>() {
    return stream_->GetInBuffer<SIG_SAVE_EXPAND_CHARAC>();
}

// IDA_EA: 0x849ABC6
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_SAVE_EXPAND_CHARACEEPT_v
// DEMANGLED: SIG_SAVE_EXPAND_CHARAC * Stream::GetInBuffer<SIG_SAVE_EXPAND_CHARAC>(void)
template <>
SIG_SAVE_EXPAND_CHARAC*
Stream::GetInBuffer<SIG_SAVE_EXPAND_CHARAC>() {
    const int bytes = sizeof(SIG_SAVE_EXPAND_CHARAC);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_SAVE_EXPAND_CHARAC* const payload =
        reinterpret_cast<SIG_SAVE_EXPAND_CHARAC*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return payload;
}

// IDA_EA: 0x845087A
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_SAVE_EXPAND_CHARACEEPT_v
// DEMANGLED: SIG_SAVE_EXPAND_CHARAC * Stream::GetOutBuffer<SIG_SAVE_EXPAND_CHARAC>(void)
template <>
SIG_SAVE_EXPAND_CHARAC*
Stream::GetOutBuffer<SIG_SAVE_EXPAND_CHARAC>() {
    const int bytes = sizeof(SIG_SAVE_EXPAND_CHARAC);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_SAVE_EXPAND_CHARAC* const payload =
        reinterpret_cast<SIG_SAVE_EXPAND_CHARAC*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return payload;
}
