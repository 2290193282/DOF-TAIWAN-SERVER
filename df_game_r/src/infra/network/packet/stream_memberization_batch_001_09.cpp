#include "infra/network/packet/stream_guard.h"
#include "old_data/db/game_option/db_save_game_option_1.h"

// IDA_EA: 0x8239d28
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_SAVE_GAME_OPTION_1EEPT_v
// DEMANGLED: SIG_SAVE_GAME_OPTION_1 * Stream::GetInBuffer<SIG_SAVE_GAME_OPTION_1>(void)
template <>
SIG_SAVE_GAME_OPTION_1* Stream::GetInBuffer<SIG_SAVE_GAME_OPTION_1>() {
    const int bytes = static_cast<int>(sizeof(SIG_SAVE_GAME_OPTION_1));
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_GAME_OPTION_1* result =
        reinterpret_cast<SIG_SAVE_GAME_OPTION_1*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236E9E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_SAVE_GAME_OPTION_1EEPT_v
// DEMANGLED: SIG_SAVE_GAME_OPTION_1 * CStreamGuard::GetInBuffer<SIG_SAVE_GAME_OPTION_1>(void)
template <>
SIG_SAVE_GAME_OPTION_1* CStreamGuard::GetInBuffer<SIG_SAVE_GAME_OPTION_1>() {
    return stream_->GetInBuffer<SIG_SAVE_GAME_OPTION_1>();
}
