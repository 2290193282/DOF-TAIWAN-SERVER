#include "infra/network/packet/stream_guard.h"
#include "old_data/db/character/db_load_charac_hotkey_option.h"

// IDA_EA: 0x8239d92
// IDA_NAME: _ZN6Stream11GetInBufferI29SIG_LOAD_CHARAC_HOTKEY_OPTIONEEPT_v
// DEMANGLED: SIG_LOAD_CHARAC_HOTKEY_OPTION * Stream::GetInBuffer<SIG_LOAD_CHARAC_HOTKEY_OPTION>(void)
template <>
SIG_LOAD_CHARAC_HOTKEY_OPTION* Stream::GetInBuffer<SIG_LOAD_CHARAC_HOTKEY_OPTION>() {
    const int bytes = sizeof(SIG_LOAD_CHARAC_HOTKEY_OPTION);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_CHARAC_HOTKEY_OPTION* result =
        reinterpret_cast<SIG_LOAD_CHARAC_HOTKEY_OPTION*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236EB4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_LOAD_CHARAC_HOTKEY_OPTIONEEPT_v
// DEMANGLED: SIG_LOAD_CHARAC_HOTKEY_OPTION * CStreamGuard::GetInBuffer<SIG_LOAD_CHARAC_HOTKEY_OPTION>(void)
template <>
SIG_LOAD_CHARAC_HOTKEY_OPTION* CStreamGuard::GetInBuffer<SIG_LOAD_CHARAC_HOTKEY_OPTION>() {
    return stream_->GetInBuffer<SIG_LOAD_CHARAC_HOTKEY_OPTION>();
}

// IDA_EA: 0x845688a
// IDA_NAME: _ZN6Stream11GetInBufferI33SIG_REQ_LOAD_CHARAC_HOTKEY_OPTIONEEPT_v
// DEMANGLED: SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION * Stream::GetInBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>(void)
template <>
SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION* Stream::GetInBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>() {
    const int bytes = 8;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION* result =
        reinterpret_cast<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452B42
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI33SIG_REQ_LOAD_CHARAC_HOTKEY_OPTIONEEPT_v
// DEMANGLED: SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION * CStreamGuard::GetInBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>(void)
template <>
SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION* CStreamGuard::GetInBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>() {
    return stream_->GetInBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>();
}
