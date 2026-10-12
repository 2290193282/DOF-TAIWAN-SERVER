#include "infra/network/packet/stream_guard.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"

// IDA_EA: 0x82398da
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_LOAD_DUNGEON_CLEAREEPT_v
// DEMANGLED: SIG_LOAD_DUNGEON_CLEAR * Stream::GetInBuffer<SIG_LOAD_DUNGEON_CLEAR>(void)
template <>
SIG_LOAD_DUNGEON_CLEAR* Stream::GetInBuffer<SIG_LOAD_DUNGEON_CLEAR>() {
    const int bytes = 6160;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_DUNGEON_CLEAR* result =
        reinterpret_cast<SIG_LOAD_DUNGEON_CLEAR*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236D88
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_LOAD_DUNGEON_CLEAREEPT_v
// DEMANGLED: SIG_LOAD_DUNGEON_CLEAR * CStreamGuard::GetInBuffer<SIG_LOAD_DUNGEON_CLEAR>(void)
template <>
SIG_LOAD_DUNGEON_CLEAR* CStreamGuard::GetInBuffer<SIG_LOAD_DUNGEON_CLEAR>() {
    return stream_->GetInBuffer<SIG_LOAD_DUNGEON_CLEAR>();
}

// IDA_EA: 0x869fc1e
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_SAVE_DUNGEON_CLEAREEPT_v
// DEMANGLED: SIG_SAVE_DUNGEON_CLEAR * Stream::GetInBuffer<SIG_SAVE_DUNGEON_CLEAR>(void)
template <>
SIG_SAVE_DUNGEON_CLEAR* Stream::GetInBuffer<SIG_SAVE_DUNGEON_CLEAR>() {
    const int bytes = 6156;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_DUNGEON_CLEAR* result =
        reinterpret_cast<SIG_SAVE_DUNGEON_CLEAR*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869AA8C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_SAVE_DUNGEON_CLEAREEPT_v
// DEMANGLED: SIG_SAVE_DUNGEON_CLEAR * CStreamGuard::GetInBuffer<SIG_SAVE_DUNGEON_CLEAR>(void)
template <>
SIG_SAVE_DUNGEON_CLEAR* CStreamGuard::GetInBuffer<SIG_SAVE_DUNGEON_CLEAR>() {
    return stream_->GetInBuffer<SIG_SAVE_DUNGEON_CLEAR>();
}
