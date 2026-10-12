#include "infra/network/packet/stream_guard.h"
#include "old_data/db/game_option/db_load_game_option.h"

// IDA_EA: 0x84568f4
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_LOAD_GAME_OPTIONEEPT_v
// DEMANGLED: SIG_LOAD_GAME_OPTION * Stream::GetInBuffer<SIG_LOAD_GAME_OPTION>(void)
template <>
SIG_LOAD_GAME_OPTION* Stream::GetInBuffer<SIG_LOAD_GAME_OPTION>() {
    const int bytes = static_cast<int>(sizeof(SIG_LOAD_GAME_OPTION));
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_GAME_OPTION* result =
        reinterpret_cast<SIG_LOAD_GAME_OPTION*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452BAC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_LOAD_GAME_OPTIONEEPT_v
// DEMANGLED: SIG_LOAD_GAME_OPTION * CStreamGuard::GetInBuffer<SIG_LOAD_GAME_OPTION>(void)
template <>
SIG_LOAD_GAME_OPTION* CStreamGuard::GetInBuffer<SIG_LOAD_GAME_OPTION>() {
    return stream_->GetInBuffer<SIG_LOAD_GAME_OPTION>();
}

// IDA_EA: 0x84f2d48
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_REQ_LOAD_GAME_OPTIONEEPT_v
// DEMANGLED: SIG_REQ_LOAD_GAME_OPTION * Stream::GetInBuffer<SIG_REQ_LOAD_GAME_OPTION>(void)
template <>
SIG_REQ_LOAD_GAME_OPTION* Stream::GetInBuffer<SIG_REQ_LOAD_GAME_OPTION>() {
    const int bytes = static_cast<int>(sizeof(SIG_REQ_LOAD_GAME_OPTION));
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REQ_LOAD_GAME_OPTION* result =
        reinterpret_cast<SIG_REQ_LOAD_GAME_OPTION*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84F1646
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_REQ_LOAD_GAME_OPTIONEEPT_v
// DEMANGLED: SIG_REQ_LOAD_GAME_OPTION * CStreamGuard::GetInBuffer<SIG_REQ_LOAD_GAME_OPTION>(void)
template <>
SIG_REQ_LOAD_GAME_OPTION* CStreamGuard::GetInBuffer<SIG_REQ_LOAD_GAME_OPTION>() {
    return stream_->GetInBuffer<SIG_REQ_LOAD_GAME_OPTION>();
}
